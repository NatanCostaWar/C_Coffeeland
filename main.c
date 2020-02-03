#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef struct Bee_Texture Bee_Texture;

typedef struct MOUSE{
    int x;
    int y;
    int moving;

    struct
    {
        int state;
        int x;
        int y;
        int w;
        int h;
    }selection;

}MOUSE;

typedef struct System_file{
    int w;
    int h;
    char* path;
    char** buffer;
}System_file;

typedef struct System_texture_rect{
    SDL_Rect source;
    SDL_Rect destination;
}System_texture_rect;

typedef struct System_camera{
    int x;
    int y;
}System_camera;

SDL_Rect System_CreateRect(int w, int h, int x, int y){
    SDL_Rect bee_rect;

    bee_rect.x = x;
    bee_rect.y = y;
    bee_rect.h = h;
    bee_rect.w = w;

    return bee_rect;
}

SDL_Texture* System_CreateTexture(SDL_Renderer* renderer, char* path){
    SDL_Texture* bee_texture;
    SDL_Surface* bee_surface;

    bee_surface = IMG_Load(path);
    bee_texture = SDL_CreateTextureFromSurface(renderer, bee_surface);

    SDL_FreeSurface(bee_surface);
    return bee_texture;
}

System_file System_FileOpen(char* map_name){
    FILE* file = fopen(map_name, "r");
    int ch;
    int w=0;
    int h=1;
    int i=0;
    int j=0;
    System_file map;
    map.path = map_name;
    while((ch = fgetc(file))!=-1){
        if(ch == '\n')
        {
            h=h+1;
            w=0;
        }else{
            w=w+1;
        }
    }

    map.w = w;
    map.h = h;



    map.buffer = malloc(sizeof(char*)*h);
    for(i=0;i<h;i++){
        map.buffer[i] = malloc(sizeof(char)*w);
    }
    rewind(file);

    i=0;
    j=0;

    while((ch = fgetc(file))!=-1){
        if(ch == '\n')
        {
            i=i+1;
            j=0;
        }else{
            map.buffer[i][j] = ch;
            j=j+1;
        }
    }

    fclose(file);

    return map;
}


int main(int argc,char** argv)
{
    /*Loop Variables*/
    int quit = 0;
    int i;
    int j;
    uint32 startingTick;

    /*Config Variables*/
    SDL_Renderer* renderer = NULL;
    SDL_Event event;
    SDL_Window* window;
    const int FRAME_RATE = 60;
    const int WINDOW_WIDTH = 640;
    const int WINDOW_HEIGHT = 360;
    const uint8 *KEYBOARD;
    int renderWidth = 640;
    int renderHeight = 360;
    MOUSE mouse;

    /*Game Variables*/
    System_file map;
    System_camera camera;

    /*Game Textures Rects*/
    System_texture_rect floors;
    System_texture_rect walls;
    System_texture_rect objects;
    
    /*Game Textures*/
    SDL_Texture* floor_basic;
    SDL_Texture* wall_basic;
    SDL_Texture* wall_door_basic;
    SDL_Texture* object_box;
    SDL_Texture* object_table_basic;
    SDL_Texture* object_chair_basic;

    
    /*System Start*/
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    /*Creating Game Window*/
    window = SDL_CreateWindow("Coffee", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
    SDL_WINDOW_SHOWN /*| SDL_WINDOW_BORDERLESS| SDL_WINDOW_FULLSCREEN_DESKTOP*/);
    SDL_SetWindowResizable(window, SDL_TRUE);
    
    /*Game Configurations*/
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, renderWidth, renderHeight);

    /*Game Variables Initialization*/
    map = System_FileOpen("resource\\coffee.map");
    camera.x = renderWidth/2;
    camera.y = renderHeight/2;

    /*Game Rects Initialization*/
    floors.source.w = 32;
    floors.source.h = 15;
    floors.source.x = 0;
    floors.source.y = 0;
    floors.destination.w = 32;
    floors.destination.h = 15;

    walls.source.w = 16;
    walls.source.h = 48;
    walls.source.x = 0;
    walls.source.y = 0;
    walls.destination.w = 16;
    walls.destination.h = 48;

    objects.source.w = 32;
    objects.source.h = 32;
    objects.source.x = 0;
    objects.source.y = 0;
    objects.destination.w = 32;
    objects.destination.h = 32;

    /*Game Textures Initialization*/
    floor_basic = System_CreateTexture(renderer, "resource\\floor_basic.png");
    wall_basic = System_CreateTexture(renderer, "resource\\wall_basic.png");
    wall_door_basic = System_CreateTexture(renderer, "resource\\wall_door_basic.png");
    object_box = System_CreateTexture(renderer, "resource\\object_box.png");
    object_table_basic = System_CreateTexture(renderer, "resource\\object_table_basic.png");
    object_chair_basic = System_CreateTexture(renderer, "resource\\object_chair_basic.png");
    
    while(!quit)
    {
        /*Getting the ticks to consolidate the FPS*/
        startingTick = SDL_GetTicks();

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_MOUSEMOTION:
                    mouse.x = event.motion.x;
                    mouse.y = event.motion.y;
                    if (mouse.moving == 1)
                    {
                        camera.x += event.motion.xrel; /*camera.x-((mouse.selection.x-mouse.x)/5);*/
                        camera.y += event.motion.yrel; /*camera.y-((mouse.selection.y-mouse.y)/5);*/
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    mouse.selection.x = mouse.x;
                    mouse.selection.y = mouse.y;
                    mouse.selection.state = 1;
                    mouse.moving = 1;
                    break;

                case SDL_MOUSEBUTTONUP:
                    mouse.selection.w = mouse.x-mouse.selection.x;
                    mouse.selection.h = mouse.y-mouse.selection.y;
                    mouse.selection.state = 0;
                    mouse.moving = 0;
                    break;
                
            }
        }

        /*Rendering the Game*/
        SDL_RenderClear(renderer);

        /*Rendering Walls And Door*/
        for (j = 0; j < map.h; j++)
        {
            for (i = 0; i < map.w; i++)
            {   
                if(j == 0)
                {
                    walls.destination.x = ((j-i)*16)+camera.x;
                    walls.destination.y = (((j+i)*8)-41)+camera.y;
                    SDL_RenderCopy(renderer, wall_basic, &walls.source, &walls.destination);

                    if (i==3){
                        SDL_RenderCopy(renderer, wall_door_basic, &walls.source, &walls.destination);
                    }
                }
                if(i == 0)
                {
                    walls.destination.x = (((j-i)*16)+16)+camera.x;
                    walls.destination.y = (((j+i)*8)-41)+camera.y;
                    SDL_RenderCopyEx(renderer, wall_basic, &walls.source, &walls.destination, 0,NULL,SDL_FLIP_HORIZONTAL);
                }
                floors.destination.x = ((j-i)*16)+camera.x;
                floors.destination.y = ((j+i)*8)+camera.y;
                SDL_RenderCopy(renderer, floor_basic, &floors.source, &floors.destination);
            }
        }

        /*Rendering Objects*/
        for (j = 0; j < map.h; j++)
        {
            for (i = 0; i < map.w; i++)
            {   
                switch(map.buffer[j][i]){
                    case 'b':
                        objects.destination.x = ((j-i)*16)+camera.x;
                        objects.destination.y = (((j+i)*8)-16)+camera.y;
                        SDL_RenderCopy(renderer, object_box, &objects.source, &objects.destination);
                        break;
                    case 't':
                        objects.destination.x = ((j-i)*16)+camera.x;
                        objects.destination.y = (((j+i)*8)-16)+camera.y;
                        SDL_RenderCopy(renderer, object_table_basic, &objects.source, &objects.destination);
                        break;
                    case 'c':
                        objects.destination.x = ((j-i)*16)+camera.x;
                        objects.destination.y = (((j+i)*8)-16)+camera.y;
                        SDL_RenderCopy(renderer, object_chair_basic, &objects.source, &objects.destination);
                        break;
                }
                
            }
        }

        SDL_RenderPresent(renderer);


        /*Setting the FPS*/
        if((1000/FRAME_RATE) > SDL_GetTicks() - startingTick)
        {
            SDL_Delay(1000/FRAME_RATE - (SDL_GetTicks() - startingTick));
        }
    }

    /*System Stop*/
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}