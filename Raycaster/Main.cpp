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

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create window
    window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
    screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, RENDER_WIDTH, RENDER_HEIGHT);

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

    if (currentKeyStates[SDL_SCANCODE_W])
    {
        playerSpeed = 1;
    }

    if (currentKeyStates[SDL_SCANCODE_A])
    {
        playerDir = -1;
    }

    if (currentKeyStates[SDL_SCANCODE_S])
    {
        playerSpeed = -1;
    }

    if (currentKeyStates[SDL_SCANCODE_D])
    {
        playerDir = 1;
    }

    if (currentKeyStates[SDL_SCANCODE_Q])
    {

    }

    if (currentKeyStates[SDL_SCANCODE_E])
    {

    }
}

double Rad(double deg)
{
    return deg * (M_PI / 180);
}

void Update()
{
    double moveStep = playerSpeed * (playerMoveSpeed * deltaTime);
    double newX = playerX + cos(playerRot) * moveStep;
    double newY = playerY + sin(playerRot) * moveStep;

    playerX = newX;
    playerY = newY;
    playerRot += playerDir * (playerRotSpeed * deltaTime);
    playerSpeed = 0;
    playerDir = 0;

    viewDist = (RENDER_WIDTH / 2) / tan(FOV / 2);

    if (playerRot < 0)
    {
        playerRot += TWO_PI;
    }
    else if (playerRot >= TWO_PI)
    {
        playerRot -= TWO_PI;
    }

    for (int x = 0; x < RENDER_WIDTH; x++)
    {
        // Where on the screen the ray goes through
        double rayScreenPos = (-RENDER_WIDTH / 2 + x);

        // The distance from the viewer to the point on the screen
        double rayViewDist = sqrt((rayScreenPos * rayScreenPos) + (viewDist * viewDist));

        // The angle of the ray, relative to the viewing direction.
        double rayAngle = asin(rayScreenPos / rayViewDist);

        CastRay(playerRot + rayAngle, x);
    }

    Minimap();
}

void CastRay(double rayAngle, int col)
{
    //std::cout << "Reay at " << rayAngle * (180 / M_PI) << " degrees." << std::endl;
    if (rayAngle < 0)
    {
        rayAngle += TWO_PI;
    }
    else if (rayAngle >= TWO_PI)
    {
        rayAngle -= TWO_PI;
    }

    // Check the quadrant of the ray
    bool right = (rayAngle > TWO_PI * 0.75 || rayAngle < TWO_PI * 0.25);
    bool up = (rayAngle < 0 || rayAngle > M_PI);

    double dist = 0.0; // Distance to tile we hit

    double xHit = 0.0;
    double yHit = 0.0;

    int tileX;
    int tileY;

    int hitTileX;
    int hitTileY;

    int side = 0;

    // First check against the vertical tile lines
    // We do this by moving to thr right or left edge of the block we're standing in,
    // and then moving in 1 map unit steps horizontally. The amount we have to move vertically
    // is determined by the slope of the way.

    double slope = sin(rayAngle) / cos(rayAngle);
    double dx = right ? 1 : -1; // We move either 1 map unit to the left or right
    double dy = dx * slope; // How much to move up or done

    double x = right ? ceil(playerX) : floor(playerX); // Starting horizontal position, at one of the edges of the current map tile
    double y = playerY + (x - playerX) * slope; // starting vertical position. We add the small horizontal step we just made, multiplied by the slope.

    while (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
    {
        int tileMapX = floor(x + (right ? 0 : -1));
        int tileMapY = floor(y);

        // Is this point inside a wall block?
        if (GetTile(Vector2D(tileMapX, tileMapY)) > 0)
        {
            double distX = x - playerX;
            double distY = y - playerY;
            dist = (distX * distX) + (distY * distY); // the distance from the player to this point, squared.

            side = 0;

            hitTileX = tileMapX;
            hitTileY = tileMapY;

            xHit = x;
            yHit = y;

            break;
        }

        x += dx;
        y += dy;
    }

    // Now check against horizontal lines. it's basically the same thing, but turned around.
    // The only difference here is that once we hit a map block, we check if there was also one
    // found there in the vertical run. If so, we only register this hit if this distance is smaller.

    slope = cos(rayAngle) / sin(rayAngle);
    dy = up ? -1 : 1;
    dx = dy * slope;
    y = up ? floor(playerY) : ceil(playerY);
    x = playerX + (y - playerY) * slope;

    while (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
    {
        int tileMapX = floor(x);
        int tileMapY = floor(y + (up ? -1 : 0));

        // Is this point inside a wall block?
        if (GetTile(Vector2D(tileMapX, tileMapY)) > 0)
        {
            double distX = x - playerX;
            double distY = y - playerY;
            double tileDist = (distX * distX) + (distY * distY);
            if (!dist || tileDist < dist)
            {
                dist = tileDist;

                side = 1;

                hitTileX = tileMapX;
                hitTileY = tileMapY;

                xHit = x;
                yHit = y;
            }

            break;
        }

        x += dx;
        y += dy;
    }

    if (dist)
    {
        dist = sqrt(dist);

        // Adjust for fish eye
        dist *= cos(playerRot - rayAngle);

        // Calculate the position and height of the wall strip.
        // The wall height is 1 unit, the distance from the player to the screen is viewDist,
        // thus the height on the screen is equal to
        // wallHeight * viewDist / dist
        double height = round(viewDist / dist);

        double drawStart = round((RENDER_HEIGHT / 2) - (height / 2));
        double drawEnd = drawStart + height;

        int tile = GetTile(Vector2D(hitTileX, hitTileY));
        Color color;
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
        DrawVerticalLine(col, drawStart, drawEnd, color);

        DrawRay(xHit, yHit);
    }
}

void DrawRay(int x, int y)
{
    Vector2D start(playerX * 8, playerY * 8);
    Vector2D end(x * 8, y * 8);
    DrawLine(start, end, RED);
}

void Render()
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    byte* pPixels;
    int pitch = 0;
    SDL_LockTexture(screenTexture, nullptr, (void**)&pPixels, &pitch);

    memcpy(pPixels, pixels, (RENDER_WIDTH * RENDER_HEIGHT) * 4);

    SDL_UnlockTexture(screenTexture);

    SDL_Rect renderRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_RenderCopyEx(renderer, screenTexture, nullptr, &renderRect, 0, NULL, SDL_FLIP_NONE);

    SDL_RenderPresent(renderer);
    memset(pixels, 0x00, RENDER_WIDTH * RENDER_HEIGHT * 4);
}

void Minimap()
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            //DrawLine(Vector2D(0, y) * 8, Vector2D(MAP_WIDTH, y) * 8, WHITE);
            //DrawLine(Vector2D(x, 0) * 8, Vector2D(x, MAP_HEIGHT) * 8, WHITE);

            int tile = GetTile(Vector2D(x, y));
            if (tile != 0)
            {
                Color color;
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

                DrawRect((x)* 8, (y)* 8, 8, 8, color);
            }
        }
    }
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
    if (x < 0 || y < 0 || x >= RENDER_WIDTH || y >= RENDER_HEIGHT)
    {
        return;
    }

    pixels[((y * RENDER_WIDTH) + x) * 4 + 3] = color.GetR();
    pixels[((y * RENDER_WIDTH) + x) * 4 + 2] = color.GetG();
    pixels[((y * RENDER_WIDTH) + x) * 4 + 1] = color.GetB();
    pixels[((y * RENDER_WIDTH) + x) * 4 + 0] = color.GetA();
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

void DrawRect(int x, int y, int RENDER_WIDTH, int RENDER_HEIGHT, Color color)
{
    for (int xIndex = 0; xIndex < RENDER_WIDTH; xIndex++)
    {
        for (int yIndex = 0; yIndex < RENDER_HEIGHT; yIndex++)
        {
            SetPixel(x + xIndex, y + yIndex, color);
        }
    }
}