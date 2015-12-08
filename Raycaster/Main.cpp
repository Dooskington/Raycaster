#include "PCH.hpp"
#include "Main.hpp"

static const Color BLACK(0, 0, 0);
static const Color WHITE(255, 255, 255);
static const Color GRAY(128, 128, 128);
static const Color RED(194, 59, 34);
static const Color GREEN(119, 190, 119);
static const Color BLUE(119, 158, 203);
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

    double previousTime;
    double currentTime = SDL_GetTicks();

    SDL_Event event;
    while (isRunning)
    {
        double frameStartTime = SDL_GetTicks();

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

        double frameEndTime = SDL_GetTicks();
        while (true)
        {
            frameEndTime = SDL_GetTicks();
            double frameTime = (double)(frameEndTime - frameStartTime) / 1000;

            // Break out once we use up our time per frame
            if (frameTime >= (1 / FRAMERATE))
            {
                deltaTime = frameTime;
                break;
            }
        }
    }

    Quit();
    
    return 0;
}

void ProcessInput()
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    Vector2D movement(0, 0);

    if (currentKeyStates[SDL_SCANCODE_W])
    {
        movement += direction * moveSpeed;
    }

    if (currentKeyStates[SDL_SCANCODE_A])
    {
        movement += Vector2D(direction.GetY(), -direction.GetX()) * moveSpeed;
    }

    if (currentKeyStates[SDL_SCANCODE_S])
    {
        movement -= direction * moveSpeed;
    }

    if (currentKeyStates[SDL_SCANCODE_D])
    {
        movement += Vector2D(-direction.GetY(), direction.GetX()) * moveSpeed;
    }

    if (currentKeyStates[SDL_SCANCODE_Q])
    {
        rotation -= rotationSpeed * deltaTime;
    }

    if (currentKeyStates[SDL_SCANCODE_E])
    {
        rotation += rotationSpeed * deltaTime;
    }

    if (currentKeyStates[SDL_SCANCODE_1])
    {
        cameraPlane = cameraPlane * .9;
    }

    if (currentKeyStates[SDL_SCANCODE_2])
    {
        cameraPlane = cameraPlane * 1.1;
    }

    position = position + (movement * moveSpeed * deltaTime);
}

void Update()
{
    direction.Rotate(rotation);
    cameraPlane.Rotate(rotation);
    rotation = 0;

    for (int x = 0; x < width; x++)
    {
        DrawLine(Vector2D(x, height / 2), Vector2D(x, height), GRAY);

        // Calculate the ray position and direction
        double cameraX = 2 * (x / (double)width) - 1; // X coordinate, in camera space
        Vector2D rayPosition = position;
        Vector2D rayDirection = direction + (cameraPlane * cameraX);

        Vector2D mapPos((int)rayPosition.GetX(), (int)rayPosition.GetY());

        // The distance from one x or y side to the next x or y side
        double sideDistanceX = sqrt(1.0 + (rayDirection.GetY() * rayDirection.GetY() / (rayDirection.GetX() * rayDirection.GetX())));
        double sideDistanceY = sqrt(1.0 + (rayDirection.GetX() * rayDirection.GetX() / (rayDirection.GetY() * rayDirection.GetY())));

        // The distance from the rays position to the next x or y side
        double nextSideDistanceX;
        double nextSideDistanceY;

        // Which direction to step
        Vector2D stepDirection;

        // Calculate step and initial sideDist
        if (rayDirection.GetX() < 0)
        {
            stepDirection.SetX(-1);
            nextSideDistanceX = (rayPosition.GetX() - mapPos.GetX()) * sideDistanceX;
        }
        else
        {
            stepDirection.SetX(1);
            nextSideDistanceX = (mapPos.GetX() + 1.0 - rayPosition.GetX()) * sideDistanceX;
        }

        if (rayDirection.GetY() < 0)
        {
            stepDirection.SetY(-1);
            nextSideDistanceY = (rayPosition.GetY() - mapPos.GetY()) * sideDistanceY;
        }
        else
        {
            stepDirection.SetY(1);
            nextSideDistanceY = (mapPos.GetY() + 1.0 - rayPosition.GetY()) * sideDistanceY;
        }
        
        bool hit = false;
        int side;

        // Perform DDA
        while (!hit)
        {
            // Jump to the next map square
            if (nextSideDistanceX < nextSideDistanceY)
            {
                nextSideDistanceX += sideDistanceX;
                mapPos.SetX(mapPos.GetX() + stepDirection.GetX());
                side = 0;
            }
            else
            {
                nextSideDistanceY += sideDistanceY;
                mapPos.SetY(mapPos.GetY() + stepDirection.GetY());
                side = 1;
            }

            int tile = GetTile(mapPos);
            if (tile > 0)
            {
                hit = true;
            }
        }

        // Calculate the distance
        double perpWallDistance;
        if (side == 0)
        {
            perpWallDistance = abs((mapPos.GetX() - position.GetX() + (1 - stepDirection.GetX()) / 2) / rayDirection.GetX());
        }
        else
        {
            perpWallDistance = abs((mapPos.GetY() - position.GetY() + (1 - stepDirection.GetY()) / 2) / rayDirection.GetY());
        }

        // Calculate the line height
        int lineHeight = abs((int)(height / perpWallDistance));

        // Calculate the lowest and highest pixel of the line
        int drawStart = (-lineHeight / 2) + (height / 2);
        int drawEnd = (lineHeight / 2) + (height / 2);

        // Determine the color
        Color color;
        int tile = GetTile(mapPos);
        switch (tile)
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
        DrawLine(rayPosition * 8, (rayPosition + Vector2D(rayDirection.GetX() * perpWallDistance, rayDirection.GetY() * perpWallDistance)) * 8, MAGENTA);
        //DrawLine(Vector2D(rayPositionX, rayPositionY) * 8, (Vector2D(rayPositionX, rayPositionY) + Vector2D(rayDirectionX, rayDirectionY)) * 8, MAGENTA);
    }


    // Direction Line
    //DrawLine(position * 8, (position + direction * 5) * 8, MAGENTA);

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

int GetTile(Vector2D position)
{
    return map[((int)position.GetY() * MAP_WIDTH) + (int)position.GetX()];
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