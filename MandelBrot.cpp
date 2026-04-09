#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <raylib.h>

#include <xmmintrin.h>
#include <emmintrin.h> 
#include <immintrin.h> 
// -mavx2

#define _MAX_ITER_    255
#define _WINDOW_WDTH_ 800
#define _WINDOW_HGHT_ 600
#define _CMP_MASK_    0b1111
#define _STEP_        4

typedef unsigned char byte;

//g++ -o mndlbrt mandelbrot.cpp -lraylib
// to do AOS

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
    float lmax = 4.0;

    float scale_coef = 1.1;
    float mov_coef = 0.1;

    __m128 l2max = _mm_set1_ps(lmax * lmax);

    float dx = (x_max - x_min) / 2, dy = (y_max - y_min) / 2; 
    float x0 = (x_max + x_min) / 2, y0 = (y_max + y_min) / 2; 

    int size = wdth * hght;

    Color *clr_data = (Color *) calloc(size, sizeof(Color));
    assert(clr_data);

    Image image =   {
                        clr_data,
                        wdth,
                        hght,
                        1,
                        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 
                    };

    Texture2D texture = LoadTextureFromImage(image);

    while (!WindowShouldClose())
    {
        int index = 0;

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

        float ax = (2 * dx) / wdth;
        float ay = (2 * dy) / hght;

        for (int py = 0; py < hght; py++) 
        {
            __m128 y = _mm_set1_ps(y_min + py * ay);

            for (int px = 0; px < wdth; px += _STEP_) 
            {
                float x0_val = x_min + (px + 0) * ax;
                float x1_val = x_min + (px + 1) * ax;
                float x2_val = x_min + (px + 2) * ax;
                float x3_val = x_min + (px + 3) * ax;
                __m128 x = _mm_set_ps(x3_val, x2_val, x1_val, x0_val);

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

                for (int idx = 0; idx < _STEP_; idx++) 
                { 
                    int iter = _mm_cvtsi128_si32(iter_data);
                    iter_data = _mm_srli_si128(iter_data, 4);

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
        }

        BeginDrawing();
        ClearBackground(WHITE);
        UpdateTexture(texture, clr_data);
        DrawTexture(texture, 0, 0, WHITE);
        DrawFPS(10, 10); //to do rdtsc
        EndDrawing();
    }

    free(clr_data);
}
