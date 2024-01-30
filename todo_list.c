#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#define FPS 60
bool running = true, creating_task = false,canWrite = false, isDragling = false;
int coord[1024][5];

SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius) {
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    while (offsety >= offsetx) {

        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
                                     x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
                                     x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
                                     x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
                                     x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}

SDL_RenderText(SDL_Renderer * renderer, SDL_Rect rect_toDraw, char text_toWrite[], bool isCreatingTask) {
    // printf("input rend text %s\n", text_toWrite);
    char text_wrapped[1024];
    strncpy(text_wrapped, text_toWrite, rect_toDraw.w/(29));
    int size;
    TTF_Font *font;
    font=TTF_OpenFont("Cookie-Regular.ttf",60);
    SDL_Color textColor = { 0, 0, 0, 0 };

    int loop_len = strlen(text_toWrite);
    int j = 2;
    // printf("len %d, max %d\n", strlen(text_wrapped), rect_toDraw.w/(29));
    for(int i = strlen(text_wrapped); i < loop_len; ++i) {
        // printf("loop %d, jmax %d\n", i, rect_toDraw.h/(68));
        if(isCreatingTask && j-1 >= rect_toDraw.h/(68)) {
            // printf("lemealone \n");
            canWrite = false;
            break;
        } else canWrite = true;
        if(i%(rect_toDraw.w/(29)) == 0) {
            char text_writted[1024];
            strncpy(text_writted, text_toWrite+((rect_toDraw.w/(29))*(j-1)), rect_toDraw.w/(29));
            text_writted[rect_toDraw.w/(29)] = '\0';
            
            // printf("rend mid %s, j %d\n", text_writted, j);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text_writted, textColor);
            SDL_Texture* textUre = SDL_CreateTextureFromSurface(renderer, textSurface);
            int text_width = textSurface->w;
            int text_height = textSurface->h;
            SDL_FreeSurface(textSurface);
            SDL_Rect text_creation = rect_toDraw;
            // printf("heigth %d\n", text_creation.y);
            text_creation.y *= j * 2;
            // printf("jmult %d\n", text_creation.y);
            if(!creating_task) {
                // text_creation.x *= 20;
                text_creation.y *= 20;
            }
            size = text_creation.y + 68;
            text_creation.w = text_width;
            text_creation.h = text_height;
            // printf("heigth %d, width %d\n", text_creation.y, text_creation.h);


            SDL_RenderCopy(renderer, textUre, NULL, &text_creation);
            SDL_DestroyTexture(textUre);
            ++j;
        }
    }
        // printf("rend final %s\n", text_wrapped);
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text_wrapped, textColor);
        SDL_Texture* textUre = SDL_CreateTextureFromSurface(renderer, textSurface);
        int text_width = textSurface->w;
        int text_height = textSurface->h;
        SDL_FreeSurface(textSurface);
        SDL_Rect text_creation = rect_toDraw;
        text_creation.w = text_width;
        text_creation.h = text_height;
        SDL_RenderCopy(renderer, textUre, NULL, &text_creation);
        SDL_DestroyTexture(textUre);
    



    return size;
}

int main(int argc, char* argv[]) {
    int WIDTH = 600;
    int HEIGHT = 600;
    float third_width = WIDTH / 3;
    
    char text[1024];
    strcpy(text, " ");

    /* Init SDL and libs */
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }
    if(TTF_Init() != 0) {
        printf("Error initializing TTF: %s\n", SDL_GetError());
        return 0;
    }
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    /* Create a window */
    SDL_Window* window = SDL_CreateWindow("Todo list",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL );
    if (!window) {
        printf("Error creating window: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    /* Create a renderer */
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, render_flags);
    if (!renderer) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    // Load image
    SDL_Texture *load_image(const char* file_name) {
        SDL_Surface *image = IMG_Load(file_name);
        SDL_Texture * image_texture = SDL_CreateTextureFromSurface(renderer, image);
        SDL_FreeSurface(image);
        return image_texture;
    }
    SDL_Texture * image_add = load_image("add.png");
    SDL_Texture * image_remove = load_image("remove.png");
    SDL_Rect texture_remove;
    SDL_Cursor *hover_cursor;
    SDL_Cursor *drag_cursor;


    // Prepare shape
    SDL_Rect coll_toDo = {(int) third_width * 0, (int) 0, third_width, HEIGHT};
    SDL_Rect coll_inProgress = {(int) third_width * 1, (int) 0, third_width, HEIGHT};
    SDL_Rect coll_done = {(int) third_width * 2, (int) 0, third_width, HEIGHT};
    SDL_Rect task_creation = {(int) 35, (int) 35, WIDTH - 70, HEIGHT - 70};


    /* Main loop */
    SDL_Event event;
    while(running) {
        // Start from blank
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        

        /*===============*\
        |   DRAW CENTER   |
        \*===============*/
        TTF_Font *font;
        font=TTF_OpenFont("Cookie-Regular.ttf",60);
        SDL_Color textColor = { 0, 0, 0, 0 };
        SDL_SetRenderDrawColor(renderer, 128, 255, 128, 255);
        SDL_RenderFillRect(renderer, &coll_toDo);
        SDL_Surface* textToDo = TTF_RenderText_Solid(font, "TODO", textColor);
        SDL_Texture* textUreDo = SDL_CreateTextureFromSurface(renderer, textToDo);
        int text_width = textToDo->w;
        int text_height = textToDo->h;
        SDL_FreeSurface(textToDo);
        SDL_Rect text_creationDo = coll_toDo;
        text_creationDo.w = text_width;
        text_creationDo.h = text_height;
        SDL_RenderCopy(renderer, textUreDo, NULL, &text_creationDo);
        SDL_DestroyTexture(textUreDo);
        SDL_SetRenderDrawColor(renderer, 128, 128, 255, 255);
        SDL_RenderFillRect(renderer, &coll_inProgress);
        SDL_Surface* textinProgress = TTF_RenderText_Solid(font, "IN PROGRESS", textColor);
        SDL_Texture* textUreIn = SDL_CreateTextureFromSurface(renderer, textinProgress);
        text_width = textinProgress->w;
        text_height = textinProgress->h;
        SDL_FreeSurface(textinProgress);
        SDL_Rect text_creationIn = coll_inProgress;
        text_creationIn.w = text_width;
        text_creationIn.h = text_height;
        SDL_RenderCopy(renderer, textUreIn, NULL, &text_creationIn);
        SDL_DestroyTexture(textUreIn);
        SDL_SetRenderDrawColor(renderer, 255, 128, 128, 255);
        SDL_RenderFillRect(renderer, &coll_done);
        SDL_Surface* textDone = TTF_RenderText_Solid(font, "DONE", textColor);
        SDL_Texture* textUreDone = SDL_CreateTextureFromSurface(renderer, textDone);
        text_width = textDone->w;
        text_height = textDone->h;
        SDL_FreeSurface(textDone);
        SDL_Rect text_creationDone = coll_done;
        text_creationDone.w = text_width;
        text_creationDone.h = text_height;
        SDL_RenderCopy(renderer, textUreDone, NULL, &text_creationDone);
        SDL_DestroyTexture(textUreDone);

        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderFillCircle(renderer, WIDTH - 35, HEIGHT - 35, 20);
        SDL_Rect texture_add = {(int) WIDTH - 57, (int) HEIGHT - 57, 45, 45};
        SDL_RenderCopy(renderer, image_add, NULL, &texture_add);

        FILE *file = fopen("tiadiba.csv", "r+");
        if (file == NULL) {
            perror(file);
            return 1;
        } 

        char buffer[100];
        char *token, *text_readed[1024];
        // strcpy(text_readed, " ");   
        int coll_readed;
        int coll_height[3] = { 68,68,68 };
        
        int loopSmiley;
        for (loopSmiley = 0;fgets(buffer, 1024, file) != NULL;++loopSmiley) {
            // printf("%s\n", buffer);
            token = strtok(buffer, ";");
            sscanf(token, "%d", &coll_readed);
            // printf("%d\n", coll_readed);
            token = strtok(NULL, ";");
            strcpy(text_readed, token);
            // printf("%s\n", text_readed);

            // printf("final %s, y%d\n",text_readed, coll_height[coll_readed]);
            SDL_Rect task_readed = {(int) coll_readed * third_width, (int) coll_height[coll_readed], third_width, 68};
            // printf("widht %d, y %d\n", task_readed.w,task_readed.y);// Pourquoi ça marche plus quand je suppr se printf ?

            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);

            SDL_RenderText(renderer, task_readed, text_readed, false);
            coll_height[coll_readed] += 68;
            // printf("trh %d, chcr %d\n", task_readed.h, coll_height[coll_readed]);
            SDL_RenderDrawRect(renderer, &task_readed);
            coord[loopSmiley][0] = task_readed.x;
            // printf("%d\n",task_readed.x);
            coord[loopSmiley][1] = task_readed.y;
            // printf("%d\n",task_readed.y);
            coord[loopSmiley][2] = task_readed.w;
            // printf("%d\n",task_readed.w);
            coord[loopSmiley][3] = task_readed.h;
            // printf("%d\n",task_readed.h);

        }
        coord[loopSmiley+1][0] = 0;
        coord[loopSmiley+1][1] = 0;
        coord[loopSmiley+1][2] = 0;
        coord[loopSmiley+1][3] = 0;
        // printf("exit?\n");
        fclose(file);
        // strcat(text_readed, "\0");



        // Creating task
        if(creating_task) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &task_creation);
            SDL_SetTextInputRect(&task_creation);
            SDL_StartTextInput();

            SDL_RenderText(renderer, task_creation, text, true);


        }


        /*================*\
        |   MOUSE CENTER   |
        \*================*/
        int *mouse_x, *mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        int hover_x = mouse_x, hover_y = mouse_y;

        // Hover add button
        if(hover_x < (WIDTH - 35) + 20 &&
            hover_x + 20 > (WIDTH - 35) &&
            hover_y < (HEIGHT - 35) + 20 &&
            hover_y + 20 > (HEIGHT - 35)
        ) {
            hover_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
            SDL_SetCursor(hover_cursor);
        }
        else SDL_FreeCursor(hover_cursor);

        // Drag
        if(isDragling) {
            SDL_Surface *surfaceCursor = IMG_Load("screenshot.bmp");
            drag_cursor = SDL_CreateColorCursor(surfaceCursor, 0, 0);
            SDL_SetCursor(drag_cursor);
        } else 
            SDL_FreeCursor(drag_cursor);



        /*================*\
        |   EVENT CENTER   |
        \*================*/
        while (SDL_PollEvent(&event))  {
            switch (event.type)  {
                case SDL_WINDOWEVENT:
                    switch (event.window.event)  {
                        // Handle resize event
                        case SDL_WINDOWEVENT_SIZE_CHANGED: 
                            WIDTH = event.window.data1;
                            HEIGHT = event.window.data2;
                            third_width = WIDTH / 3;

                            coll_toDo.x = third_width * 0;
                            coll_toDo.w = third_width;
                            coll_toDo.h = HEIGHT;
                            coll_inProgress.x = third_width * 1;
                            coll_inProgress.w = third_width;
                            coll_inProgress.h = HEIGHT;
                            coll_done.x = third_width * 2;
                            coll_done.w = third_width;
                            coll_done.h = HEIGHT;
                            task_creation.w = WIDTH - 70;
                            task_creation.h = HEIGHT - 70;
                        break;
                        
                        // Handle quit event
                        case SDL_WINDOWEVENT_CLOSE:
                            event.type = SDL_QUIT;
                            SDL_PushEvent(&event);
                        break;
                        
                    }
                break;
                
                case SDL_MOUSEBUTTONUP:
                    if(event.button.x < (WIDTH - 35) + 20 &&
                        event.button.x + 20 > (WIDTH - 35) &&
                        event.button.y < (HEIGHT - 35) + 20 &&
                        event.button.y + 20 > (HEIGHT - 35)
                    ) creating_task = true, canWrite = true;
                    if(isDragling) {
                        isDragling = false;
                        FILE *file = fopen("tiadiba.csv", w);
                        

                    }
                break;
                
                case SDL_MOUSEBUTTONDOWN :
                    for(int loopCoord = 0;coord[loopCoord][2]!=0; ++loopCoord) {
                        // printf("x %d y %d w%d h%d\nx %d y %d\n",coord[loopCoord][0],coord[loopCoord][1],coord[loopCoord][2],coord[loopCoord][3],event.button.x,event.button.y);
                        if(event.button.x < coord[loopCoord][0] + coord[loopCoord][2] &&
                        event.button.x > coord[loopCoord][0] &&
                        event.button.y < coord[loopCoord][1] + coord[loopCoord][3] &&
                        event.button.y > coord[loopCoord][1]
                        ) {
                            isDragling = true;
                            printf("LOOP %d COORD\n", loopCoord);
                            SDL_Surface *sshot = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
                            SDL_Rect screen = {(int) coord[loopCoord][0], (int) coord[loopCoord][1], coord[loopCoord][2], coord[loopCoord][3]};
                            SDL_RenderReadPixels(renderer, &screen, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
                            SDL_SaveBMP(sshot, "screenshot.bmp");
                            SDL_FreeSurface(sshot);
                        }
                    }
                break;

                case SDL_QUIT:  
                    running = false;
                break;
                
            }
            if(creating_task) {
                switch (event.type) {
                    case SDL_TEXTINPUT: 
                        /* Add new text onto the end of our text */
                        if(canWrite) 
                            strcat(text, event.text.text);
                    break;
                    case SDL_KEYDOWN:
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_BACKSPACE:
                                if(strlen(text) > 1)
                                    text[strlen(text)-1] = '\0';
                            break;
                            case SDL_SCANCODE_RETURN:
                                SDL_StopTextInput();
                                canWrite = false, creating_task = false;
                                char finalfinalforrealv2[1024];
                                strcpy(finalfinalforrealv2, "\n0;");
                                strcat(finalfinalforrealv2, text);
                                // printf("final %s\n", finalfinalforrealv2);
                                FILE *file = fopen("tiadiba.csv", "a");
                                fprintf(file, finalfinalforrealv2);
                                fclose(file);
                                strcpy(text, " ");
                            break;
                        }
                    break;
                }
            }
        }


        /* Draw to window and loop */
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/FPS);
    }

    /* Release resources */
    SDL_DestroyTexture(image_add);
    SDL_DestroyTexture(image_remove);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);    
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}