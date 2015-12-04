/*
QuickCG 20071121

Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
QuickCG is an SDL codebase that wraps some of the SDL functionality.
It' used by Lode's Computer Graphics Tutorial to work with simple C++ calls
to demonstrate graphical programs.

QuickCG can handle some things that standard C++ doesn't but that are commonly useful, such as:
-drawing graphics
-a bitmap font
-simplified saving and loading of files
-reading keyboard and mouse input
-playing sound
-color models
-loading images
*/

#include "quickcg.h"

#include <SDL.h>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

namespace QuickCG
{

    ////////////////////////////////////////////////////////////////////////////////
    //VARIABLES/////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    int w; //width of the screen
    int h; //height of the screen

    std::map<int, bool> keypressed; //for the "keyPressed" function to detect a keypress only once
    SDL_Surface* scr; //the single SDL surface used
    Uint8* inkeys;
    SDL_Event event = { 0 };

    ////////////////////////////////////////////////////////////////////////////////
    //KEYBOARD FUNCTIONS////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    bool keyDown(int key) //this checks if the key is held down, returns true all the time until the key is up
    {
        return (inkeys[key] != 0);
    }

    bool keyPressed(int key) //this checks if the key is *just* pressed, returns true only once until the key is up again
    {
        if (keypressed.find(key) == keypressed.end()) keypressed[key] = false;
        if (inkeys[key])
        {
            if (keypressed[key] == false)
            {
                keypressed[key] = true;
                return true;
            }
        }
        else keypressed[key] = false;

        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //BASIC SCREEN FUNCTIONS////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    //The screen function: sets up the window for 32-bit color graphics.
    //Creates a graphical screen of width*height pixels in 32-bit color.
    //Set fullscreen to 0 for a window, or to 1 for fullscreen output
    //text is the caption or title of the window
    //also inits SDL
    void screen(int width, int height, bool fullscreen, const std::string& text)
    {
        int colorDepth = 32;
        w = width;
        h = height;

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            printf("Unable to init SDL: %s\n", SDL_GetError());
            SDL_Quit();
            std::exit(1);
        }
        std::atexit(SDL_Quit);
        if (fullscreen)
        {
            scr = SDL_SetVideoMode(width, height, colorDepth, SDL_SWSURFACE | SDL_FULLSCREEN);
            lock();
        }
        else
        {
            scr = SDL_SetVideoMode(width, height, colorDepth, SDL_HWSURFACE | SDL_HWPALETTE);
        }
        if (scr == NULL)
        {
            printf("Unable to set video: %s\n", SDL_GetError());
            SDL_Quit();
            std::exit(1);
        }
        SDL_WM_SetCaption(text.c_str(), NULL);

        SDL_EnableUNICODE(1); //for the text input things
    }

    //Locks the screen
    void lock()
    {
        if (SDL_MUSTLOCK(scr))
            if (SDL_LockSurface(scr) < 0)
                return;
    }

    //Unlocks the screen
    void unlock()
    {
        if (SDL_MUSTLOCK(scr))
            SDL_UnlockSurface(scr);
    }

    //Updates the screen.  Has to be called to view new pixels, but use only after
    //drawing the whole screen because it's slow.
    void redraw()
    {
        SDL_UpdateRect(scr, 0, 0, 0, 0);
    }

    //Clears the screen to black
    void cls(const ColorRGB& color)
    {
        SDL_FillRect(scr, NULL, 65536 * color.r + 256 * color.g + color.b);
    }

    //Puts an RGB color pixel at position x,y
    void pset(int x, int y, const ColorRGB& color)
    {
        if (x < 0 || y < 0 || x >= w || y >= h) return;
        Uint32 colorSDL = SDL_MapRGB(scr->format, color.r, color.g, color.b);
        Uint32* bufp;
        bufp = (Uint32*)scr->pixels + y * scr->pitch / 4 + x;
        *bufp = colorSDL;
    }

    //Gets RGB color of pixel at position x,y
    ColorRGB pget(int x, int y)
    {
        if (x < 0 || y < 0 || x >= w || y >= h) return RGB_Black;
        Uint32* bufp;
        bufp = (Uint32*)scr->pixels + y * scr->pitch / 4 + x;
        Uint32 colorSDL = *bufp;
        ColorRGB8bit colorRGB;
        SDL_GetRGB(colorSDL, scr->format, &colorRGB.r, &colorRGB.g, &colorRGB.b);
        return ColorRGB(colorRGB);
    }

    //Draws a buffer of pixels to the screen
    void drawBuffer(Uint32* buffer)
    {
        Uint32* bufp;
        bufp = (Uint32*)scr->pixels;

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                *bufp = buffer[h * x + y];
                bufp++;
            }
            bufp += scr->pitch / 4;
            bufp -= w;
        }
    }

    void getScreenBuffer(std::vector<Uint32>& buffer)
    {
        Uint32* bufp;
        bufp = (Uint32*)scr->pixels;

        buffer.resize(w * h);

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                buffer[h * x + y] = *bufp;
                bufp++;
            }
            bufp += scr->pitch / 4;
            bufp -= w;
        }
    }

    bool onScreen(int x, int y)
    {
        return (x >= 0 && y >= 0 && x < w && y < h);
    }



    ////////////////////////////////////////////////////////////////////////////////
    //NON GRAPHICAL FUNCTIONS///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    //Waits until you press a key. First the key has to be loose, this means, if you put two sleep functions in a row, the second will only work after you first released the key.
    void sleep()
    {
        int done = 0;
        while (done == 0)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT) end();
                if (event.type == SDL_KEYDOWN) done = 1;
            }
            SDL_Delay(5); //so it consumes less processing power
        }
    }

    void waitFrame(double oldTime, double frameDuration) //in seconds
    {
        float time = getTime();
        while (time - oldTime < frameDuration)
        {
            time = getTime();
            SDL_PollEvent(&event);
            if (event.type == SDL_QUIT) end();
            inkeys = SDL_GetKeyState(NULL);
            if (inkeys[SDLK_ESCAPE]) end();
            SDL_Delay(5); //so it consumes less processing power
        }
    }

    //Returns 1 if you close the window or press the escape key. Also handles everything that's needed per frame.
    //Never put key input code right before done() or SDL may see the key as SDL_QUIT
    bool done(bool quit_if_esc, bool delay) //delay makes CPU have some free time, use once per frame to avoid 100% usage of a CPU core
    {
        if (delay) SDL_Delay(5); //so it consumes less processing power
        int done = 0;
        if (!SDL_PollEvent(&event)) return 0;
        readKeys();
        if (quit_if_esc && inkeys[SDLK_ESCAPE]) done = 1;
        if (event.type == SDL_QUIT) done = 1;
        return done;
    }

    //Ends the program
    void end()
    {
        SDL_Quit();
        std::exit(1);
    }

    //Gives value of pressed key to inkeys.
    //the variable inkeys can then be used anywhere to check for input
    //Normally you have to use readkeys every time you want to use inkeys, but the done() function also uses inkeys so it's not needed to use readkeys if you use done().
    void readKeys()
    {
        inkeys = SDL_GetKeyState(NULL);
    }

    void getMouseState(int& mouseX, int& mouseY)
    {
        SDL_GetMouseState(&mouseX, &mouseY);
    }

    void getMouseState(int& mouseX, int& mouseY, bool& LMB, bool& RMB)
    {
        Uint8 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        if (mouseState & 1) LMB = true;
        else LMB = false;
        if (mouseState & 4) RMB = true;
        else RMB = false;
    }

    //Returns the time in milliseconds since the program started
    unsigned long getTicks()
    {
        return SDL_GetTicks();
    }


    ////////////////////////////////////////////////////////////////////////////////
    //2D SHAPES/////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////


    //Fast horizontal line from (x1,y) to (x2,y), with rgb color
    bool horLine(int y, int x1, int x2, const ColorRGB& color)
    {
        if (x2 < x1) { x1 += x2; x2 = x1 - x2; x1 -= x2; } //swap x1 and x2, x1 must be the leftmost endpoint   
        if (x2 < 0 || x1 >= w || y < 0 || y >= h) return 0; //no single point of the line is on screen
        if (x1 < 0) x1 = 0; //clip
        if (x2 >= w) x2 = w - 1; //clip

        Uint32 colorSDL = SDL_MapRGB(scr->format, color.r, color.g, color.b);
        Uint32* bufp;
        bufp = (Uint32*)scr->pixels + y * scr->pitch / 4 + x1;
        for (int x = x1; x <= x2; x++)
        {
            *bufp = colorSDL;
            bufp++;
        }
        return 1;
    }


    //Fast vertical line from (x,y1) to (x,y2), with rgb color
    bool verLine(int x, int y1, int y2, const ColorRGB& color)
    {
        if (y2 < y1) { y1 += y2; y2 = y1 - y2; y1 -= y2; } //swap y1 and y2
        if (y2 < 0 || y1 >= h || x < 0 || x >= w) return 0; //no single point of the line is on screen
        if (y1 < 0) y1 = 0; //clip
        if (y2 >= w) y2 = h - 1; //clip

        Uint32 colorSDL = SDL_MapRGB(scr->format, color.r, color.g, color.b);
        Uint32* bufp;

        bufp = (Uint32*)scr->pixels + y1 * scr->pitch / 4 + x;
        for (int y = y1; y <= y2; y++)
        {
            *bufp = colorSDL;
            bufp += scr->pitch / 4;
        }
        return 1;
    }


    //Bresenham line from (x1,y1) to (x2,y2) with rgb color
    bool drawLine(int x1, int y1, int x2, int y2, const ColorRGB& color)
    {
        if (x1 < 0 || x1 > w - 1 || x2 < 0 || x2 > w - 1 || y1 < 0 || y1 > h - 1 || y2 < 0 || y2 > h - 1) return 0;

        int deltax = std::abs(x2 - x1); //The difference between the x's
        int deltay = std::abs(y2 - y1); //The difference between the y's
        int x = x1; //Start x off at the first pixel
        int y = y1; //Start y off at the first pixel
        int xinc1, xinc2, yinc1, yinc2, den, num, numadd, numpixels, curpixel;

        if (x2 >= x1) //The x-values are increasing
        {
            xinc1 = 1;
            xinc2 = 1;
        }
        else //The x-values are decreasing
        {
            xinc1 = -1;
            xinc2 = -1;
        }
        if (y2 >= y1) //The y-values are increasing
        {
            yinc1 = 1;
            yinc2 = 1;
        }
        else //The y-values are decreasing
        {
            yinc1 = -1;
            yinc2 = -1;
        }
        if (deltax >= deltay) //There is at least one x-value for every y-value
        {
            xinc1 = 0; //Don't change the x when numerator >= denominator
            yinc2 = 0; //Don't change the y for every iteration
            den = deltax;
            num = deltax / 2;
            numadd = deltay;
            numpixels = deltax; //There are more x-values than y-values
        }
        else //There is at least one y-value for every x-value
        {
            xinc2 = 0; //Don't change the x for every iteration
            yinc1 = 0; //Don't change the y when numerator >= denominator
            den = deltay;
            num = deltay / 2;
            numadd = deltax;
            numpixels = deltay; //There are more y-values than x-values
        }
        for (curpixel = 0; curpixel <= numpixels; curpixel++)
        {
            pset(x % w, y % h, color);  //Draw the current pixel
            num += numadd;  //Increase the numerator by the top of the fraction
            if (num >= den) //Check if numerator >= denominator
            {
                num -= den; //Calculate the new numerator value
                x += xinc1; //Change the x as appropriate
                y += yinc1; //Change the y as appropriate
            }
            x += xinc2; //Change the x as appropriate
            y += yinc2; //Change the y as appropriate
        }

        return 1;
    }


    //Bresenham circle with center at (xc,yc) with radius and red green blue color
    bool drawCircle(int xc, int yc, int radius, const ColorRGB& color)
    {
        if (xc - radius < 0 || xc + radius >= w || yc - radius < 0 || yc + radius >= h) return 0;
        int x = 0;
        int y = radius;
        int p = 3 - (radius << 1);
        int a, b, c, d, e, f, g, h;
        while (x <= y)
        {
            a = xc + x; //8 pixels can be calculated at once thanks to the symmetry
            b = yc + y;
            c = xc - x;
            d = yc - y;
            e = xc + y;
            f = yc + x;
            g = xc - y;
            h = yc - x;
            pset(a, b, color);
            pset(c, d, color);
            pset(e, f, color);
            pset(g, f, color);
            if (x > 0) //avoid drawing pixels at same position as the other ones
            {
                pset(a, d, color);
                pset(c, b, color);
                pset(e, h, color);
                pset(g, h, color);
            }
            if (p < 0) p += (x++ << 2) + 6;
            else p += ((x++ - y--) << 2) + 10;
        }

        return 1;
    }


    //Filled bresenham circle with center at (xc,yc) with radius and red green blue color
    bool drawDisk(int xc, int yc, int radius, const ColorRGB& color)
    {
        if (xc + radius < 0 || xc - radius >= w || yc + radius < 0 || yc - radius >= h) return 0; //every single pixel outside screen, so don't waste time on it
        int x = 0;
        int y = radius;
        int p = 3 - (radius << 1);
        int a, b, c, d, e, f, g, h;
        int pb = yc + radius + 1, pd = yc + radius + 1; //previous values: to avoid drawing horizontal lines multiple times  (ensure initial value is outside the range)
        while (x <= y)
        {
            // write data
            a = xc + x;
            b = yc + y;
            c = xc - x;
            d = yc - y;
            e = xc + y;
            f = yc + x;
            g = xc - y;
            h = yc - x;
            if (b != pb) horLine(b, a, c, color);
            if (d != pd) horLine(d, a, c, color);
            if (f != b)  horLine(f, e, g, color);
            if (h != d && h != f) horLine(h, e, g, color);
            pb = b;
            pd = d;
            if (p < 0) p += (x++ << 2) + 6;
            else p += ((x++ - y--) << 2) + 10;
        }

        return 1;
    }

    //Rectangle with corners (x1,y1) and (x2,y2) and rgb color
    bool drawRect(int x1, int y1, int x2, int y2, const ColorRGB& color)
    {
        if (x1 < 0 || x1 > w - 1 || x2 < 0 || x2 > w - 1 || y1 < 0 || y1 > h - 1 || y2 < 0 || y2 > h - 1) return 0;
        SDL_Rect rec;
        rec.x = x1;
        rec.y = y1;
        rec.w = x2 - x1 + 1;
        rec.h = y2 - y1 + 1;
        Uint32 colorSDL = SDL_MapRGB(scr->format, color.r, color.g, color.b);
        SDL_FillRect(scr, &rec, colorSDL);  //SDL's ability to draw a hardware rectangle is used for now
        return 1;
    }

    //Functions for clipping a 2D line to the screen, which is the rectangle (0,0)-(w,h)
    //This is the Cohen-Sutherland Clipping Algorithm
    //Each of 9 regions gets an outcode, based on if it's at the top, bottom, left or right of the screen
    // 1001 1000 1010  9 8 10
    // 0001 0000 0010  1 0 2
    // 0101 0100 0110  5 4 6
    //int findregion returns which of the 9 regions a point is in, void clipline does the actual clipping
    int findRegion(int x, int y)
    {
        int code = 0;
        if (y >= h)
            code |= 1; //top
        else if (y < 0)
            code |= 2; //bottom
        if (x >= w)
            code |= 4; //right
        else if (x < 0)
            code |= 8; //left
        return(code);
    }
    bool clipLine(int x1, int y1, int x2, int y2, int & x3, int & y3, int & x4, int & y4)
    {
        int code1, code2, codeout;
        bool accept = 0, done = 0;
        code1 = findRegion(x1, y1); //the region outcodes for the endpoints
        code2 = findRegion(x2, y2);
        do  //In theory, this can never end up in an infinite loop, it'll always come in one of the trivial cases eventually
        {
            if (!(code1 | code2)) accept = done = 1;  //accept because both endpoints are in screen or on the border, trivial accept
            else if (code1 & code2) done = 1; //the line isn't visible on screen, trivial reject
            else  //if no trivial reject or accept, continue the loop
            {
                int x, y;
                codeout = code1 ? code1 : code2;
                if (codeout & 1) //top
                {
                    x = x1 + (x2 - x1) * (h - y1) / (y2 - y1);
                    y = h - 1;
                }
                else if (codeout & 2) //bottom
                {
                    x = x1 + (x2 - x1) * -y1 / (y2 - y1);
                    y = 0;
                }
                else if (codeout & 4) //right
                {
                    y = y1 + (y2 - y1) * (w - x1) / (x2 - x1);
                    x = w - 1;
                }
                else //left
                {
                    y = y1 + (y2 - y1) * -x1 / (x2 - x1);
                    x = 0;
                }
                if (codeout == code1) //first endpoint was clipped
                {
                    x1 = x; y1 = y;
                    code1 = findRegion(x1, y1);
                }
                else //second endpoint was clipped
                {
                    x2 = x; y2 = y;
                    code2 = findRegion(x2, y2);
                }
            }
        } while (done == 0);

        if (accept)
        {
            x3 = x1;
            x4 = x2;
            y3 = y1;
            y4 = y2;
            return 1;
        }
        else
        {
            x3 = x4 = y3 = y4 = 0;
            return 0;
        }
    }


    ////////////////////////////////////////////////////////////////////////////////
    //COLOR STRUCTS/////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    ColorRGB::ColorRGB(Uint8 r, Uint8 g, Uint8 b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    }
    ColorRGB::ColorRGB(const ColorRGB8bit& color)
    {
        this->r = color.r;
        this->g = color.g;
        this->b = color.b;
    }
    ColorRGB::ColorRGB()
    {
        this->r = 0;
        this->g = 0;
        this->b = 0;
    }
    ColorRGB8bit::ColorRGB8bit(Uint8 r, Uint8 g, Uint8 b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    }
    ColorRGB8bit::ColorRGB8bit(const ColorRGB& color)
    {
        this->r = color.r;
        this->g = color.g;
        this->b = color.b;
    }
    ColorRGB8bit::ColorRGB8bit()
    {
        this->r = 0;
        this->g = 0;
        this->b = 0;
    }

    //Add two colors
    ColorRGB operator+(const ColorRGB& color, const ColorRGB& color2)
    {
        ColorRGB c;
        c.r = color.r + color2.r;
        c.g = color.g + color2.g;
        c.b = color.b + color2.b;
        return c;
    }

    //Subtract two colors
    ColorRGB operator-(const ColorRGB& color, const ColorRGB& color2)
    {
        ColorRGB c;
        c.r = color.r - color2.r;
        c.g = color.g - color2.g;
        c.b = color.b - color2.b;
        return c;
    }

    //Multiplies a color with an integer
    ColorRGB operator*(const ColorRGB& color, int a)
    {
        ColorRGB c;
        c.r = color.r * a;
        c.g = color.g * a;
        c.b = color.b * a;
        return c;
    }

    //Multiplies a color with an integer
    ColorRGB operator*(int a, const ColorRGB& color)
    {
        ColorRGB c;
        c.r = color.r * a;
        c.g = color.g * a;
        c.b = color.b * a;
        return c;
    }

    //Divides a color through an integer
    ColorRGB operator/(const ColorRGB& color, int a)
    {
        if (a == 0) return color;
        ColorRGB c;
        c.r = color.r / a;
        c.g = color.g / a;
        c.b = color.b / a;
        return c;
    }

    //Are both colors equal?
    bool operator==(const ColorRGB& color, const ColorRGB& color2)
    {
        return(color.r == color2.r && color.g == color2.g && color.b == color2.b);
    }

    //Are both colors not equal?
    bool operator!=(const ColorRGB& color, const ColorRGB& color2)
    {
        return(!(color.r == color2.r && color.g == color2.g && color.b == color2.b));
    }

    ColorHSL::ColorHSL(Uint8 h, Uint8 s, Uint8 l)
    {
        this->h = h;
        this->s = s;
        this->l = l;
    }
    ColorHSL::ColorHSL()
    {
        this->h = 0;
        this->s = 0;
        this->l = 0;
    }
    ColorHSV::ColorHSV(Uint8 h, Uint8 s, Uint8 v)
    {
        this->h = h;
        this->s = s;
        this->v = v;
    }
    ColorHSV::ColorHSV()
    {
        this->h = 0;
        this->s = 0;
        this->v = 0;
    }


    ////////////////////////////////////////////////////////////////////////////////
    //COLOR CONVERSIONS/////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    /*
    Convert colors from one type to another
    r=red  g=green  b=blue  h=hue  s=saturation  l=lightness  v=value
    Color components from the color structs are Uint8's between 0 and 255
    color components used in the calculations are normalized between 0.0-1.0
    */

    //Converts an RGB color to HSL color
    ColorHSL RGBtoHSL(const ColorRGB& colorRGB)
    {
        float r, g, b, h = 0, s = 0, l; //this function works with floats between 0 and 1
        r = colorRGB.r / 256.0;
        g = colorRGB.g / 256.0;
        b = colorRGB.b / 256.0;

        float maxColor = std::max(r, std::max(g, b));
        float minColor = std::min(r, std::min(g, b));

        if (minColor == maxColor) //R = G = B, so it's a shade of grey
        {
            h = 0; //it doesn't matter what value it has
            s = 0;
            l = r; //doesn't matter if you pick r, g, or b
        }
        else
        {
            l = (minColor + maxColor) / 2;

            if (l < 0.5) s = (maxColor - minColor) / (maxColor + minColor);
            if (l >= 0.5) s = (maxColor - minColor) / (2.0 - maxColor - minColor);

            if (r == maxColor) h = (g - b) / (maxColor - minColor);
            if (g == maxColor) h = 2.0 + (b - r) / (maxColor - minColor);
            if (b == maxColor) h = 4.0 + (r - g) / (maxColor - minColor);

            h /= 6; //to bring it to a number between 0 and 1
            if (h < 0) h += 1;
        }

        ColorHSL colorHSL;
        colorHSL.h = int(h * 255.0);
        colorHSL.s = int(s * 255.0);
        colorHSL.l = int(l * 255.0);
        return colorHSL;
    }

    //Converts an HSL color to RGB color
    ColorRGB HSLtoRGB(const ColorHSL& colorHSL)
    {
        float r, g, b, h, s, l; //this function works with floats between 0 and 1
        float temp1, temp2, tempr, tempg, tempb;
        h = colorHSL.h / 256.0;
        s = colorHSL.s / 256.0;
        l = colorHSL.l / 256.0;

        //If saturation is 0, the color is a shade of grey
        if (s == 0) r = g = b = l;
        //If saturation > 0, more complex calculations are needed
        else
        {
            //set the temporary values
            if (l < 0.5) temp2 = l * (1 + s);
            else temp2 = (l + s) - (l * s);
            temp1 = 2 * l - temp2;
            tempr = h + 1.0 / 3.0;
            if (tempr > 1.0) tempr--;
            tempg = h;
            tempb = h - 1.0 / 3.0;
            if (tempb < 0.0) tempb++;

            //red
            if (tempr < 1.0 / 6.0) r = temp1 + (temp2 - temp1) * 6.0 * tempr;
            else if (tempr < 0.5) r = temp2;
            else if (tempr < 2.0 / 3.0) r = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempr) * 6.0;
            else r = temp1;

            //green
            if (tempg < 1.0 / 6.0) g = temp1 + (temp2 - temp1) * 6.0 * tempg;
            else if (tempg < 0.5) g = temp2;
            else if (tempg < 2.0 / 3.0) g = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempg) * 6.0;
            else g = temp1;

            //blue
            if (tempb < 1.0 / 6.0) b = temp1 + (temp2 - temp1) * 6.0 * tempb;
            else if (tempb < 0.5) b = temp2;
            else if (tempb < 2.0 / 3.0) b = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempb) * 6.0;
            else b = temp1;
        }

        ColorRGB colorRGB;
        colorRGB.r = int(r * 255.0);
        colorRGB.g = int(g * 255.0);
        colorRGB.b = int(b * 255.0);
        return colorRGB;
    }

    //Converts an RGB color to HSV color
    ColorHSV RGBtoHSV(const ColorRGB& colorRGB)
    {
        float r, g, b, h = 0.0, s = 0.0, v; //this function works with floats between 0 and 1
        r = colorRGB.r / 256.0;
        g = colorRGB.g / 256.0;
        b = colorRGB.b / 256.0;

        float maxColor = std::max(r, std::max(g, b));
        float minColor = std::min(r, std::min(g, b));

        v = maxColor;

        if (maxColor != 0.0) //avoid division by zero when the color is black
        {
            s = (maxColor - minColor) / maxColor;
        }

        if (s == 0.0)
        {
            h = 0.0; //it doesn't matter what value it has
        }
        else
        {
            if (r == maxColor) h = (g - b) / (maxColor - minColor);
            if (g == maxColor) h = 2.0 + (b - r) / (maxColor - minColor);
            if (b == maxColor) h = 4.0 + (r - g) / (maxColor - minColor);

            h /= 6.0; //to bring it to a number between 0 and 1
            if (h < 0.0) h++;
        }

        ColorHSV colorHSV;
        colorHSV.h = int(h * 255.0);
        colorHSV.s = int(s * 255.0);
        colorHSV.v = int(v * 255.0);
        return colorHSV;
    }

    //Converts an HSV color to RGB color
    ColorRGB HSVtoRGB(const ColorHSV& colorHSV)
    {
        float r, g, b, h, s, v; //this function works with floats between 0 and 1
        h = colorHSV.h / 256.0;
        s = colorHSV.s / 256.0;
        v = colorHSV.v / 256.0;

        //if saturation is 0, the color is a shade of grey
        if (s == 0.0) r = g = b = v;
        //if saturation > 0, more complex calculations are needed
        else
        {
            float f, p, q, t;
            int i;
            h *= 6.0; //to bring hue to a number between 0 and 6, better for the calculations
            i = int(floor(h)); //e.g. 2.7 becomes 2 and 3.01 becomes 3 or 4.9999 becomes 4
            f = h - i;//the fractional part of h

            p = v * (1.0 - s);
            q = v * (1.0 - (s * f));
            t = v * (1.0 - (s * (1.0 - f)));

            switch (i)
            {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
            default: r = g = b = 0; break;
            }
        }
        ColorRGB colorRGB;
        colorRGB.r = int(r * 255.0);
        colorRGB.g = int(g * 255.0);
        colorRGB.b = int(b * 255.0);
        return colorRGB;
    }

    Uint32 RGBtoINT(const ColorRGB& colorRGB)
    {
        return 65536 * colorRGB.r + 256 * colorRGB.g + colorRGB.b;
    }

    ColorRGB INTtoRGB(Uint32 colorINT)
    {
        ColorRGB colorRGB;
        colorRGB.r = (colorINT / 65536) % 256;
        colorRGB.g = (colorINT / 256) % 256;
        colorRGB.b = colorINT % 256;
        return colorRGB;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //FILE FUNCTIONS////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    void loadFile(std::vector<unsigned char>& buffer, const std::string& filename) //designed for loading files from hard disk in an std::vector
    {
        std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

        //get filesize
        std::streamsize size = 0;
        if (file.seekg(0, std::ios::end).good()) size = file.tellg();
        if (file.seekg(0, std::ios::beg).good()) size -= file.tellg();

        //read contents of the file into the vector
        buffer.resize(size_t(size));
        if (size > 0) file.read((char*)(&buffer[0]), size);
    }

    //write given buffer to the file, overwriting the file, it doesn't append to it.
    void saveFile(const std::vector<unsigned char>& buffer, const std::string& filename)
    {
        std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
        file.write(buffer.size() ? (char*)&buffer[0] : 0, std::streamsize(buffer.size()));
    }

    ////////////////////////////////////////////////////////////////////////////////
    //IMAGE FUNCTIONS///////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    int loadImage(std::vector<ColorRGB>& out, unsigned long& w, unsigned long& h, const std::string& filename)
    {
        std::vector<unsigned char> file, image;
        loadFile(file, filename);
        if (decodePNG(image, w, h, file)) return 1;

        out.resize(image.size() / 4);

        for (size_t i = 0; i < out.size(); i++)
        {
            out[i].r = image[i * 4 + 0];
            out[i].g = image[i * 4 + 1];
            out[i].b = image[i * 4 + 2];
            //out[i].a = image[i * 4 + 3];
        }

        return 0;
    }

    int loadImage(std::vector<Uint32>& out, unsigned long& w, unsigned long& h, const std::string& filename)
    {
        std::vector<unsigned char> file, image;
        loadFile(file, filename);
        if (decodePNG(image, w, h, file)) return 1;

        out.resize(image.size() / 4);

        for (size_t i = 0; i < out.size(); i++)
        {
            out[i] = 0x1000000 * image[i * 4 + 3] + 0x10000 * image[i * 4 + 0] + 0x100 * image[i * 4 + 1] + image[i * 4 + 2];
        }

        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //TEXT FUNCTIONS////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    //Draws character n at position x,y with color RGB and, if enabled, background color
    //This function is used by the text printing functions below, and uses the font data
    //defined below to draw the letter pixel by pixel
    void drawLetter(unsigned char n, int x, int y, const ColorRGB& color, bool bg, const ColorRGB& color2)
    {
        int u, v;

        for (v = 0; v < 8; v++)
            for (u = 0; u < 8; u++)
            {
                if (font[n][u][v]) pset(x + u, y + v, color);
                else if (bg) pset(x + u, y + v, color2);
            }
    }

    //Draws a string of text
    int printString(const std::string& text, int x, int y, const ColorRGB& color, bool bg, const ColorRGB& color2, int forceLength)
    {
        int amount = 0;
        for (size_t i = 0; i < text.size(); i++)
        {
            amount++;
            drawLetter(text[i], x, y, color, bg, color2);
            x += 8;
            if (x > w - 8) { x %= 8; y += 8; }
            if (y > h - 8) { y %= 8; }
        }
        while (amount < forceLength)
        {
            amount++;
            drawLetter(' ', x, y, color, bg, color2);
            x += 8;
            if (x > w - 8) { x %= 8; y += 8; }
            if (y > h - 8) { y %= 8; }
        }
        return h * x + y;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //TEXT INPUT FUNCTIONS//////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    const int ASCII_ENTER = 13;
    const int ASCII_BACKSPACE = 8;
    const int ASCII_SPACE = 32; //smallest printable ascii char

    Uint8 getInputCharacter()
    {
        int ascii = 0;
        static int previouschar = 0;

        if ((event.key.keysym.unicode & 0xFF80) == 0)
        {
            if (event.type == SDL_KEYDOWN)
            {
                ascii = event.key.keysym.unicode & 0x7F;
            }
        }

        if (ascii < ASCII_SPACE && ascii != ASCII_ENTER && ascii != ASCII_BACKSPACE) ascii = 0; //<32 ones, except enter and backspace

        if (ascii != previouschar) previouschar = ascii;
        else ascii = 0;

        return ascii;
    }
    //returns a string, length is the maximum length of the given string array
    void getInputString(std::string& text, const std::string& message, bool clear, int x, int y, const ColorRGB& color, bool bg, const ColorRGB& color2)
    {
        std::vector<Uint32> screenBuffer;
        getScreenBuffer(screenBuffer);

        bool enter = 0;
        bool change = 1;
        text.clear();

        while (enter == 0)
        {
            if (done()) end();
            Uint8 temp = getInputCharacter();
            if (temp >= ASCII_SPACE)
            {
                text.push_back(temp);
                change = 1;
            }
            if (temp == ASCII_BACKSPACE && text.size() > 0) { text.resize(text.size() - 1); change = 1; }

            if (change)
            {
                drawBuffer(&screenBuffer[0]);
                int pos = print(message, x, y, color, bg, color2);
                int x2 = pos / h, y2 = pos % h;
                print(text, x2, y2, color, bg, color2);
                redraw();
            }
            if (temp == ASCII_ENTER) { enter = 1; }
        }

        //remove the input stuff from the screen again so there is room for possible next input
        if (clear)
        {
            drawBuffer(&screenBuffer[0]);
            redraw();
        }
    }

    void encodeBase64(const std::vector<unsigned char>& in, std::string& out)
    {
        const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        unsigned long bit24 = 0;
        unsigned long sextet[4] = { 0, 0, 0, 0 };
        unsigned long octet[3] = { 0, 0, 0 };

        out.clear();
        out.reserve((4 * in.size()) / 3);

        for (size_t pos = 0; pos < in.size(); pos += 3)
        {
            octet[0] = in[pos + 0];
            octet[1] = (pos + 1 < in.size()) ? in[pos + 1] : 0;
            octet[2] = (pos + 2 < in.size()) ? in[pos + 2] : 0;

            bit24 = 256 * 256 * octet[0];
            bit24 += 256 * octet[1];
            bit24 += octet[2];

            sextet[0] = (bit24 / (64 * 64 * 64)) % 64;
            sextet[1] = (bit24 / (64 * 64)) % 64;
            sextet[2] = (bit24 / (64)) % 64;
            sextet[3] = (bit24 / (1)) % 64;

            for (size_t i = 0; i < 4; i++)
            {
                if (pos + i - 1 < in.size()) out.push_back(characters[sextet[i]]);
                else out.push_back('=');
            }

            if (pos % 57 == 0 && pos != 0) out.push_back(10); //newline char every 76 chars (57 = 3/4th of 76)
        }
    }

    void decodeBase64(std::vector<unsigned char>& out, const std::string& in)
    {
        unsigned long bit24 = 0;
        unsigned long sextet[4] = { 0, 0, 0, 0 };
        unsigned long octet[3] = { 0, 0, 0 };

        out.clear();
        out.reserve((3 * in.size()) / 4);

        for (size_t pos = 0; pos < in.size() - 3; pos += 4)
        {
            for (size_t i = 0; i < 4; i++)
            {
                unsigned long c = in[pos + i];
                if (c >= 65 && c <= 90) sextet[i] = c - 65;
                else if (c >= 97 && c <= 122) sextet[i] = c - 71;
                else if (c >= 48 && c <= 57) sextet[i] = c + 4;
                else if (c == '+') sextet[i] = 62;
                else if (c == '/') sextet[i] = 63;
                else if (c == '=') sextet[i] = 0; //value doesn't matter
                else //unknown char, can be whitespace, newline, ...
                {
                    pos++;
                    if (pos > in.size() - 3) return;
                    i--;
                }
            }

            bit24 = 64 * 64 * 64 * sextet[0];
            bit24 += 64 * 64 * sextet[1];
            bit24 += 64 * sextet[2];
            bit24 += sextet[3];

            octet[0] = (bit24 / (256 * 256)) % 256;
            octet[1] = (bit24 / (256)) % 256;
            octet[2] = (bit24 / (1)) % 256;

            for (size_t i = 0; i < 3; i++)
            {
                if (in[pos + 1 + i] != '=') out.push_back(octet[i]);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // PNG                                                                        //
    ////////////////////////////////////////////////////////////////////////////////

    int decodePNG(std::vector<unsigned char>& out_image_32bit, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, unsigned long in_size)
    {
        // picoPNG version 20071020
        // Copyright (c) 2005-2007 Lode Vandevenne
        //
        // This software is provided 'as-is', without any express or implied
        // warranty. In no event will the authors be held liable for any damages
        // arising from the use of this software.
        //
        // Permission is granted to anyone to use this software for any purpose,
        // including commercial applications, and to alter it and redistribute it
        // freely, subject to the following restrictions:
        //
        //     1. The origin of this software must not be misrepresented; you must not
        //     claim that you wrote the original software. If you use this software
        //     in a product, an acknowledgment in the product documentation would be
        //     appreciated but is not required.
        //     2. Altered source versions must be plainly marked as such, and must not be
        //     misrepresented as being the original software.
        //     3. This notice may not be removed or altered from any source distribution.

        // picoPNG is a PNG decoder in one C++ function. Use picoPNG for programs that need
        // only 1 .cpp file. Apologies for the compact code style, it's to make it tiny.

        static const unsigned long lengthbase[29] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258 };
        static const unsigned long lengthextra[29] = { 0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0 };
        static const unsigned long distancebase[30] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577 };
        static const unsigned long distanceextra[30] = { 0,0,0,0,1,1,2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  11,  11,  12,   12,   13,   13 };
        static const unsigned long clcl[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 }; //code length code lengths
        struct Zlib //nested functions for zlib decompression
        {
            static unsigned long readBitFromStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (bitp & 0x7)) & 1; bitp++; return result; }
            static unsigned long readBitsFromStream(size_t& bitp, const unsigned char* bits, size_t nbits)
            {
                unsigned long result = 0;
                for (size_t i = 0; i < nbits; i++) result += (readBitFromStream(bitp, bits)) << i;
                return result;
            }
            struct HuffmanTree
            {
                int makeFromLengths(const std::vector<unsigned long>& bitlen, unsigned long maxbitlen)
                { //make tree given the lengths
                    unsigned long numcodes = (unsigned long)(bitlen.size()), treepos = 0, nodefilled = 0;
                    std::vector<unsigned long> tree1d(numcodes), blcount(maxbitlen + 1, 0), nextcode(maxbitlen + 1, 0);
                    for (unsigned long bits = 0; bits < numcodes; bits++) blcount[bitlen[bits]]++; //count number of instances of each code length
                    for (unsigned long bits = 1; bits <= maxbitlen; bits++) nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
                    for (unsigned long n = 0; n < numcodes; n++) if (bitlen[n] != 0) tree1d[n] = nextcode[bitlen[n]]++; //generate all the codes
                    tree2d.clear(); tree2d.resize(numcodes * 2, 32767); //32767 here means the tree2d isn't filled there yet
                    for (unsigned long n = 0; n < numcodes; n++) //the codes
                        for (unsigned long i = 0; i < bitlen[n]; i++) //the bits for this code
                        {
                            unsigned long bit = (tree1d[n] >> (bitlen[n] - i - 1)) & 1;
                            if (treepos > numcodes - 2) return 55;
                            if (tree2d[2 * treepos + bit] == 32767) //not yet filled in
                            {
                                if (i + 1 == bitlen[n]) { tree2d[2 * treepos + bit] = n; treepos = 0; } //last bit
                                else { tree2d[2 * treepos + bit] = ++nodefilled + numcodes; treepos = nodefilled; } //addresses are encoded as values > numcodes
                            }
                            else treepos = tree2d[2 * treepos + bit] - numcodes; //subtract numcodes from address to get address value
                        }
                    return 0;
                }
                int decode(bool& decoded, unsigned long& result, size_t& treepos, unsigned long bit) const
                { //Decodes a symbol from the tree
                    unsigned long numcodes = (unsigned long)tree2d.size() / 2;
                    if (treepos >= numcodes) return 11; //error: you appeared outside the codetree
                    result = tree2d[2 * treepos + bit];
                    decoded = (result < numcodes);
                    treepos = decoded ? 0 : result - numcodes;
                    return 0;
                }
                std::vector<unsigned long> tree2d; //2D representation of a huffman tree: The one dimension is "0" or "1", the other contains all nodes and leaves of the tree.
            };
            struct Inflator
            {
                int error;
                void inflate(std::vector<unsigned char>& out, const std::vector<unsigned char>& in, size_t inpos = 0)
                {
                    size_t bp = 0, pos = 0; //bit pointer and byte pointer
                    error = 0;
                    unsigned long BFINAL = 0;
                    while (!BFINAL && !error)
                    {
                        if (bp >> 3 >= in.size()) { error = 52; return; } //error, bit pointer will jump past memory
                        BFINAL = readBitFromStream(bp, &in[inpos]);
                        unsigned long BTYPE = readBitFromStream(bp, &in[inpos]); BTYPE += 2 * readBitFromStream(bp, &in[inpos]);
                        if (BTYPE == 3) { error = 20; return; } //error: invalid BTYPE
                        else if (BTYPE == 0) inflateNoCompression(out, &in[inpos], bp, pos, in.size());
                        else inflateHuffmanBlock(out, &in[inpos], bp, pos, in.size(), BTYPE);
                    }
                    if (!error) out.resize(pos); //Only now we know the true size of out, resize it to that
                }
                void generateFixedTrees(HuffmanTree& tree, HuffmanTree& treeD) //get the tree of a deflated block with fixed tree
                {
                    std::vector<unsigned long> bitlen(288, 8), bitlenD(32, 5);;
                    for (size_t i = 144; i <= 255; i++) bitlen[i] = 9;
                    for (size_t i = 256; i <= 279; i++) bitlen[i] = 7;
                    tree.makeFromLengths(bitlen, 15);
                    treeD.makeFromLengths(bitlenD, 15);
                }
                HuffmanTree codetree, codetreeD, codelengthcodetree; //the code tree for Huffman codes, distance codes, and code length codes
                unsigned long huffmanDecodeSymbol(const unsigned char* in, size_t& bp, const HuffmanTree& codetree, size_t inlength)
                { //decode a single symbol from given list of bits with given code tree. return value is the symbol
                    bool decoded; unsigned long ct;
                    for (size_t treepos = 0;;)
                    {
                        if ((bp & 0x07) == 0 && (bp >> 3) > inlength) { error = 10; return 0; } //error: end reached without endcode
                        error = codetree.decode(decoded, ct, treepos, readBitFromStream(bp, in)); if (error) return 0; //stop, an error happened
                        if (decoded) return ct;
                    }
                }
                void getTreeInflateDynamic(HuffmanTree& tree, HuffmanTree& treeD, const unsigned char* in, size_t& bp, size_t inlength)
                { //get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
                    std::vector<unsigned long> bitlen(288, 0), bitlenD(32, 0);
                    if (bp >> 3 >= inlength - 2) { error = 49; return; } //the bit pointer is or will go past the memory
                    size_t HLIT = readBitsFromStream(bp, in, 5) + 257; //number of literal/length codes + 257
                    size_t HDIST = readBitsFromStream(bp, in, 5) + 1; //number of distance codes + 1
                    size_t HCLEN = readBitsFromStream(bp, in, 4) + 4; //number of code length codes + 4
                    std::vector<unsigned long> codelengthcode(19); //lengths of tree to decode the lengths of the dynamic tree
                    for (size_t i = 0; i < 19; i++) codelengthcode[clcl[i]] = (i < HCLEN) ? readBitsFromStream(bp, in, 3) : 0;
                    error = codelengthcodetree.makeFromLengths(codelengthcode, 7); if (error) return;
                    size_t i = 0, replength;
                    while (i < HLIT + HDIST)
                    {
                        unsigned long code = huffmanDecodeSymbol(in, bp, codelengthcodetree, inlength); if (error) return;
                        if (code <= 15) { if (i < HLIT) bitlen[i++] = code; else bitlenD[i++ - HLIT] = code; } //a length code
                        else if (code == 16) //repeat previous
                        {
                            if (bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
                            replength = 3 + readBitsFromStream(bp, in, 2);
                            unsigned long value; //set value to the previous code
                            if ((i - 1) < HLIT) value = bitlen[i - 1];
                            else value = bitlenD[i - HLIT - 1];
                            for (size_t n = 0; n < replength; n++) //repeat this value in the next lengths
                            {
                                if (i >= HLIT + HDIST) { error = 13; return; } //error: i is larger than the amount of codes
                                if (i < HLIT) bitlen[i++] = value; else bitlenD[i++ - HLIT] = value;
                            }
                        }
                        else if (code == 17) //repeat "0" 3-10 times
                        {
                            if (bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
                            replength = 3 + readBitsFromStream(bp, in, 3);
                            for (size_t n = 0; n < replength; n++) //repeat this value in the next lengths
                            {
                                if (i >= HLIT + HDIST) { error = 14; return; } //error: i is larger than the amount of codes
                                if (i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
                            }
                        }
                        else if (code == 18) //repeat "0" 11-138 times
                        {
                            if (bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
                            replength = 11 + readBitsFromStream(bp, in, 7);
                            for (size_t n = 0; n < replength; n++) //repeat this value in the next lengths
                            {
                                if (i >= HLIT + HDIST) { error = 15; return; } //error: i is larger than the amount of codes
                                if (i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
                            }
                        }
                        else { error = 16; return; } //error: somehow an unexisting code appeared. This can never happen.
                    }
                    if (bitlen[256] == 0) { error = 64; return; } //the length of the end code 256 must be larger than 0
                    error = tree.makeFromLengths(bitlen, 15); if (error) return; //now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
                    error = treeD.makeFromLengths(bitlenD, 15); if (error) return;
                }
                void inflateHuffmanBlock(std::vector<unsigned char>& out, const unsigned char* in, size_t& bp, size_t& pos, size_t inlength, unsigned long btype)
                {
                    if (btype == 1) { generateFixedTrees(codetree, codetreeD); }
                    else if (btype == 2) { getTreeInflateDynamic(codetree, codetreeD, in, bp, inlength); if (error) return; }
                    for (;;)
                    {
                        unsigned long code = huffmanDecodeSymbol(in, bp, codetree, inlength); if (error) return;
                        if (code == 256) return; //end code
                        else if (code <= 255) //literal symbol
                        {
                            if (pos >= out.size()) out.resize((pos + 1) * 2); //reserve more room
                            out[pos++] = (unsigned char)(code);
                        }
                        else if (code >= 257 && code <= 285) //length code
                        {
                            size_t length = lengthbase[code - 257], numextrabits = lengthextra[code - 257];
                            if ((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
                            length += readBitsFromStream(bp, in, numextrabits);
                            unsigned long codeD = huffmanDecodeSymbol(in, bp, codetreeD, inlength); if (error) return;
                            if (codeD > 29) { error = 18; return; } //error: invalid distance code (30-31 are never used)
                            unsigned long distance = distancebase[codeD], numextrabitsD = distanceextra[codeD];
                            if ((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
                            distance += readBitsFromStream(bp, in, numextrabitsD);
                            size_t start = pos, backward = start - distance;
                            if (pos + length >= out.size()) out.resize((pos + length) * 2); //reserve more room
                            for (size_t forward = 0; forward < length; forward++)
                            {
                                out[pos++] = out[backward++];
                                if (backward >= start) backward = start - distance;
                            }
                        }
                    }
                }
                void inflateNoCompression(std::vector<unsigned char>& out, const unsigned char* in, size_t& bp, size_t& pos, size_t inlength)
                {
                    while ((bp & 0x7) != 0) bp++; //go to first boundary of byte
                    size_t p = bp / 8;
                    if (p >= inlength - 4) { error = 52; return; } //error, bit pointer will jump past memory
                    unsigned long LEN = in[p] + 256 * in[p + 1], NLEN = in[p + 2] + 256 * in[p + 3]; p += 4;
                    if (LEN + NLEN != 65535) { error = 21; return; } //error: NLEN is not one's complement of LEN
                    if (pos + LEN >= out.size()) out.resize(pos + LEN);
                    if (p + LEN > inlength) { error = 23; return; } //error: reading outside of in buffer
                    for (unsigned long n = 0; n < LEN; n++) out[pos++] = in[p++]; //read LEN bytes of literal data
                    bp = p * 8;
                }
            };
            int decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in) //returns error value
            {
                Inflator inflator;
                if (in.size() < 2) { return 53; } //error, size of zlib data too small
                if ((in[0] * 256 + in[1]) % 31 != 0) { return 24; } //error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way
                unsigned long CM = in[0] & 15, CINFO = (in[0] >> 4) & 15, FDICT = (in[1] >> 5) & 1;
                if (CM != 8 || CINFO > 7) { return 25; } //error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec
                if (FDICT != 0) { return 26; } //error: the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary."
                inflator.inflate(out, in, 2);
                return inflator.error; //note: adler32 checksum was skipped and ignored
            }
        };
        struct PNG //nested functions for PNG decoding
        {
            struct Info
            {
                unsigned long width, height, colorType, bitDepth, compressionMethod, filterMethod, interlaceMethod, key_r, key_g, key_b;
                bool key_defined; //is a transparent color key given?
                std::vector<unsigned char> palette;
            } info;
            int error;
            void decode(std::vector<unsigned char>& out, const unsigned char* in, unsigned long size)
            {
                error = 0;
                if (size == 0 || in == 0) { error = 48; return; } //the given data is empty
                readPngHeader(&in[0], size); if (error) return;
                size_t pos = 33; //first byte of the first chunk after the header
                std::vector<unsigned char> idat; //the data from idat chunks
                bool IEND = false, known_type = true;
                info.key_defined = false;
                while (!IEND) //loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer
                {
                    if (pos + 8 >= size) { error = 30; return; } //error: size of the in buffer too small to contain next chunk
                    size_t chunkLength = read32bitInt(&in[pos]); pos += 4;
                    if (chunkLength > 2147483647) { error = 63; return; }
                    if (pos + chunkLength >= size) { error = 35; return; } //error: size of the in buffer too small to contain next chunk

                    if (in[pos + 0] == 'I' && in[pos + 1] == 'D' && in[pos + 2] == 'A' && in[pos + 3] == 'T') //IDAT chunk, containing compressed image data
                    {
                        idat.insert(idat.end(), &in[pos + 4], &in[pos + 4 + chunkLength]);
                        pos += (4 + chunkLength);
                    }
                    else if (in[pos + 0] == 'I' && in[pos + 1] == 'E' && in[pos + 2] == 'N' && in[pos + 3] == 'D') { pos += 4; IEND = true; }
                    else if (in[pos + 0] == 'P' && in[pos + 1] == 'L' && in[pos + 2] == 'T' && in[pos + 3] == 'E') //palette chunk (PLTE)
                    {
                        pos += 4; //go after the 4 letters
                        info.palette.resize(4 * (chunkLength / 3));
                        if (info.palette.size() > (4 * 256)) { error = 38; return; } //error: palette too big
                        for (size_t i = 0; i < info.palette.size(); i += 4)
                        {
                            for (size_t j = 0; j < 3; j++) info.palette[i + j] = in[pos++]; //RGB
                            info.palette[i + 3] = 255; //alpha
                        }
                    }
                    else if (in[pos + 0] == 't' && in[pos + 1] == 'R' && in[pos + 2] == 'N' && in[pos + 3] == 'S') //palette transparency chunk (tRNS)
                    {
                        pos += 4; //go after the 4 letters
                        if (info.colorType == 3)
                        {
                            if (4 * chunkLength > info.palette.size()) { error = 39; return; } //error: more alpha values given than there are palette entries
                            for (size_t i = 0; i < chunkLength; i++) info.palette[4 * i + 3] = in[pos++];
                        }
                        else if (info.colorType == 0)
                        {
                            if (chunkLength != 2) { error = 40; return; } //error: this chunk must be 2 bytes for greyscale image
                            info.key_defined = 1; info.key_r = info.key_g = info.key_b = 256 * in[pos] + in[pos + 1]; pos += 2;
                        }
                        else if (info.colorType == 2)
                        {
                            if (chunkLength != 6) { error = 41; return; } //error: this chunk must be 6 bytes for RGB image
                            info.key_defined = 1;
                            info.key_r = 256 * in[pos] + in[pos + 1]; pos += 2;
                            info.key_g = 256 * in[pos] + in[pos + 1]; pos += 2;
                            info.key_b = 256 * in[pos] + in[pos + 1]; pos += 2;
                        }
                        else { error = 42; return; } //error: tRNS chunk not allowed for other color models
                    }
                    else //it's not an implemented chunk type, so ignore it: skip over the data
                    {
                        if (!(in[pos + 0] & 32)) { error = 69; return; } //error: unknown critical chunk (5th bit of first byte of chunk type is 0)
                        pos += (chunkLength + 4); //skip 4 letters and uninterpreted data of unimplemented chunk
                        known_type = false;
                    }
                    pos += 4; //step over CRC (which is ignored)
                }
                unsigned long bpp = getBpp(info);
                std::vector<unsigned char> scanlines(((info.width * (info.height * bpp + 7)) / 8) + info.height); //now the out buffer will be filled
                Zlib zlib; //decompress with the Zlib decompressor
                error = zlib.decompress(scanlines, idat); if (error) return; //stop if the zlib decompressor returned an error
                size_t bytewidth = (bpp + 7) / 8, outlength = (info.height * info.width * bpp + 7) / 8;
                out.resize(outlength); //time to fill the out buffer
                unsigned char* out_ = outlength ? &out[0] : 0; //use a regular pointer to the std::vector for faster code if compiled without optimization
                if (info.interlaceMethod == 0) //no interlace, just filter
                {
                    size_t linestart = 0, linelength = (info.width * bpp + 7) / 8; //length in bytes of a scanline, excluding the filtertype byte
                    if (bpp >= 8) //byte per byte
                        for (unsigned long y = 0; y < info.height; y++)
                        {
                            unsigned long filterType = scanlines[linestart];
                            const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
                            unFilterScanline(&out_[linestart - y], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength); if (error) return;
                            linestart += (1 + linelength); //go to start of next scanline
                        }
                    else //less than 8 bits per pixel, so fill it up bit per bit
                    {
                        std::vector<unsigned char> templine((info.width * bpp + 7) >> 3); //only used if bpp < 8
                        for (size_t y = 0, obp = 0; y < info.height; y++)
                        {
                            unsigned long filterType = scanlines[linestart];
                            const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
                            unFilterScanline(&templine[0], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength); if (error) return;
                            for (size_t bp = 0; bp < info.width * bpp;) setBitOfReversedStream(obp, out_, readBitFromReversedStream(bp, &templine[0]));
                            linestart += (1 + linelength); //go to start of next scanline
                        }
                    }
                }
                else //interlaceMethod is 1 (Adam7)
                {
                    size_t passw[7] = { (info.width + 7) / 8, (info.width + 3) / 8, (info.width + 3) / 4, (info.width + 1) / 4, (info.width + 1) / 2, (info.width + 0) / 2, (info.width + 0) / 1 };
                    size_t passh[7] = { (info.height + 7) / 8, (info.height + 7) / 8, (info.height + 3) / 8, (info.height + 3) / 4, (info.height + 1) / 4, (info.height + 1) / 2, (info.height + 0) / 2 };
                    size_t passstart[7] = { 0 };
                    size_t pattern[28] = { 0, 4, 0, 2, 0, 1, 0, 0, 0, 4, 0, 2, 0, 1, 8, 8, 4, 4, 2, 2, 1, 8, 8, 8, 4, 4, 2, 2 }; //values for the adam7 passes
                    for (int i = 0; i < 6; i++) passstart[i + 1] = passstart[i] + passh[i] * ((passw[i] ? 1 : 0) + (passw[i] * bpp + 7) / 8);
                    std::vector<unsigned char> scanlineo((info.width * bpp + 7) / 8), scanlinen((info.width * bpp + 7) / 8); //"old" and "new" scanline
                    for (int i = 0; i < 7; i++)
                        adam7Pass(&out_[0], &scanlinen[0], &scanlineo[0], &scanlines[passstart[i]], info.width, pattern[i], pattern[i + 7], pattern[i + 14], pattern[i + 21], passw[i], passh[i], bpp);
                }
                if (info.colorType != 6 || info.bitDepth != 8) //conversion needed
                {
                    std::vector<unsigned char> data = out;
                    error = convert(out, &data[0], info, info.width, info.height);
                }
            }
            void readPngHeader(const unsigned char* in, size_t inlength) //read the information from the header and store it in the Info
            {
                if (inlength < 29) { error = 27; return; } //error: the data length is smaller than the length of the header
                if (in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10) { error = 28; return; } //no PNG signature
                if (in[12] != 'I' || in[13] != 'H' || in[14] != 'D' || in[15] != 'R') { error = 29; return; } //error: it doesn't start with a IHDR chunk!
                info.width = read32bitInt(&in[16]); info.height = read32bitInt(&in[20]);
                info.bitDepth = in[24]; info.colorType = in[25];
                info.compressionMethod = in[26]; if (in[26] != 0) { error = 32; return; } //error: only compression method 0 is allowed in the specification
                info.filterMethod = in[27]; if (in[27] != 0) { error = 33; return; } //error: only filter method 0 is allowed in the specification
                info.interlaceMethod = in[28]; if (in[28] > 1) { error = 34; return; } //error: only interlace methods 0 and 1 exist in the specification
                error = checkColorValidity(info.colorType, info.bitDepth);
            }
            void unFilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon, size_t bytewidth, unsigned long filterType, size_t length)
            {
                switch (filterType)
                {
                case 0: for (size_t i = 0; i < length; i++) recon[i] = scanline[i]; break;
                case 1:
                    for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
                    for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth];
                    break;
                case 2:
                    if (precon) for (size_t i = 0; i < length; i++) recon[i] = scanline[i] + precon[i];
                    else       for (size_t i = 0; i < length; i++) recon[i] = scanline[i];
                    break;
                case 3:
                    if (precon)
                    {
                        for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i] + precon[i] / 2;
                        for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
                    }
                    else
                    {
                        for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
                        for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
                    }
                    break;
                case 4:
                    if (precon)
                    {
                        for (size_t i = 0; i < bytewidth; i++) recon[i] = (unsigned char)(scanline[i] + paethPredictor(0, precon[i], 0));
                        for (size_t i = bytewidth; i < length; i++) recon[i] = (unsigned char)(scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
                    }
                    else
                    {
                        for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
                        for (size_t i = bytewidth; i < length; i++) recon[i] = (unsigned char)(scanline[i] + paethPredictor(recon[i - bytewidth], 0, 0));
                    }
                    break;
                default: error = 36; return; //error: unexisting filter type given
                }
            }
            void adam7Pass(unsigned char* out, unsigned char* linen, unsigned char* lineo, const unsigned char* in, unsigned long w, size_t passleft, size_t passtop, size_t spacex, size_t spacey, size_t passw, size_t passh, unsigned long bpp)
            { //filter and reposition the pixels into the output when the image is Adam7 interlaced. This function can only do it after the full image is already decoded. The out buffer must have the correct allocated memory size already.
                if (passw == 0) return;
                size_t bytewidth = (bpp + 7) / 8, linelength = 1 + ((bpp * passw + 7) / 8);
                for (unsigned long y = 0; y < passh; y++)
                {
                    unsigned char filterType = in[y * linelength], *prevline = (y == 0) ? 0 : lineo;
                    unFilterScanline(linen, &in[y * linelength + 1], prevline, bytewidth, filterType, (w * bpp + 7) / 8); if (error) return;
                    if (bpp >= 8) for (size_t i = 0; i < passw; i++) for (size_t b = 0; b < bytewidth; b++) //b = current byte of this pixel
                        out[bytewidth * w * (passtop + spacey * y) + bytewidth * (passleft + spacex * i) + b] = linen[bytewidth * i + b];
                    else for (size_t i = 0; i < passw; i++)
                    {
                        size_t obp = bpp * w * (passtop + spacey * y) + bpp * (passleft + spacex * i), bp = i * bpp;
                        for (size_t b = 0; b < bpp; b++) setBitOfReversedStream(obp, out, readBitFromReversedStream(bp, &linen[0]));
                    }
                    unsigned char* temp = linen; linen = lineo; lineo = temp; //swap the two buffer pointers "line old" and "line new"
                }
            }
            static unsigned long readBitFromReversedStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (7 - bitp & 0x7)) & 1; bitp++; return result; }
            static unsigned long readBitsFromReversedStream(size_t& bitp, const unsigned char* bits, unsigned long nbits)
            {
                unsigned long result = 0;
                for (size_t i = nbits - 1; i < nbits; i--) result += ((readBitFromReversedStream(bitp, bits)) << i);
                return result;
            }
            void setBitOfReversedStream(size_t& bitp, unsigned char* bits, unsigned long bit) { bits[bitp >> 3] |= (bit << (7 - bitp & 0x7)); bitp++; }
            unsigned long read32bitInt(const unsigned char* buffer) { return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]; }
            int checkColorValidity(unsigned long colorType, unsigned long bd) //return type is a LodePNG error code
            {
                if ((colorType == 2 || colorType == 4 || colorType == 6)) if (!(bd == 8 || bd == 16)) return 37;
                else if (colorType == 0) if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37;
                else if (colorType == 3) if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8)) return 37;
                else return 31; //unexisting color type
                return 0; //allowed color type / bits combination
            }
            unsigned long getBpp(const Info& info)
            {
                if (info.colorType == 2) return (3 * info.bitDepth);
                else if (info.colorType >= 4) return (info.colorType - 2) * info.bitDepth;
                else return info.bitDepth;
            }
            int convert(std::vector<unsigned char>& out, const unsigned char* in, Info& infoIn, unsigned long w, unsigned long h)
            { //converts from any color type to 32-bit. return value = LodePNG error code
                size_t numpixels = w * h, bp = 0;
                out.resize(numpixels * 4);
                unsigned char* out_ = out.empty() ? 0 : &out[0]; //faster if compiled without optimization
                if (infoIn.bitDepth == 8 && infoIn.colorType == 0) //greyscale
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[i];
                        out_[4 * i + 3] = (infoIn.key_defined && in[i] == infoIn.key_r) ? 0 : 255;
                    }
                else if (infoIn.bitDepth == 8 && infoIn.colorType == 2) //RGB color
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        for (size_t c = 0; c < 3; c++) out_[4 * i + c] = in[3 * i + c];
                        out_[4 * i + 3] = (infoIn.key_defined == 1 && in[3 * i + 0] == infoIn.key_r && in[3 * i + 1] == infoIn.key_g && in[3 * i + 2] == infoIn.key_b) ? 0 : 255;
                    }
                else if (infoIn.bitDepth == 8 && infoIn.colorType == 3) //indexed color (palette)
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        if (4U * in[i] >= infoIn.palette.size()) return 46;
                        for (size_t c = 0; c < 4; c++) out_[4 * i + c] = infoIn.palette[4 * in[i] + c]; //get rgb colors from the palette
                    }
                else if (infoIn.bitDepth == 8 && infoIn.colorType == 4) //greyscale with alpha
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[2 * i + 0];
                        out_[4 * i + 3] = in[2 * i + 1];
                    }
                else if (infoIn.bitDepth == 8 && infoIn.colorType == 6) for (size_t i = 0; i < numpixels; i++) for (size_t c = 0; c < 4; c++) out_[4 * i + c] = in[4 * i + c]; //RGB with alpha
                else if (infoIn.bitDepth == 16 && infoIn.colorType == 0) //greyscale
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[2 * i];
                        out_[4 * i + 3] = (infoIn.key_defined && 256U * in[i] + in[i + 1] == infoIn.key_r) ? 0 : 255;
                    }
                else if (infoIn.bitDepth == 16 && infoIn.colorType == 2) //RGB color
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        for (size_t c = 0; c < 3; c++) out_[4 * i + c] = in[6 * i + 2 * c];
                        out_[4 * i + 3] = (infoIn.key_defined && 256U * in[6 * i + 0] + in[6 * i + 1] == infoIn.key_r && 256U * in[6 * i + 2] + in[6 * i + 3] == infoIn.key_g && 256U * in[6 * i + 4] + in[6 * i + 5] == infoIn.key_b) ? 0 : 255;
                    }
                else if (infoIn.bitDepth == 16 && infoIn.colorType == 4) //greyscale with alpha
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[4 * i]; //most significant byte
                        out_[4 * i + 3] = in[4 * i + 2];
                    }
                else if (infoIn.bitDepth == 16 && infoIn.colorType == 6) for (size_t i = 0; i < numpixels; i++) for (size_t c = 0; c < 4; c++) out_[4 * i + c] = in[8 * i + 2 * c]; //RGB with alpha
                else if (infoIn.bitDepth < 8 && infoIn.colorType == 0) //greyscale
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        unsigned long value = (readBitsFromReversedStream(bp, in, infoIn.bitDepth) * 255) / ((1 << infoIn.bitDepth) - 1); //scale value from 0 to 255
                        out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = (unsigned char)(value);
                        out_[4 * i + 3] = (infoIn.key_defined && value && ((1U << infoIn.bitDepth) - 1U) == infoIn.key_r && ((1U << infoIn.bitDepth) - 1U)) ? 0 : 255;
                    }
                else if (infoIn.bitDepth < 8 && infoIn.colorType == 3) //palette
                    for (size_t i = 0; i < numpixels; i++)
                    {
                        unsigned long value = readBitsFromReversedStream(bp, in, infoIn.bitDepth);
                        if (4 * value >= infoIn.palette.size()) return 47;
                        for (size_t c = 0; c < 4; c++) out_[4 * i + c] = infoIn.palette[4 * value + c]; //get rgb colors from the palette
                    }
                return 0;
            }
            long paethPredictor(long a, long b, long c) //Paeth predicter, used by PNG filter type 4
            {
                long p = a + b - c, pa = p > a ? p - a : a - p, pb = p > b ? p - b : b - p, pc = p > c ? p - c : c - p;
                return (pa <= pb && pa <= pc) ? a : pb <= pc ? b : c;
            }
        };
        PNG decoder; decoder.decode(out_image_32bit, in_png, in_size);
        image_width = decoder.info.width; image_height = decoder.info.height;
        return decoder.error;
    }

    int decodePNG(std::vector<unsigned char>& out_image_32bit, unsigned long& image_width, unsigned long& image_height, const std::vector<unsigned char>& in_png)
    {
        return decodePNG(out_image_32bit, image_width, image_height, in_png.size() ? &in_png[0] : 0, in_png.size());
    }

    ////////////////////////////////////////////////////////////////////////////////
    //DATA//////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////


    bool font[256][8][8];
    struct GenerateFont
    {
        GenerateFont()
        {
            /*
            The full extended ASCII character set, in the form 256 bitmap symbols of 8x8
            pixels in one 128x128 PNG, base64-encoded.
            The background color is black, not transparent.
            */
            const std::string font8x8string = "\
iVBORw0KGgoAAAANSUhEUgAAAIAAAACAAQMAAAD58POIAAAABlBMVEUAAAD///+l2Z/dAAAEtklE\n\
QVRIiY1VP2scRxR/OKA0g6RyQIcFwR/ggcN5i0WCfIc0qQbZjF1Mcbi4PMhm5XQp8gEsMKRKkzpl\n\
DAOCIcVD7sTBKeaqcyPMQYK8xTCb9/ZOUowS8Nzs7OzvfvP+zXszAG0bdqtd6KHfqQ+PTwB+6EvV\n\
CFCffu4fPv4G4Jd5aarKvvMnO/7wuB4YpTT16Vevv/f4+B3A69OmKUUBrgfg5F1VnTe6hO3k6bBk\n\
V5oK5Xa1FKFtC1BVqpbsIp7Ai3vWH7dgrYV1W2zXfn4KdW0twgPbQ3fQ+jnJQmtc9HUPpVj/HeuS\n\
UmIvArsD6/dYCK0y+lpktLBHbW1ri+VBb0VL7fd+am+1AMisEzNuGoaAUDnrhq8tAAqhScEhehTA\n\
KFAiV9HZGtfrEQLZKTjbZ8jyJb0YvIANY5AUOs+gMiaAcViCaQqqBQFfwL/bwCckGuUqE6kKR8Ar\n\
5jo6Zh6WMFygMaGLhhnRZWsgY1Wx6Sw1KCqMhUt0phhGFqOFgSBTFr5jixNUQylnMkSOJoiiBe+Y\n\
AQ3mOsveMt1frlKqIJH34ejIMxrv3i8DvGUf/bNnUYDgylUSoImeKGY05M7fCpAHxpmoD0fpLEFk\n\
leFEhkxSCkCsWlYHTLQsquVOyyQ/5pS4UKRdAM+efxsAds5WA7AQ/1Lglw5tENO5qQQ4rwidtUk8\n\
uqCRACXgkTOaAY6CmEDvEya3pcBqtCTKmBIVukd3reglaAsYQQULtIvJkC8SKANBw7WOn8RfYuqm\n\
YUIeLzuRYjw3sn1TsQqL7l/jI2fHHi3huQLiC0fXeGUkBaa/05RWZkmzJSW+saQDmNxas7t+jexi\n\
up5u9g8tXlRwMZ01zRCgLIBY6DnETYDUh6QADQGSDBJGgqZx5kumEpona8CZVWenGJLJogUKrO7f\n\
WjI0glHLCwQZuFmw5BjDqVoigwhAhAhUE1Ejw6SaiCuSwlvGmCg+ixZxTnb/oGma2Ga1VARY1j+5\n\
cTEKkDOYlg7btt1SQTUJYxOR+iYWIzggtQYS24pX21pyLDYtuAIbeLYH+ZjX1IbbJzSTdDdFxIme\n\
xHGV/26BjnndBHDIwth8Z1myzG+tyDiTJW1rKrIZeXajvhsOgu0JjDR3c62Fn9LGtFEHwYM6bh+K\n\
6bqDHPQ0mc3AezBI3F0e1C7ng78sP5SaU50AMZrHT60wtG75om05mrM3tchoxQhNE/H35c+3QRja\n\
/meayV98/aeccmOA8Vi6ID/++u3HwMDoxtBdjcvVuLtl2K3x1tY1o+uuijCuSum6a8ZY+lgz/VrL\n\
LXCHISetDqpUf/8L7I8fHX7oH/Uf+hsG4iaj9/te+6ND+XtgwMCxgwAdPgHQbvWxUsSrnf4/AOn7\n\
+/L6iGHldQ30fX+4vy9mdmLOakeVjPVRJz4ZkFY0RapqBJLPsoelePDT4d4xjmEqFYqtv6CUbNtG\n\
Oei7GCx7yyU183lm+INjQAWklOfzDUPOGCtXlVx/KiPGUMhZrN4TuFoyJariKNcTbOZyJm5LjUu6\n\
7ugdI7cH4p7F53JwmmCUka3bs/BqAKp6zbA2Q2UFcPVahgTouQT6MjTKUIdn+EqLb12cOWcBPi5V\n\
O9lUrLR/ABF/3H2EtBmWAAAAAElFTkSuQmCC";

            std::vector<unsigned char> png, image;
            decodeBase64(png, font8x8string);
            unsigned long w, h;
            decodePNG(image, w, h, &png[0], png.size());
            for (size_t c = 0; c < 256; c++)
                for (size_t y = 0; y < 8; y++)
                    for (size_t x = 0; x < 8; x++)
                    {
                        font[c][x][y] = image[4 * 128 * (8 * (c / 16) + y) + 4 * (8 * (c % 16) + x)] != 0;
                    }
        }
    };
    GenerateFont generateFont;

    ////////////////////////////////////////////////////////////////////////////////
    //Multithreading helper functions///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    //SDL's C functions don't have destructors and such so therefore this here is needed

    //currently only needed for audio, therefor it's not in a different cpp file.

    //this creates SDL mutexes and makes sure that they're destroyed at the end. MutexFactory does the deletion!
    struct MutexFactory
    {
        SDL_mutex* createMutex()
        {
            mutexes.push_back(SDL_CreateMutex());
            return mutexes.back();
        }

        ~MutexFactory()
        {
            for (size_t i = 0; i < mutexes.size(); i++)
                SDL_DestroyMutex(mutexes[i]);
        }

    private:

        std::vector<SDL_mutex*> mutexes;
    };

    MutexFactory mutexFactory;

    //this does SDL_mutexP in the ctor and SDL_mutexV in the dtor so no matter where you leave a function, SDL_mutexV is called
    struct Mutex
    {
        SDL_mutex** m;

        Mutex(SDL_mutex*& mutex)
        {
            m = &mutex;
            SDL_mutexP(*m);
        }

        ~Mutex()
        {
            SDL_mutexV(*m);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
    //Soundcard functions///////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    size_t audio_min_samples = 4096; //safety buffer to avoid clicks
    size_t audio_max_samples = 8192; //avoid too long queue

    double audio_volume = 1.0;
    int audio_mode = 2; //0=off, 1=full (volume ignored), 2=volume-controlled

    void audioSetBufferSamplesRange(size_t min_samples, size_t max_samples)
    {
        audio_min_samples = min_samples;
        audio_max_samples = max_samples;
    }

    void audioSetMode(int mode) //0: silent, 1: full (no volume calculations ==> faster), 2: volume-controlled (= default value)
    {
        audio_mode = mode;
    }

    void audioSetVolume(double volume) //multiplier used if mode is 2 (volume-controlled). Default value is 1.0.
    {
        audio_volume = volume;
    }

    /*
    Avoid the callback function and pushSamples function to be called at the same time,
    or the std::vector can be invalid as two threads at the same time change it.
    This SDL_mutex helps eliminate that problem.
    */
    SDL_mutex* audio_lock = mutexFactory.createMutex();

    std::vector<double> audio_data(audio_min_samples, 0);

    SDL_AudioSpec audiospec_wanted, audiospec_obtained;

    size_t audioSamplesShortage() //returns value > 0 if the soundcard is consuming more samples than you're producing
    {
        if (audio_data.size() < audio_min_samples) return audio_min_samples - audio_data.size();
        else return 0;
    }

    size_t audioSamplesOverflow() //returns value > 0 if you're producing more samples than the soundard is consuming - so take it easy a bit
    {
        if (audio_data.size() > audio_max_samples) return audio_data.size() - audio_max_samples;
        else return 0;
    }

    void audioCallback(void* /*userdata*/, Uint8* stream, int len)
    {
        Mutex mutex(audio_lock);

        int dataLengthLeft = audio_data.size();

        //only play if we have data left
        if (dataLengthLeft <= 0) return;

        int nsamples = len / 2; //always 16-bit, so always 2 bytes per sample, hence the amount of samples being len / 2
        int fill_len = (nsamples < dataLengthLeft ? nsamples : dataLengthLeft);

        for (int i = 0; i < nsamples; i++)
        {
            if (i < fill_len)
            {
                int s = int(audio_data[i] * 32768);
                if (s < -32768) s = -32768;
                if (s > 32767) s = 32767;

                stream[i * 2 + 0] = Uint8(s % 256);
                stream[i * 2 + 1] = Uint8(s / 256);
            }
            else stream[i * 2 + 0] = stream[i * 2 + 1] = 0;
        }

        audio_data.erase(audio_data.begin(), audio_data.begin() + fill_len);
    }

    int audioOpen(int samplerate, int framesize) //always 16-bit mono sound for now
    {
        //set the audio format
        audiospec_wanted.freq = samplerate;
        audiospec_wanted.format = AUDIO_S16;
        audiospec_wanted.channels = 1;  //1 = mono, 2 = stereo
        audiospec_wanted.samples = framesize;
        audiospec_wanted.callback = audioCallback;
        audiospec_wanted.userdata = NULL;

        /*
        when using alsa and 44100 samples/second, then the framesize (samples)
        will be 940 instead of 1024. Resampled to 48000Hz, this gives back 1024.
        */

        //open the audio device, forcing the wanted format
        if (SDL_OpenAudio(&audiospec_wanted, &audiospec_obtained) < 0)
        {
            return 1;
        }

        SDL_PauseAudio(0);

        return 0;
    }

    void audioClose()
    {
        SDL_CloseAudio();
    }

    int audioReOpen() //closes and opens again with same parameters
    {
        SDL_PauseAudio(1);
        SDL_CloseAudio();
        if (SDL_OpenAudio(&audiospec_wanted, &audiospec_obtained) < 0)
        {
            return 1;
        }
        SDL_PauseAudio(0);

        return 0;
    }


    //only works correct for 16 bit audio currently
    void audioPushSamples(const std::vector<double>& samples, size_t pos, size_t end)
    {
        if (audio_mode == 0) return;

        Mutex mutex(audio_lock);

        if (audio_mode == 1)
        {
            audio_data.insert(audio_data.end(), samples.begin() + pos, samples.begin() + end);
        }
        else if (audio_mode == 2)
        {
            size_t j = audio_data.size();
            audio_data.resize(j + samples.size());
            for (size_t i = 0; i < samples.size(); i++)
            {
                audio_data[j + i] = samples[i] * audio_volume;
            }
        }
    }

    void audioPlay(const std::vector<double>& samples)
    {
        if (audio_mode == 0) return;

        Mutex mutex(audio_lock);

        //the *current* time is at the first sample of audio_data, the rest has been played through soundcard already

        if (samples.size() > audio_data.size()) audio_data.resize(samples.size(), 0.0);

        if (audio_mode == 1) for (size_t i = 0; i < samples.size(); i++) audio_data[i] += samples[i];
        else if (audio_mode == 2) for (size_t i = 0; i < samples.size(); i++) audio_data[i] += samples[i] * audio_volume;
    }

}