#define _POSIX_C_SOURCE 199309L
#include <time.h>

#include <inttypes.h>
#include <stdio.h>

#include "test_png.h"

#include "test_spng.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lodepng.h"

long get_interval(struct timespec *a, struct timespec *b)
{
    if ((b->tv_nsec - a->tv_nsec) < 0) return b->tv_nsec - a->tv_nsec + 1000000000;
    else return b->tv_nsec - a->tv_nsec;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("no input file\n");
        return 1;
    }

    FILE *png;
    unsigned char *pngbuf;
    png = fopen(argv[1], "r");
    if(png==NULL)
    {
        printf("error opening input file %s\n", argv[1]);
        return 1;
    }

    fseek(png, 0, SEEK_END);
    long siz_pngbuf = ftell(png);
    rewind(png);

    if(siz_pngbuf < 1) return 1;

    pngbuf = malloc(siz_pngbuf);
    if(pngbuf==NULL)
    {
        printf("malloc() failed\n");
        return 1;
    }

    if(fread(pngbuf, siz_pngbuf, 1, png) != 1)
    {
        printf("fread() failed\n");
        return 1;
    }

    struct timespec a, b;
    long best_libpng = LONG_MAX, best_spng = LONG_MAX, best_stb = LONG_MAX, best_lodepng = LONG_MAX;

    int i, runs = 5;
    for(i=0; i < runs; i++)
    {
        /* libpng */
        size_t img_png_size;

        clock_gettime(CLOCK_MONOTONIC, &a);
        unsigned char *img_png = getimage_libpng(pngbuf, siz_pngbuf, &img_png_size, SPNG_FMT_RGBA8, 0);
        clock_gettime(CLOCK_MONOTONIC, &b);

        long time_libpng = get_interval(&a, &b);
        if(best_libpng > time_libpng) best_libpng = time_libpng;

        free(img_png);

        /* libspng */
        struct spng_ihdr ihdr;
        size_t img_spng_size;

        clock_gettime(CLOCK_MONOTONIC, &a);
        unsigned char *img_spng = getimage_libspng(pngbuf, siz_pngbuf, &img_spng_size, SPNG_FMT_RGBA8, 0, &ihdr);
        clock_gettime(CLOCK_MONOTONIC, &b);

        long time_spng = get_interval(&a, &b);
        if(best_spng > time_spng) best_spng = time_spng;

        free(img_spng);

        /* stb_image */
        int x, y, bpp;

        clock_gettime(CLOCK_MONOTONIC, &a);
        unsigned char *img_stb = stbi_load_from_memory(pngbuf, siz_pngbuf, &x, &y, &bpp, 4);
        clock_gettime(CLOCK_MONOTONIC, &b);

        long time_stb = get_interval(&a, &b);
        if(best_stb > time_stb) best_stb = time_stb;

        free(img_stb);

        /* lodepng */
        unsigned int width, height;

        clock_gettime(CLOCK_MONOTONIC, &a);
        int e = lodepng_decode32(&img_png, &width, &height, pngbuf, siz_pngbuf);
        clock_gettime(CLOCK_MONOTONIC, &b);

        long time_lodepng = get_interval(&a, &b);
        if(best_lodepng > time_lodepng) best_lodepng = time_lodepng;
    }

    printf("libpng:    %lu usec\n", best_libpng / 1000);
    printf("spng:      %lu usec\n", best_spng / 1000);
    printf("stb_image: %lu usec\n", best_stb / 1000);
    printf("lodepng:   %lu usec\n", best_lodepng / 1000);

    free(pngbuf);

    return 0;
}
