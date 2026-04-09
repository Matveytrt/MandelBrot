#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <raylib.h>

#define _MAX_ITER_    255
#define _WINDOW_WDTH_ 800
#define _WINDOW_HGHT_ 600

typedef unsigned char byte;

//g++ -o mndlbrt mandelbrot.cpp -lraylib

void DrawFractal(int wdth, int hght, int max_iter);

int main()
{
    int scr_wdth = _WINDOW_WDTH_;
    int scr_hght = _WINDOW_HGHT_;
    int max_iter = _MAX_ITER_;

    InitWindow(scr_wdth, scr_hght, "MandelBrot");
    SetTargetFPS(60);
    printf("Window created successfully!\n");

    if (IsWindowReady()) {DrawFractal(scr_wdth, scr_hght, max_iter);}

    CloseWindow();
    printf("Window closed!\n");
}

void DrawFractal(int wdth, int hght, int max_iter)
{

    float x_min = -2.0f, x_max = 2.0f;
    float y_min = -1.5f, y_max = 1.5f;
    float lmax = 10.0;

    float scale_coef = 1.1;
    float mov_coef = 0.1;

    float l2max = lmax * lmax;
    float dx = (x_max - x_min) / 2, dy = (y_max - y_min) / 2; 
    float x0 = (x_max + x_min) / 2, y0 = (y_max + y_min) / 2; 

    float x = 0, y = 0;
    byte iter = 0;
    int size = wdth * hght;

    Color *clr_data = (Color *) calloc(size, sizeof(Color));
    assert(clr_data);

    Texture2D texture = {};

    Image image =   {
                        clr_data,
                        wdth,
                        hght,
                        1,
                        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 
                    };

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        int index = 0;

        if(IsKeyDown(KEY_EQUAL)) {  dx /= scale_coef; x_max = x0 + dx; x_min = x0 - dx;
                                    dy /= scale_coef; y_max = y0 + dy; y_min = y0 - dy; }

        if(IsKeyDown(KEY_MINUS)) {  dx *= scale_coef; x_max = x0 + dx; x_min = x0 - dx;
                                    dy *= scale_coef; y_max = y0 + dy; y_min = y0 - dy; }

        if(IsKeyDown(KEY_W))     {  y0 += mov_coef; y_max = y0 + dy; y_min = y0 - dy; }
        if(IsKeyDown(KEY_A))     {  x0 += mov_coef; x_max = x0 + dx; x_min = x0 - dx; }
        if(IsKeyDown(KEY_S))     {  y0 -= mov_coef; y_max = y0 + dy; y_min = y0 - dy; }
        if(IsKeyDown(KEY_D))     {  x0 -= mov_coef; x_max = x0 + dx; x_min = x0 - dx; } 

        for (int py = 0; py < hght; py++) 
        {
            y = y_min + (y_max - y_min) * ((float) py / hght);

            for (int px = 0; px < wdth; px++) 
            {
                x = x_min + (x_max - x_min) * ((float) px / wdth);

                float zx = x, zy = y;
                
                for (iter = 0; iter < max_iter; iter++) 
                {
                    float x2 = zx * zx;
                    float y2 = zy * zy;

                    float xy = zx * zy;
                    
                    zx = x2 - y2 + x;
                    zy = 2 * xy + y;

                    if ((x2 + y2) > l2max) {break;}
                }

                if (iter == max_iter) {clr_data[index++] = BLACK;}
    
                else    
                {
                    byte r = iter + 255;
                    byte g = iter + 255;
                    byte b = iter + 255;
                    clr_data[index++] = {r, g, b, 255};
                }
            }
        }

        texture = LoadTextureFromImage(image);
        DrawTexture(texture, 0, 0, WHITE);
        DrawFPS(10, 10); //to do rdtsc
        EndDrawing();
    }

    free(clr_data);
}
