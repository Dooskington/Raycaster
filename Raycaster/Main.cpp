#include <iostream>
#include <SDL.h>
#include "Main.hpp"

int main(int argc, char** argv)
{
    isRunning = true;

    // Initial x and y position
    posX = 22;
    posY = 12;
    
    // Initial direction vector
    dirX = -1;
    dirY = 0;

    // Camera plane
    planeX = 0;
    planeY = 1;

    moveSpeed = 0.5;
    rotationSpeed = 0.2;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create window
    window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cerr << "Window could not be created! SDL error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        std::cerr << "Renderer could not be created! SDL error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Event event;
    while (isRunning)
    {
        // Poll for window input
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_w:
                        posX += dirX * moveSpeed;
                        posY += dirY * moveSpeed;
                        break;
                    case SDLK_a:
                        posY += dirX * moveSpeed;
                        break;
                    case SDLK_d:
                        posY -= dirX * moveSpeed;
                        break;
                    case SDLK_e:
                        oldDirX = dirX;
                        dirX = dirX * cos(-rotationSpeed) - dirY * sin(-rotationSpeed);
                        dirY = oldDirX * sin(-rotationSpeed) + dirY * cos(-rotationSpeed);

                        oldPlaneX = planeX;
                        planeX = planeX * cos(-rotationSpeed) - planeY * sin(-rotationSpeed);
                        planeY = oldPlaneX * sin(-rotationSpeed) + planeY * cos(-rotationSpeed);
                        break;
                    case SDLK_s:
                        posX -= dirX * moveSpeed;
                        posY -= dirY * moveSpeed;
                        break;
                    case SDLK_q:
                        oldDirX = dirX;
                        dirX = dirX * cos(rotationSpeed) - dirY * sin(rotationSpeed);
                        dirY = oldDirX * sin(rotationSpeed) + dirY * cos(rotationSpeed);

                        oldPlaneX = planeX;
                        planeX = planeX * cos(rotationSpeed) - planeY * sin(rotationSpeed);
                        planeY = oldPlaneX * sin(rotationSpeed) + planeY * cos(rotationSpeed);
                        break;
                }
            }
        }

        Update();
        Render();
    }

    Quit();
    
    return 0;
}

void Update()
{
    for (int x = 0; x < width; x++)
    {
        // Calculate the ray position and direction
        double cameraX = 2 * x / (double)width - 1.0; // X coordinate, in camera space
        double rayPosX = posX;
        double rayPosY = posY;
        double rayDirX = dirX + (planeX * cameraX);
        double rayDirY = dirY + (planeY * cameraX);

        int mapX = (int)rayPosX;
        int mapY = (int)rayPosY;

        // Length of ray from one x or y side to the next x or y side
        double sideDistX;
        double sideDistY;

        // Length of ray from one x or y side to the next x or y side
        double deltaDistX = sqrt(1.0 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
        double deltaDistY = sqrt(1.0 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
        double perpWallDist;

        // Which direction to step
        int stepX;
        int stepY;
        
        int hit = 0;
        int side;

        // Calculate step and initial sideDist
        if (rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (rayPosX - mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
        }

        if (rayDirY < 0)
        {
            stepY = -1;
            sideDistY = (rayPosY - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
        }

        // Perform DDA
        while (hit == 0)
        {
            // Jump to the next map square
            if (sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (worldMap[mapX][mapY] > 0)
            {
                hit = 1;
            }
        }

        // Calculate the distance
        if (side == 0)
        {
            perpWallDist = abs((mapX - posX + (1 - stepX) / 2) / rayDirX);
        }
        else
        {
            perpWallDist = abs((mapY - posY + (1 - stepY) / 2) / rayDirY);
        }

        // Calculate the line height
        int lineHeight = abs((int)(height / perpWallDist));

        // Calculate the lowest and highest pixel of the line
        int drawStart = (-lineHeight / 2) + (height / 2);
        if (drawStart < 0)
        {
            drawStart = 0;
        }

        int drawEnd = (lineHeight / 2) + (height / 2);
        if (drawEnd >= height)
        {
            drawEnd = height - 1;
        }

        // Determine the color
        byte color;
        switch (worldMap[mapX][mapY])
        {
            case 1:
                color = 0xFF;
                break;
            case 2:
                color = 0xFF;
                break;
            case 3:
                color = 0xFF;
                break;
            case 4:
                color = 0xFF;
                break;
            default:
                color = 0xFF;
                break;
        }

        if (side == 1)
        {
            color = color / 2;
        }
        
        DrawVerticalLine(x, drawStart, drawEnd, color);

        //if (x % 2 == 0)
        //{
        //    DrawVerticalLine(x, 0, height, 0xFF);
        //}

        //DrawVerticalLine(10, 0, height, 0xFF);
    }
}

void Render()
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    for (int x = 0; x < width; x++)  
    {
        for (int y = 0; y < height; y++)
        {
            byte color = pixels[x][y];
            SDL_SetRenderDrawColor(renderer, color, color, color, 0xFF);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    //SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    //SDL_RenderDrawLine(renderer, posX, posY, posX + dirX * 5, posX + dirY * 5);

    //SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    //SDL_RenderDrawPoint(renderer, posX, posY);

    SDL_RenderPresent(renderer);
    memset(pixels, 0x00, (width * height) * 1);
}

void Quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void SetPixel(int x, int y, byte color)
{
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        return;
    }

    pixels[x][y] = color;
}

void DrawVerticalLine(int x, int y1, int y2, byte color)
{
    if (y2 < 0 || y1 >= height || x < 0 || x >= width)
    {
        return;
    }

    if (y1 < 0)
    {
        y1 = 0;
    }

    if (y2 >= width)
    {
        y2 = width - 1;
    }

    for (int y = y1; y <= y2; y++)
    {
        SetPixel(x, y, color);
    }
}