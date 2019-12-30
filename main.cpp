//
//  main.cpp
//  IsometricTest
//
//  Created by Marat Isaw on 15.12.19.
//  Copyright © 2019 Marat Isaw. All rights reserved.
//

#include <iostream>
#include "common.hpp"
#include "gfx.hpp"
#include "noise.hpp"
#include <SDL2/SDL.h>
//#define DEBUGTIME

// TODO: Rotation

int ihatx = 0;
int ihaty = -1;
int jhatx = 0;
int jhaty = -1;
int currentRotation = 0;
int rotations[4][4] = {
    {  1,  0,  0,  1 },
    {  0,  1, -1,  0 },
    { -1,  0,  0, -1 },
    {  0, -1,  1,  0 }
};

/* Matrix multiplikation
 +-   -+   +- -+       +- -+       +- -+   +-             -+
 | a c | * | x | = x * | a | + y * | c | = | a * x + c * y |
 | b d |   | y |       | b |       | d |   | b * x + d * y |
 +-   -+   +- -+       +- -+       +- -+   +-             -+
 */
void rotateLeft() {
    currentRotation = (currentRotation + 1) % 4;
    
    /*int tx = ihatx;
    int ty = ihaty;
    ihatx = -jhaty;
    ihaty = jhatx;
    jhatx = -ty;
    jhaty = tx;*/
}
void rotateRight() {
    currentRotation = (currentRotation - 1 + 4) % 4;
    /*int tx = ihatx;
    int ty = ihaty;
    ihatx =  0 * jhatx + 1 * jhaty;
    ihaty = -1 * jhatx + 0 * jhaty;
    jhatx =  0 * ihatx + 1 * ihaty;
    jhaty = -1 * ihatx + 0 * ihaty;*/
}


int curx, cury;
int keyx, keyy;

uint32_t oldTicks;
uint32_t newTicks;
uint32_t delta;

struct Block {
    int height;
    
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    
    uint32_t getSequenceColor() {
        return ((this->r << 24) +
                (this->g << 16) +
                (this->b <<  8) +
                 this->a);
    }
    void toColor(uint32_t color) {
        this->r = (color >> 24) & 0xff;
        this->g = (color >> 16) & 0xff;
        this->b = (color >> 8) & 0xff;
        this->a = (color) & 0xff;
    }
};

Block* world;

int getRotatedIndex(int x, int y) {
    int index = 0;
    
    
//    int maxx = ihatx * (WORLD_WIDTH- 1) + jhatx * (WORLD_HEIGHT - 1);
//    int maxy = ihaty * (WORLD_WIDTH- 1) + jhaty * (WORLD_HEIGHT - 1);
//    int minx = (ihatx * 0 + jhatx * 0);
//    int miny = (ihaty * 0 + jhaty * 0);
    
    int nx = ((rotations[currentRotation][0]) * x + (rotations[currentRotation][2]) * y);
    int ny = ((rotations[currentRotation][1]) * x + (rotations[currentRotation][3]) * y);
    
    if(nx < 0) {
        nx += WORLD_WIDTH;
    }
    if(ny < 0) {
        ny += WORLD_HEIGHT;
    }
    
    
    index = nx + ny * WORLD_WIDTH;
    
    return index;
}

int getRotatedX(int x, int y) {
    int nx = (rotations[currentRotation][0] * x + rotations[currentRotation][2] * y);
    
    if(nx < 0) {
        nx += WORLD_HEIGHT;
    }
    
    return nx;
}
int getRotatedY(int x, int y) {
    int ny = (rotations[currentRotation][1] * x + rotations[currentRotation][3] * y);
    
    if(ny < 0) {
        ny += WORLD_HEIGHT;
    }
    
    return ny;
}

int getSimpleRotatedX(int x, int y) {
    return (rotations[currentRotation][0] * x + rotations[currentRotation][2] * y);
}
int getSimpleRotatedY(int x, int y) {
    return (rotations[currentRotation][1] * x + rotations[currentRotation][3] * y);
}

int main(int argc, const char * argv[]) {
    
    for (int i = 0; i < 4; i++) {
            std::cout << "ix: " << rotations[i][0] << " iy: " << rotations[i][1] << " | "  << "jx: " << rotations[i][2] << " jy: " << rotations[i][3] << std::endl;
        
        std::cout<<"-------------------------" << '\n';
    }
    
    std::cout << ((3 + 4) % 4) << std::endl;
    
//    return 0;
    
//    srand(20016316);
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_Event event;
    
    window = SDL_CreateWindow("Isometric World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    int surfaceW = WIDTH / 2;
    int surfaceH = HEIGHT / 2;
    surface = SDL_CreateRGBSurfaceWithFormat(0, surfaceW, surfaceH, 32, SDL_PIXELFORMAT_ABGR32);
    gfx_init(surfaceW, surfaceH);
    SDL_Rect zoomedRect { 0, 0, surfaceW*2, surfaceH*2 };
    
//    SDL_ShowCursor(SDL_DISABLE);
    
    int octaves = 20;
    int size = 200;
    int bufferW = WORLD_WIDTH;
    int bufferH = WORLD_HEIGHT;
    float** heightmap;
    heightmap = generateNoise(bufferW, bufferH, size, octaves);
    
    float** falloffValues;
    falloffValues = generateFallOffMap(bufferW, bufferH, 2);
    
    for (int i = 0; i < bufferW; i++)
        for (int j = 0; j < bufferH; j++)
            heightmap[j][i] = abs(heightmap[j][i]);
    
    heightmap = combineNoise_sub(heightmap, falloffValues, bufferW, bufferH);
    
    int blockHeight = TILE_HEIGHT;
    
    world = new Block[WORLD_WIDTH * WORLD_HEIGHT] { 0 };
    for(int i = 0; i < WORLD_WIDTH; i++) {
        for(int j = 0; j < WORLD_HEIGHT; j++) {
            int index = i + j * WORLD_WIDTH;
            world[index].height = 0;
            
            world[index].height = (int)(heightmap[j][i] * 5.0f) * blockHeight;


            if(heightmap[j][i] > 0.90)
                world[index].height = (int)(heightmap[j][i]/(heightmap[j][i]-0.05f) * 6.0f) * blockHeight;
            if(heightmap[j][i] > 0.97)
                world[index].height = (int)(heightmap[j][i]/(heightmap[j][i]-0.1f) * 7.0f) * blockHeight;

            if(heightmap[j][i] <= 0.17)
                world[index].height = 0;

            if(heightmap[j][i] > 0.17)
                world[index].height += blockHeight/2;
            

            float& currentBufferValue = heightmap[j][i];
            
            if(currentBufferValue < 0.05f)
            {
                world[index].r = 0x33;
                world[index].g = 0x66;
                world[index].b = 0xFF;
                
            }
            else if(currentBufferValue > 0.05f && currentBufferValue < 0.17f)
            {
                world[index].r = lerp(0x33/(float)0xff, 0x44/(float)0xff, currentBufferValue/0.25)*0xff;
                world[index].g = lerp(0x66/(float)0xff, 0x85/(float)0xff, currentBufferValue/0.25)*0xff;
                world[index].b = 0xff;
                
            }
            else if(currentBufferValue > 0.17f && currentBufferValue < 0.25f)
            {
                world[index].r = 0xDD;
                world[index].g = 0xDD;
                world[index].b = 0x44;
            }
            else if(currentBufferValue > 0.25f && currentBufferValue < 0.60f)
            {
                world[index].r = 0x22;
                world[index].g = 0xAA;
                world[index].b = 0x55;
            }
            else if(currentBufferValue > 0.60f && currentBufferValue < 0.90f)
            {
                world[index].r = 0x00;
                world[index].g = 0x8A;
                world[index].b = 0x35;
            }
            else if(currentBufferValue > 0.90f && currentBufferValue < 0.94f)
            {
                world[index].r = 0xA5;
                world[index].g = 0x75;
                world[index].b = 0x45;
            }
            else if(currentBufferValue > 0.94f && currentBufferValue < 0.97f)
            {
                world[index].r = 0x85;
                world[index].g = 0x55;
                world[index].b = 0x25;
            }
            else if(currentBufferValue > 0.97)
            {
                world[index].r = 0xff;
                world[index].g = 0xff;
                world[index].b = 0xff;
            }
            world[index].a = 0xff;
            world[index].r = clamp(world[index].r + (rand()%10-5), 0, 255);
            world[index].g = clamp(world[index].g + (rand()%10-5), 0, 255);
            world[index].b = clamp(world[index].b + (rand()%10-5), 0, 255);
            
//            world[index].r = clamp(world[index].r, 0, 255);
//            world[index].g = clamp(world[index].g, 0, 255);
//            world[index].b = clamp(world[index].b, 0, 255);
        }
    }
    
    
    uint32_t currentColor = -1;
    bool changeColor = false;
    bool drawOutline = true;
    bool border = true;
    int worldCursorX = 0, worldCursorY = 0;
    
    float mouseClickTimer = 7;
    float mouseClickTimerDelta = 0;

//    float t = 0;

    bool alive = true;
    while(alive) {
        oldTicks = newTicks;
        newTicks = SDL_GetTicks();
        
        while(SDL_PollEvent(&event)) {
            
            SDL_PumpEvents();
            Uint8 *keystate = (Uint8*)SDL_GetKeyboardState(NULL);
            
            switch(event.type) {
                case SDL_QUIT:
                    alive = false;
                    break;
                
                case SDL_KEYDOWN:
                    
                    if(keystate[SDL_SCANCODE_UP]) {
                        ORIGIN_Y += 4;
                    }
                    if(keystate[SDL_SCANCODE_DOWN]) {
                        ORIGIN_Y -= 4;
                    }
                    if(keystate[SDL_SCANCODE_LEFT]) {
                        ORIGIN_X += 2;
                    }
                    if(keystate[SDL_SCANCODE_RIGHT]) {
                        ORIGIN_X -= 2;
                    }
                    
                    if(keystate[SDL_SCANCODE_W]) {
                        if(!keystate[SDL_SCANCODE_LSHIFT])
                            ORIGIN_Y += 0.5;
                        else
                            ORIGIN_Y += 2;
                    }
                    if(keystate[SDL_SCANCODE_S]) {
                        if(!keystate[SDL_SCANCODE_LSHIFT])
                            ORIGIN_Y -= 0.5;
                        else
                            ORIGIN_Y -= 2;
                    }
                    if(keystate[SDL_SCANCODE_A]) {
                        if(!keystate[SDL_SCANCODE_LSHIFT])
                            ORIGIN_X += 0.5;
                        else
                            ORIGIN_X += 2;
                    }
                    if(keystate[SDL_SCANCODE_D]) {
                        if(!keystate[SDL_SCANCODE_LSHIFT])
                            ORIGIN_X -= 0.5;
                        else
                            ORIGIN_X -= 2;
                    }
                    if(keystate[SDL_SCANCODE_Q]) {
                        rotateLeft();
                        
                        
                        float oldX = ORIGIN_X - ORIGIN_X_CENTER_OFFSET;
                        float oldY = ORIGIN_Y - ORIGIN_Y_CENTER_OFFSET;
                        float newX = oldY;
                        float newY = -oldX;
                        ORIGIN_X = newX + ORIGIN_X_CENTER_OFFSET;
                        ORIGIN_Y = newY + ORIGIN_Y_CENTER_OFFSET;
                    }
                    if(keystate[SDL_SCANCODE_E]) {
                        rotateRight();
                        
                        float oldX = ORIGIN_X - ORIGIN_X_CENTER_OFFSET;
                        float oldY = ORIGIN_Y - ORIGIN_Y_CENTER_OFFSET;
                        float newX = -oldY;
                        float newY = oldX;
                        ORIGIN_X = newX + ORIGIN_X_CENTER_OFFSET;
                        ORIGIN_Y = newY + ORIGIN_Y_CENTER_OFFSET;
                    }
                    
                    
                    if(keystate[SDL_SCANCODE_0]) {
                        currentColor = 0x000000ff;
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_1]) {
                        currentColor = 0xff0000ff;
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_2]) {
                        currentColor = 0x00ff00ff;
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_3]) {
                        currentColor = 0x0000ffff;
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_4]) {
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_5]) {
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_6]) {
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_7]) {
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_8]) {
                        currentColor = 0x333333ff;
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_9]) {
                        currentColor = 0xffffffff;
                        changeColor = true;
                    }
                    if(keystate[SDL_SCANCODE_C]) {
                        currentColor = world[worldCursorX + worldCursorY * WORLD_WIDTH].getSequenceColor();
                    }
                    if(keystate[SDL_SCANCODE_X]) {
                        changeColor = false;
                    }
                    
                    if(event.key.keysym.sym == SDLK_PLUS) {
                        
                        TILE_WIDTH *= 2;
                        if(blockHeight == 0) blockHeight = 1;
                        blockHeight *= 2;
                        TILE_WIDTH_HALF = TILE_WIDTH / 2;
                        TILE_HEIGHT = TILE_WIDTH_HALF;
                        TILE_HEIGHT_HALF = TILE_HEIGHT / 2;
                        
                        for(int i = 0; i < WORLD_WIDTH; i++) {
                            for(int j = 0; j < WORLD_HEIGHT; j++) {
                                int index = i + j * WORLD_WIDTH;
                                world[index].height *= 2;
                            }
                        }
                        
                        ORIGIN_X_CENTER_OFFSET = std::round(WIDTH / 2 / TILE_WIDTH / 2 * 2) / 2.0f - 0.5;
                        ORIGIN_Y_CENTER_OFFSET = std::round(((HEIGHT / 2 / TILE_HEIGHT / 2)  - WORLD_HEIGHT/2)*2.0f) / 2.0f - 0.5;
                        ORIGIN_X -= surfaceW / 2 / TILE_WIDTH;
                        ORIGIN_Y -= surfaceH / 2 / TILE_HEIGHT;
                    }
                    if(event.key.keysym.sym == SDLK_MINUS) {
                        if(blockHeight / 2 > 2) {
                            TILE_WIDTH /= 2;
                            blockHeight /= 2;
                            TILE_WIDTH_HALF = TILE_WIDTH / 2;
                            TILE_HEIGHT = TILE_WIDTH_HALF;
                            TILE_HEIGHT_HALF = TILE_HEIGHT / 2;

                            for(int i = 0; i < WORLD_WIDTH; i++) {
                                for(int j = 0; j < WORLD_HEIGHT; j++) {
                                    int index = i + j * WORLD_WIDTH;
                                    world[index].height /= 2;
                                }
                            }

                            ORIGIN_X_CENTER_OFFSET = std::round(WIDTH / 2 / TILE_WIDTH / 2 * 2) / 2.0f - 0.5;
                            ORIGIN_Y_CENTER_OFFSET = std::round(((HEIGHT / 2 / TILE_HEIGHT / 2)  - WORLD_HEIGHT/2)*2.0f) / 2.0f - 0.5;
                            ORIGIN_X += surfaceW / 4 / TILE_WIDTH;
                            ORIGIN_Y += surfaceH / 4 / TILE_HEIGHT;
                        }
                        else {
                        }
                            
                    }
                    if(event.key.keysym.sym == SDLK_o) {
                        drawOutline = !drawOutline;
                    }
                    if(event.key.keysym.sym == SDLK_b) {
                        border = !border;
                    }
                    break;
                    
                case SDL_KEYUP:
                    break;
            }
        }
        
        mouseClickTimerDelta++;
        
        if(SDL_GetMouseState(&curx, &cury) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if(worldCursorX != -1 && worldCursorY != -1) {
                if(changeColor)
                    world[getRotatedIndex(worldCursorX, worldCursorY)].toColor(currentColor);
                
                if(!border)
                    world[getRotatedIndex(worldCursorX, worldCursorY)].height += 1;
                else if(mouseClickTimerDelta > mouseClickTimer){
                    world[getRotatedIndex(worldCursorX, worldCursorY)].height += blockHeight;
                    mouseClickTimerDelta = 0;
                }
            }
        }
        
        else if (SDL_GetMouseState(&curx, &cury) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            if(worldCursorX != -1 && worldCursorY != -1) {
                    
                if(!border && world[getRotatedIndex(worldCursorX, worldCursorY)].height - 1 >= 0)
                    world[getRotatedIndex(worldCursorX, worldCursorY)].height -= 1;
                else if(mouseClickTimerDelta > mouseClickTimer && world[getRotatedIndex(worldCursorX, worldCursorY)].height - blockHeight >= 0) {
                    world[getRotatedIndex(worldCursorX, worldCursorY)].height -= blockHeight;
                    mouseClickTimerDelta = 0;
                }
            }
            if(changeColor)
                world[getRotatedIndex(worldCursorX, worldCursorY)].toColor(currentColor);
        }
        
        
        
//        fx = (((curx-TILE_WIDTH/2-ORIGIN_X*TILE_WIDTH) / (float)TILE_WIDTH) + ( (cury-ORIGIN_Y*TILE_HEIGHT) / (float)TILE_HEIGHT) );
//        fy = (((cury-ORIGIN_Y*TILE_HEIGHT) / (float)TILE_HEIGHT) -(curx-TILE_WIDTH/2- ORIGIN_X*TILE_WIDTH) / (float)TILE_WIDTH);
        curx /= 2;
        cury /= 2;
        worldCursorX = ((curx - TILE_WIDTH_HALF - ORIGIN_X * TILE_WIDTH) / (float)TILE_WIDTH_HALF + (cury - ORIGIN_Y * TILE_HEIGHT) / (float)TILE_HEIGHT_HALF) / 2;
        worldCursorY = ((cury - ORIGIN_Y * TILE_HEIGHT) / (float)TILE_HEIGHT_HALF - ((curx - TILE_WIDTH_HALF - ORIGIN_X * TILE_WIDTH) / (float)TILE_WIDTH_HALF)) / 2;
//        int oldCursorX = worldCursorX;
//        worldCursorX = getRotatedX(worldCursorX, worldCursorY);
//        worldCursorY = getRotatedY(oldCursorX, worldCursorY);
        
        
//        int oldX = worldCursorX;
//        int oldY = worldCursorY;
//        worldCursorX = getRotatedX(oldX, oldY);
//        worldCursorY = getRotatedY(oldX, oldY);
        
//        if(worldCursorX < 0 || worldCursorX >= WORLD_WIDTH) worldCursorX = -1;
//        if(worldCursorY < 0 || worldCursorY >= WORLD_HEIGHT) worldCursorY = -1;
        
        
        
        
        
        uint32_t* pixels = (uint32_t*)surface->pixels;
        for(int i = 0; i < surfaceW; i++) {
            for(int j = 0; j < surfaceH; j++) {
                int index = i + j * surfaceW;
                pixels[index] = 0x000000ff;
            }
        }
        
//        fillRectangle(pixels, 0x33AA55FF, 0, 0, 50, 50);
//        drawLineAdd(pixels, 0x333333ff, 0, 0, 60, 60);
        
        
//        t += 0.08f;
        for(int j = 0; j < WORLD_HEIGHT; j++) {
            for(int i = 0; i < WORLD_WIDTH; i++) {
                
                
                
                int height = world[getRotatedIndex(i, j)].height;
                
//                float height = (sin(i + t) + 1) * 10; // SINE WAVE
                int x = worldToScreenX(i, j);
                int y = worldToScreenY(i, j) - height;
                
                // skip those that are outside of the screen
                if(x + TILE_WIDTH_HALF < 0 || x - TILE_WIDTH_HALF > surfaceW ||
                   y + height + TILE_HEIGHT < 0 || y - TILE_HEIGHT_HALF >= surfaceH) {
                    continue;
                }

                if(i + 1 < WORLD_WIDTH && j + 1 < WORLD_HEIGHT && height != 0 && height < world[getRotatedIndex(i+1, j+1)].height - TILE_HEIGHT)
                    continue;
                
                int yr = world[getRotatedIndex(i, j)].r;
                int yg = world[getRotatedIndex(i, j)].g;
                int yb = world[getRotatedIndex(i, j)].b;
                int ya = world[getRotatedIndex(i, j)].a/2.5;
                
                int xr = world[getRotatedIndex(i, j)].r;
                int xg = world[getRotatedIndex(i, j)].g;
                int xb = world[getRotatedIndex(i, j)].b;
                int xa = world[getRotatedIndex(i, j)].a/1.5;
                
                int tr = world[getRotatedIndex(i, j)].r;
                int tg = world[getRotatedIndex(i, j)].g;
                int tb = world[getRotatedIndex(i, j)].b;
                int ta = world[getRotatedIndex(i, j)].a;
                

                
                /* TRIANGLE RENDERING: SLOW */
                // y side
//                drawTriangle(pixels,
//                             (yr << 24) + (yg << 16) + (yb << 8) + ya,
//                             (yr << 24) + (yg << 16) + (yb << 8) + ya,
//                             (yr << 24) + (yg << 16) + (yb << 8) + ya,
//                             x - TILE_WIDTH / 2, y + TILE_HEIGHT / 2,
//                             x - TILE_WIDTH / 2, y + TILE_HEIGHT / 2 + height,
//                             x, y + TILE_HEIGHT + height);
//                drawTriangle(pixels,
//                             (yr << 24) + (yg << 16) + (yb << 8) + ya,
//                             (yr << 24) + (yg << 16) + (yb << 8) + ya,
//                             (yr << 24) + (yg << 16) + (yb << 8) + ya,
//                             x - TILE_WIDTH / 2, y + TILE_HEIGHT / 2,
//                             x, y + TILE_HEIGHT,
//                             x, y + TILE_HEIGHT + height);
//
//                // x side
//                drawTriangle(pixels,
//                             (xr << 24) + (xg << 16) + (xb << 8) + xa,
//                             (xr << 24) + (xg << 16) + (xb << 8) + xa,
//                             (xr << 24) + (xg << 16) + (xb << 8) + xa,
//                             x, y + TILE_HEIGHT,
//                             x, y + TILE_HEIGHT + height,
//                             x + TILE_WIDTH / 2, y + TILE_HEIGHT / 2);
//                drawTriangle(pixels,
//                             (xr << 24) + (xg << 16) + (xb << 8) + xa,
//                             (xr << 24) + (xg << 16) + (xb << 8) + xa,
//                             (xr << 24) + (xg << 16) + (xb << 8) + xa,
//                             x + TILE_WIDTH / 2, y + TILE_HEIGHT / 2 + height,
//                             x, y + TILE_HEIGHT + height,
//                             x + TILE_WIDTH / 2, y + TILE_HEIGHT / 2);
                
//                if(i + 1 < WORLD_WIDTH && height < world[(i+1) + j * WORLD_WIDTH])
//                    continue;
//                if(j + 1 < WORLD_HEIGHT && height < world[i + (j+1) * WORLD_WIDTH])
//                    continue;
                
                
                
                int outlinecolor = ((tr+xr+yr) / 3 << 24) + ((tg+xg+yg) / 3 << 16) + ((tb+xb+yb) / 3 << 8) + ta / 1.75;
                int miny = height;
                int minx = height;
                
//                if(i + 1 < WORLD_WIDTH && world[(i+1) + j * WORLD_WIDTH].height != 0)
//                    minx = std::min(miny, abs(height - world[(i+1) + j * WORLD_WIDTH].height));
//                if(j + 1 < WORLD_HEIGHT && world[i + (j+1) * WORLD_WIDTH].height != 0)
//                    miny = std::min(miny, abs(height - world[i + (j+1) * WORLD_WIDTH].height));
//                if(i + 1 < WORLD_WIDTH && j + 1 < WORLD_HEIGHT && world[(i+1) + (j+1) * WORLD_WIDTH] != 0)
//                    miny = minx = std::min(miny, abs((int)(height - world[(i+1) + (j+1) * WORLD_WIDTH] + TILE_HEIGHT)));
                
                // Sides
                for(int k = 0; k < TILE_WIDTH_HALF; k++) {

                    gfx_drawLine(pixels, ((xr << 24) + (xg << 16) + (xb << 8) + xa),
                             x + k, y + TILE_HEIGHT + k * -0.5,
                             x + k, y + TILE_HEIGHT + k * -0.5 + minx+1);

                    gfx_drawLine(pixels, ((yr << 24) + (yg << 16) + (yb << 8) + ya),
                             x - TILE_WIDTH_HALF + k, y + TILE_HEIGHT_HALF + k * 0.5,
                             x - TILE_WIDTH_HALF + k, y + TILE_HEIGHT_HALF + k * 0.5 + miny+1);
                }
                
                /*if(!drawOutline) {
                    for(int k = 0; k < height; k++) {
                        if(y + TILE_HEIGHT + k < HEIGHT)
                            pixels[x + (int)(y + TILE_HEIGHT + k) * WIDTH] = outlinecolor;
                        
                    }

                    for(int k = 0; k < miny; k++) {
                        if(x - TILE_WIDTH_HALF >= 0 && y + TILE_HEIGHT_HALF + k < HEIGHT)
                            pixels[x - (int)TILE_WIDTH_HALF + (int)(y + TILE_HEIGHT_HALF + k) * WIDTH] = outlinecolor;
                    }
                    
                    for(int k = 0; k < minx; k++) {
                        if(x + TILE_WIDTH_HALF < WIDTH && y + TILE_HEIGHT_HALF + k < HEIGHT)
                            pixels[x + (int)TILE_WIDTH_HALF + (int)(y + TILE_HEIGHT_HALF + k) * WIDTH] = outlinecolor;
                    }
                }*/
                
                
                // top
//                drawTriangle(pixels,
//                             (tr << 24) + (tg << 16) + (tb << 8) + ta,
//                             (tr << 24) + (tg << 16) + (tb << 8) + ta,
//                             (tr << 24) + (tg << 16) + (tb << 8) + ta,
//                             x            , y,
//                             x + TILE_WIDTH_HALF, y + TILE_HEIGHT_HALF,
//                             x            , y + TILE_HEIGHT);
//                drawTriangle(pixels,
//                             (tr << 24) + (tg << 16) + (tb << 8) + ta,
//                             (tr << 24) + (tg << 16) + (tb << 8) + ta,
//                             (tr << 24) + (tg << 16) + (tb << 8) + ta,
//                             x            , y,
//                             x - TILE_WIDTH_HALF, y + TILE_HEIGHT_HALF,
//                             x            , y + TILE_HEIGHT);
                
                // new better top
                for(int i = 0; i < TILE_HEIGHT_HALF; i++) {
                    gfx_drawLine(pixels, ((tr << 24) + (tg << 16) + (tb << 8) + ta),
                                 x - TILE_WIDTH_HALF + i * 2, y + TILE_HEIGHT_HALF-i,
                                 x + TILE_WIDTH_HALF - i * 2, y + TILE_HEIGHT_HALF-i);
                    gfx_drawLine(pixels, ((tr << 24) + (tg << 16) + (tb << 8) + ta),
                                 x - TILE_WIDTH_HALF + i * 2, y + TILE_HEIGHT_HALF+i,
                                 x + TILE_WIDTH_HALF - i * 2, y + TILE_HEIGHT_HALF+i);
                }
                
//                for(int i = 0; i < TILE_WIDTH_HALF; i++) {
//                    gfx_drawLine(pixels, 0xff0000ff, x-TILE_WIDTH_HALF, y+TILE_HEIGHT_HALF, x+TILE_WIDTH_HALF, y+TILE_HEIGHT_HALF);
//                    gfx_drawLine(pixels, ((tr << 24) + (tg << 16) + (tb << 8) + ta),
//                             (x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i), y                 + i * 0.5 ,
//                             (x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i), (y + TILE_HEIGHT) - i * 0.5);

//                    gfx_drawLine(pixels, ((tr << 24) + (tg << 16) + (tb << 8) + ta),
//                             (x) + i, (y + i * 0.5),
//                             (x) + i, (y + TILE_HEIGHT) - i * 0.5);

                    /*if(!drawOutline) {
                    if(x + i >= 0 &&
                       x + i < WIDTH &&
                       y + (int)(i * 0.5) >= 0 &&
                       y + (int)(i * 0.5) < HEIGHT)
                        pixels[(x + i) + (y + (int)(i * 0.5)) * WIDTH] = outlinecolor;

                    if((x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i) >= 0 &&
                       (x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i) < WIDTH &&
                       (y + TILE_HEIGHT) - (int)(i * 0.5) >= 0 &&
                       (y + TILE_HEIGHT) - (int)(i * 0.5) < HEIGHT)
                        pixels[(int)((x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i)) + ((int)(y + TILE_HEIGHT) - (int)(i * 0.5)) * WIDTH] = outlinecolor;

                    if((x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i) >= 0 &&
                       (x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i) < WIDTH &&
                       y + (int)(i * 0.5) >= 0 &&
                       y + (int)(i * 0.5) < HEIGHT)
                        pixels[(int)((x - TILE_WIDTH_HALF) + (TILE_WIDTH_HALF - i)) + (y + (int)(i * 0.5)) * WIDTH] = outlinecolor;

                    if(x + i >= 0 &&
                       x + i < WIDTH &&
                       (y + TILE_HEIGHT) - (int)(i * 0.5) >= 0 &&
                       (y + TILE_HEIGHT) - (int)(i * 0.5) < HEIGHT)
                        pixels[(int)(x + i) + ((int)(y + TILE_HEIGHT) - (int)(i * 0.5)) * WIDTH] = outlinecolor;
                    }*/
//                }
                
                
                if(drawOutline) {
                    // top
                    gfx_drawLine(pixels, outlinecolor,
                             x,                   y,
                             x + TILE_WIDTH_HALF, y + TILE_HEIGHT_HALF);
                    gfx_drawLine(pixels, outlinecolor,
                             x,                   y,
                             x - TILE_WIDTH_HALF, y + TILE_HEIGHT_HALF);
                    gfx_drawLine(pixels, outlinecolor,
                             x,                   y + TILE_HEIGHT,
                             x + TILE_WIDTH_HALF, y + TILE_HEIGHT_HALF);
                    gfx_drawLine(pixels, outlinecolor,
                             x,                   y + TILE_HEIGHT,
                             x - TILE_WIDTH_HALF, y + TILE_HEIGHT_HALF);
                        
//                    if(height != world[(i+1) + j * WORLD_WIDTH] && height != world[i + (j+1) * WORLD_WIDTH] && height != world[(i+1) + (j+1) * WORLD_WIDTH]) {
                        // x side
//                        drawLine(pixels, outlinecolor,
//                                 x, y + TILE_HEIGHT,
//                                 x, y + TILE_HEIGHT + minx);
                        gfx_drawLine(pixels, outlinecolor,
                                 x + TILE_WIDTH / 2, y + TILE_HEIGHT / 2,
                                 x + TILE_WIDTH / 2, y + TILE_HEIGHT / 2 + minx);
                        
                        // y side
                        gfx_drawLine(pixels, outlinecolor,
                                 x, y + TILE_HEIGHT,
                                 x, y + TILE_HEIGHT + miny);
                        gfx_drawLine(pixels, outlinecolor,
                                 x - TILE_WIDTH / 2, y + TILE_HEIGHT / 2,
                                 x - TILE_WIDTH / 2, y + TILE_HEIGHT / 2 + miny);
//                    }
                    
                     
                    
                    
                    
                    
                }
            }
        }
        
        // SELECT HEIGHT
        bool isLeft = (curx < (worldToScreenX(worldCursorX, worldCursorY)));
        
        int bx = worldCursorX + std::min(WORLD_WIDTH - 1 - worldCursorX, WORLD_HEIGHT - 1 - worldCursorY); // BOTTOM
        int by = worldCursorY + std::min(WORLD_WIDTH - 1 - worldCursorX, WORLD_HEIGHT - 1 - worldCursorY);
        
        if(!isLeft && bx < WORLD_WIDTH - 1) {
            bx++;
            isLeft = !isLeft;
        } else if(isLeft && bx == WORLD_WIDTH - 1 && by <= WORLD_HEIGHT - 2) {
            by++;
            isLeft = !isLeft;
        }
        
            
        int sx = 0;
        int sy = 0;
        
        int lx = bx;
        int ly = by;
        int t = isLeft;

        while(lx >= 0 && ly >= 0)
        {
            sx = worldToScreenX(lx, ly);
            sy = worldToScreenY(lx, ly) - world[getRotatedIndex(lx, ly)].height;

#ifdef DEBUGTIME
            gfx_drawTriangle(pixels,
                         0x3DD0DDff,
                         0x3DD0DDff,
                         0x3DD0DDff,
                         sx, sy,
                         sx + TILE_WIDTH_HALF, sy + TILE_HEIGHT / 2,
                         sx, sy + TILE_HEIGHT);
            
            gfx_drawTriangle(pixels,
                         0x30D0DDff,
                         0x30D0DDff,
                         0x30D0DDff,
                         sx, sy,
                         sx - TILE_WIDTH_HALF, sy + TILE_HEIGHT / 2,
                         sx, sy + TILE_HEIGHT);
#endif
            
            float nx = 0; // new x
            
            if(t) {
                nx = sx - TILE_WIDTH_HALF; // new x
            }
            else {
                nx = sx + TILE_WIDTH_HALF;
            }
            
            float ny = sy + TILE_HEIGHT_HALF;
            
            float slope = (ny - sy) / (nx - sx);
            float b = ny - slope * nx;
            float pointY = slope * curx + b;

            if(cury > pointY)
            {
                worldCursorX = lx;
                worldCursorY = ly;
                break;
            }

            
            if(t) {
                lx--;
            } else {
                ly--;
            }

            t = !t;
            
//            pixels[sx + sy * WIDTH] = 0xff0000ff;
            
        }

        // Draw selected tile
        if(worldCursorX >= 0 && worldCursorX < WORLD_WIDTH &&
           worldCursorY >= 0 && worldCursorY < WORLD_HEIGHT) {
            int selectedX = worldToScreenX(worldCursorX, worldCursorY);
            int selectedY = worldToScreenY(worldCursorX, worldCursorY) - world[getRotatedIndex(worldCursorX, worldCursorY)].height;

            for(int i = 0; i < TILE_HEIGHT_HALF; i++) {
                gfx_drawLine(pixels, currentColor,
                             selectedX - TILE_WIDTH_HALF + i * 2, selectedY + TILE_HEIGHT_HALF-i,
                             selectedX + TILE_WIDTH_HALF - i * 2, selectedY + TILE_HEIGHT_HALF-i);
                gfx_drawLine(pixels, currentColor,
                             selectedX - TILE_WIDTH_HALF + i * 2, selectedY + TILE_HEIGHT_HALF+i,
                             selectedX + TILE_WIDTH_HALF - i * 2, selectedY + TILE_HEIGHT_HALF+i);
                
            }
        }
        
#ifdef DEBUGTIME
        sx = worldToScreenX(bx, by);
        sy = worldToScreenY(bx, by);
        
        gfx_drawTriangle(pixels,
                     0xff00ffff,
                     0xff00ffff,
                     0xff00ffff,
                     sx, sy,
                     sx + TILE_WIDTH_HALF, sy + TILE_HEIGHT / 2,
                     sx, sy + TILE_HEIGHT);
        
        gfx_drawTriangle(pixels,
                     0xff00ffff,
                     0xff00ffff,
                     0xff00ffff,
                     sx, sy,
                     sx - TILE_WIDTH_HALF, sy + TILE_HEIGHT / 2,
                     sx, sy + TILE_HEIGHT);
#endif
        
//        std::cout << "curx: " << curx  << " cury: " << cury  << std::endl;
//        std::cout << "worldx: " << worldCursorX  << " worldy: " << worldCursorY  << std::endl;
//        std::cout << "isLeft: " << isLeft << std::endl;
        
        std::cout << "x: " << ORIGIN_X - std::round(WIDTH / 2 / TILE_WIDTH / 2 * 2) / 2.0f << std::endl;
        std::cout << "y: " << ORIGIN_Y - std::round(((HEIGHT / 2 / TILE_HEIGHT / 2)  - WORLD_HEIGHT/2)*2.0f) / 2.0f << std::endl;
        
//        int centerX = worldToScreenX(screenToWorldX(WIDTH/2, HEIGHT/2), screenToWorldY(WIDTH/2, HEIGHT/2));
//        int centerY = worldToScreenY(screenToWorldX(WIDTH/2, HEIGHT/2), screenToWorldY(WIDTH/2, HEIGHT/2));

//        pixels[centerX + centerY * surfaceW] = 0xff0000ff;
//        pixels[surfaceW/2 + surfaceH/2 * surfaceW] = 0xff0000ff;
        
        
        delta += (newTicks - oldTicks);
        
        if(delta >= 1000/1.0f){
            std::cout << "fps: " << 1000.0f / (newTicks - oldTicks) << std::endl;
            delta -= 1000;
        }
        
        gfx_drawLine(pixels, 0xff0000ff,
                 0, surfaceH/2,
                 surfaceW, surfaceH/2);

        gfx_drawLine(pixels, 0xff0000ff,
                 surfaceW/2, 0,
                 surfaceW/2, surfaceH);
        
//        if(1000.0f / (newTicks - oldTicks) < 30)
//            fillRectangle(pixels, 0xDD7733ff, 0, 0, 64, 64);
//        else if(1000.0f / (newTicks - oldTicks) > 30 && 1000.0f / (newTicks - oldTicks) < 60)
//            fillRectangle(pixels, 0xDDBB33ff, 0, 0, 64, 64);
//        else
//            fillRectangle(pixels, 0x44DD33ff, 0, 0, 64, 64);
        
        
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &zoomedRect);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture);
    }
    
    
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/*drawTriangleOutlineAdd(pixels,
                    0x555555ff,
                    x + tileW / 2, y,
                    x + tileW / 2 + tileW / 2, y + tileH / 2,
                    x + tileW / 2, y + tileH);

drawTriangleOutlineAdd(pixels,
                    0x555555ff,
                    x + tileW / 2, y,
                    x + tileW / 2 - tileW / 2, y + tileH / 2,
                    x + tileW / 2, y + tileH);

drawTriangleOutlineAdd(pixels,
                    0x555555ff,
                    x + tileW / 2, y + tileH,
                    x + tileW / 2, y + tileH + height,
                    x + tileW / 2 + tileW / 2, y + tileH / 2);

drawTriangleOutlineAdd(pixels,
                    0x555555ff,
                    x + tileW / 2 + tileW / 2, y + tileH / 2 + height,
                    x + tileW / 2, y + tileH + height,
                    x + tileW / 2 + tileW / 2, y + tileH / 2);
      
drawTriangleOutlineAdd(pixels,
                    0x555555ff,
                    x + tileW / 2 - tileW / 2, y + tileH / 2,
                    x + tileW / 2 - tileW / 2, y + tileH / 2 + height,
                    x + tileW / 2, y + tileH + height);

drawTriangleOutlineAdd(pixels,
                    0x555555ff,
                    x + tileW / 2 - tileW / 2, y + tileH / 2,
                    x + tileW / 2, y + tileH,
                    x + tileW / 2, y + tileH + height);*/
