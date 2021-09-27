#ifndef SPNGT_BENCH_H
#define SPNGT_BENCH_H

#include <spng.h>

struct spngt_bench_params
{
    void *image;
    size_t image_size;
    
    void *png;
    size_t png_size;
    
    struct spng_ihdr ihdr;
    struct spng_plte plte;
};


#endif /* SPNGT_BENCH_H */