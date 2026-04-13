#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <assert.h>

#include <x86intrin.h>
#include <xmmintrin.h>
#include <emmintrin.h> 
#include <immintrin.h> 

// g++ -O3 -march=haswell -mtune=intel -mavx2 -funroll-loops -o xmm mndlbrt_xmm.cpp -lraylib

//#define DRAWMODE

#define _MAX_ITER_    255
#define _WINDOW_WDTH_ 800
#define _WINDOW_HGHT_ 600
#define _STEP_        4

#ifdef DRAWMODE
    #include <raylib.h>
    #define ISDRAWMODEON(on_par) on_par
#else
    #define ISDRAWMODEON(on_par) 
    #define BLACK {0, 0, 0, 255}

    typedef struct Color {
        unsigned char r;        
        unsigned char g;        
        unsigned char b;        
        unsigned char a;        
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

    __m128 l2max = _mm_set1_ps(lmax * lmax);
    float xval[_STEP_] = {};

    float dx = (x_max - x_min) / 2, dy = (y_max - y_min) / 2; 
    float x0 = (x_max + x_min) / 2, y0 = (y_max + y_min) / 2; 

    float ax = (2 * dx) / wdth;
    float ay = (2 * dy) / hght;

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

            ax = (2 * dx) / wdth;
            ay = (2 * dy) / hght;

        #else
            size_t start = __rdtsc();
        #endif


        for (int py = 0; py < hght; py++) 
        {
            __m128 y = _mm_set1_ps(y_min + py * ay);

            for (int px = 0; px < wdth; px += _STEP_) 
            {
                for (int idx = 0; idx < _STEP_; idx++) { xval[idx] = x_min + (px + idx) * ax;}
                __m128 x = _mm_set_ps(xval[3], xval[2], xval[1], xval[0]);

                __m128 zx = x;
                __m128 zy = y;
                __m128 cmp = _mm_set1_ps(0xffffffff);
                __m128i iter_data = _mm_set1_epi32(0);
                
                
                for (int iter = 0; iter < max_iter; iter++)
                {
                    __m128 x2 = _mm_mul_ps(zx, zx);
                    __m128 y2 = _mm_mul_ps(zy, zy);
                    __m128 xy = _mm_mul_ps(zx, zy);

                    zx = _mm_add_ps(_mm_sub_ps(x2, y2), x);
                    zy = _mm_add_ps(_mm_add_ps(xy, xy), y);

                    iter_data = _mm_sub_epi32(iter_data, _mm_castps_si128(cmp));
                    
                    __m128 r2 = _mm_add_ps(x2, y2);
                    cmp = _mm_cmplt_ps(r2, l2max);

                    int mask = _mm_movemask_ps(cmp) & 0xF;

                    if (mask == 0) { break; }
                }

                int iter_vals[_STEP_] = {};
                _mm_storeu_si128((__m128i*)iter_vals, iter_data);

                for (int idx = 0; idx < _STEP_; idx++) 
                { 
                    int iter = iter_vals[idx];

                    if (iter == max_iter) { clr_data[index++] = BLACK; }
        
                    else    
                    {
                        byte r = iter + 255;
                        byte g = iter + 255;
                        byte b = iter + 255;
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
