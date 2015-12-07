#include "PCH.hpp"
#include "Main.hpp"

static const Color BLACK(0, 0, 0);
static const Color WHITE(255, 255, 255);
static const Color GRAY(128, 128, 128);
static const Color RED(255, 0, 0);
static const Color GREEN(0, 255, 0);
static const Color BLUE(0, 0, 255);
static const Color CYAN(0, 255, 255);
static const Color MAGENTA(255, 0, 255);

int main(int argc, char** argv)
{
    isRunning = true;

    // Initial x and y position
    position = Vector2D(14.5, 20);
    
    // Initial direction vector
    direction = Vector2D(0, -1);

    // Camera plane
    cameraPlane = Vector2D(1, 0);

    moveSpeed = .2;
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
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        std::cerr << "Renderer could not be created! SDL error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create the screen texture
    screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);

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
        }

        ProcessInput();
        Update();
        Render();
    }

    Quit();
    
    return 0;
}

void ProcessInput()
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    if (currentKeyStates[SDL_SCANCODE_W])
    {
        position.SetY(position.GetY() - 1 * moveSpeed);
    }

    if (currentKeyStates[SDL_SCANCODE_A])
    {
        position.SetX(position.GetX() - 1 * moveSpeed);
    }

    if (currentKeyStates[SDL_SCANCODE_S])
    {
        position.SetY(position.GetY() + 1 * moveSpeed);
    }

    if (currentKeyStates[SDL_SCANCODE_D])
    {
        position.SetX(position.GetX() + 1 * moveSpeed);
    }

    if (currentKeyStates[SDL_SCANCODE_1])
    {
        cameraPlane = cameraPlane * .9;
    }

    if (currentKeyStates[SDL_SCANCODE_2])
    {
        cameraPlane = cameraPlane * 1.1;
    }
}

void Update()
{
    for (int x = 0; x < width; x++)
    {
        DrawLine(Vector2D(x, height / 2), Vector2D(x, height), GRAY);

        // Calculate the ray position and direction
        double cameraX = 2 * x / (double)width - 1.0; // X coordinate, in camera space
        double rayPosX = position.GetX();
        double rayPosY = position.GetY();
        double rayDirX = direction.GetX() + (cameraPlane.GetX() * cameraX);
        double rayDirY = direction.GetY() + (cameraPlane.GetY() * cameraX);

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

            if (map[(mapY * MAP_WIDTH) + mapX] > 0)
            {
                hit = 1;
            }
        }

        // Calculate the distance
        if (side == 0)
        {
            perpWallDist = abs((mapX - position.GetX() + (1 - stepX) / 2) / rayDirX);
        }
        else
        {
            perpWallDist = abs((mapY - position.GetY() + (1 - stepY) / 2) / rayDirY);
        }

        // Calculate the line height
        int lineHeight = abs((int)(height / perpWallDist));

        // Calculate the lowest and highest pixel of the line
        int drawStart = (-lineHeight / 2) + (height / 2);
        int drawEnd = (lineHeight / 2) + (height / 2);

        // Determine the color
        Color color;
        switch (map[(mapY * MAP_WIDTH) + mapX])
        {
            case 1:
                color = RED;
                break;
            case 2:
                color = GREEN;
                break;
            case 3:
                color = BLUE;
                break;
            case 4:
                color = WHITE;
                break;
            default:
                color = MAGENTA;
                break;
        }

        if (side == 1)
        {
            color = Color(color.GetR() / 2, color.GetG() / 2, color.GetB() / 2);
        }
        
        // Wall
        //DrawLine(Vector2D(x, drawStart), Vector2D(x, drawEnd), color);
        DrawVerticalLine(x, drawStart, drawEnd, color);

        // Ray
        DrawLine(Vector2D(rayPosX, rayPosY) * 8, (Vector2D(rayPosX, rayPosY) + Vector2D(rayDirX * perpWallDist, rayDirY * perpWallDist)) * 8, MAGENTA);
        //DrawLine(Vector2D(rayPosX, rayPosY) * 8, (Vector2D(rayPosX, rayPosY) + Vector2D(rayDirX, rayDirY)) * 8, MAGENTA);
    }


    // Direction Line
    // DrawLine(position * 8, (position + direction * 5) * 8, MAGENTA);

    // Camera Plane
    //DrawLine((position + direction - cameraPlane) * 8, ((position + direction + cameraPlane) * 8), MAGENTA);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (map[(y * MAP_WIDTH) + x] != 0)
            {
                DrawRect((x * 8), (y * 8), 8, 8, WHITE);
            }
        }
    }
}

void Render()
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    byte* pPixels;
    int pitch = 0;
    SDL_LockTexture(screenTexture, nullptr, (void**)&pPixels, &pitch);

    memcpy(pPixels, pixels, (width * height) * 4);

    SDL_UnlockTexture(screenTexture);

    SDL_Rect renderRect = { 0, 0, width, height };
    SDL_RenderCopy(renderer, screenTexture, nullptr, &renderRect);

    SDL_RenderPresent(renderer);
    memset(pixels, 0x00, width * height * 4);
}

void Quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(screenTexture);

    SDL_Quit();
}

void SetPixel(int x, int y, Color color)
{
    if (x < 0 || y < 0 || x >= width || y >= height)
    {
        return;
    }

    pixels[((y * width) + x) * 4 + 3] = color.GetR();
    pixels[((y * width) + x) * 4 + 2] = color.GetG();
    pixels[((y * width) + x) * 4 + 1] = color.GetB();
    pixels[((y * width) + x) * 4 + 0] = color.GetA();
}

void DrawVerticalLine(int x, int y1, int y2, Color color)
{
    for (int y = y1; y <= y2; y++)
    {
        SetPixel(x, y, color);
    }
}

void DrawLine(Vector2D start, Vector2D end, Color color)
{
    double x = end.GetX() - start.GetX();
    double y = end.GetY() - start.GetY();
    double length = sqrt(x*x + y*y);

    double addx = x / length;
    double addy = y / length;

    x = start.GetX();
    y = start.GetY();

    for (int i = 0; i < length; i += 1)
    {
        SetPixel(start.GetX(), start.GetY(), color);
        start = start + Vector2D(addx, addy);
    }
}

void DrawRect(int x, int y, int width, int height, Color color)
{
    for (int xIndex = 0; xIndex < width; xIndex++)
    {
        for (int yIndex = 0; yIndex < height; yIndex++)
        {
            SetPixel(x + xIndex, y + yIndex, color);
        }
    }
}