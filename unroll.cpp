#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <x86intrin.h>

//#define DRAWMODE

#define _MAX_ITER_    255
#define _WINDOW_WDTH_ 800
#define _WINDOW_HGHT_ 600
#define _STEP_        8

#ifdef DRAWMODE
    #include <raylib.h>
    #define ISDRAWMODEON(on_par) on_par
#else
    #define ISDRAWMODEON(on_par) 
    #define BLACK {0, 0, 0, 255}

    typedef struct Color {
        unsigned char r;        // Color red value
        unsigned char g;        // Color green value
        unsigned char b;        // Color blue value
        unsigned char a;        // Color alpha value
    } Color;
#endif

typedef unsigned char byte;

//g++ -o mndlbrt mandelbrot.cpp -lraylib
// to do AOS

void DrawFractal(int wdth, int hght, int max_iter);

int main()
{
    int scr_wdth = _WINDOW_WDTH_;
    int scr_hght = _WINDOW_HGHT_;
    int max_iter = _MAX_ITER_;

    #ifdef DRAWMODE
        InitWindow(scr_wdth, scr_hght, "MandelBrot");
        printf("Window created successfully!\n");

        if (IsWindowReady()) {DrawFractal(scr_wdth, scr_hght, max_iter);}
        CloseWindow();
        printf("Window closed!\n");
    #else
        DrawFractal(scr_wdth, scr_hght, max_iter);
    #endif
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

    float x[_STEP_] = {};

    float zx[_STEP_] = {};
    float zy[_STEP_] = {};
    float x2[_STEP_] = {};
    float y2[_STEP_] = {};
    float xy[_STEP_] = {};

    int iter_data[_STEP_] = {};
    int cmp[_STEP_] = {};
    int mask = 0;

    int size = wdth * hght;

    Color *clr_data = (Color *) calloc(size, sizeof(Color));
    assert(clr_data);

    #ifdef DRAWMODE
        Image image =   {
                            clr_data,
                            wdth,
                            hght,
                            1,
                            PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 
                        };

        Texture2D texture = LoadTextureFromImage(image);
    #endif
    
    #ifdef DRAWMODE
    while ( !WindowShouldClose() )

    #else
    for (int i = 0; i < 100; i++)

    #endif
    {
        int index = 0;

        #ifdef DRAWMODE
            if(IsKeyDown(KEY_EQUAL)) {  dx /= scale_coef; x_max = x0 + dx; x_min = x0 - dx;
                                        dy /= scale_coef; y_max = y0 + dy; y_min = y0 - dy; 
                                        mov_coef /= scale_coef; }

            if(IsKeyDown(KEY_MINUS)) {  dx *= scale_coef; x_max = x0 + dx; x_min = x0 - dx;
                                        dy *= scale_coef; y_max = y0 + dy; y_min = y0 - dy; 
                                        mov_coef *= scale_coef; }

            if(IsKeyDown(KEY_W))     {  y0 -= mov_coef; y_max = y0 + dy; y_min = y0 - dy; }
            if(IsKeyDown(KEY_A))     {  x0 -= mov_coef; x_max = x0 + dx; x_min = x0 - dx; } 
            if(IsKeyDown(KEY_S))     {  y0 += mov_coef; y_max = y0 + dy; y_min = y0 - dy; }
            if(IsKeyDown(KEY_D))     {  x0 += mov_coef; x_max = x0 + dx; x_min = x0 - dx; }
        #else
            size_t start = __rdtsc();
        #endif

        for (int py = 0; py < hght; py++) 
        {
            float y = y_min + (y_max - y_min) * (((float) py) / hght);

            for (int px = 0; px < wdth; px += _STEP_) 
            {
                for (int idx = 0; idx < _STEP_; idx++) { x[idx] = x_min + (x_max - x_min) * (((float) px + idx) / wdth); }
                for (int idx = 0; idx < _STEP_; idx++) { zx[idx] = x[idx]; }
                for (int idx = 0; idx < _STEP_; idx++) { zy[idx] = y; }
                for (int idx = 0; idx < _STEP_; idx++) { cmp[idx] = 1; }
                for (int idx = 0; idx < _STEP_; idx++) { iter_data[idx] = 0; }
                
                
                for (int iter = 0; iter < max_iter; iter++)
                {
                    mask = 0;
                    for (int idx = 0; idx < _STEP_; idx++) { x2[idx] =  zx[idx] * zx[idx]; }
                    for (int idx = 0; idx < _STEP_; idx++) { y2[idx] =  zy[idx] * zy[idx]; }
                    for (int idx = 0; idx < _STEP_; idx++) { xy[idx] =  zx[idx] * zy[idx]; }

                    for (int idx = 0; idx < _STEP_; idx++) { zx[idx] = x2[idx] - y2[idx] + x[idx]; }
                    for (int idx = 0; idx < _STEP_; idx++) { zy[idx] = 2 * xy[idx] + y; }

                    for (int idx = 0; idx < _STEP_; idx++) { iter_data[idx] += cmp[idx]; }
                    
                    for (int idx = 0; idx < _STEP_; idx++) { if ((x2[idx] + y2[idx]) > l2max) cmp[idx] = 0; }
                    for (int idx = 0; idx < _STEP_; idx++) { mask |= (cmp[idx] << idx);}

                    if (mask == 0) { break; }
                }

                for (int idx = 0; idx < _STEP_; idx++) 
                { 
                    if (iter_data[idx] == max_iter) {clr_data[index++] = BLACK;}
        
                    else    
                    {
                        byte r = iter_data[idx] + 255;
                        byte g = iter_data[idx] + 255;
                        byte b = iter_data[idx] + 255;
                        clr_data[index++] = {r, g, b, 255};
                    }
                }
            }
        }

        #ifndef DRAWMODE
            size_t end = __rdtsc();
            printf("%d; %ld\n", i, end - start);
        #endif

        #ifdef DRAWMODE
            BeginDrawing();
            ClearBackground(WHITE);
            UpdateTexture(texture, clr_data);
            DrawTexture(texture, 0, 0, WHITE);
            DrawFPS(10, 10); //to do rdtsc
            EndDrawing();
        #endif
    }
    
    free(clr_data);
}