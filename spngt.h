#ifndef SPNGT_H
#define SPNGT_H

#include <spng.h>

#include <string.h>
#include <inttypes.h>
#include <stdio.h>

enum spngt_errno
{
    SPNGT_OK = 0,
    SPNGT_EINVAL = 1, /* invalid argument */
    SPNGT_EMEM, /* out of memory */
    SPNGT_ERROR, /* generic error */
    SPNGT_ENOTSUPP, /* (operation) not supported */
};

enum spngt_zlib_strategy
{
    SPNGT_Z_FILTERED = 1,
    SPNGT_Z_HUFFMAN_ONLY = 2,
    SPNGT_Z_RLE = 3,
    SPNGT_Z_FIXED = 4,
    SPNGT_Z_DEFAULT_STRATEGY = 0
};

struct spngt_params
{
    void *image;
    size_t image_size;

    void *png;
    size_t png_size;

    /* Output format when decoding, input format when encoding */
    enum spng_format fmt;

    struct spng_ihdr ihdr;
    struct spng_plte plte;

    /* Applies to all options below */
    int override_defaults;

    /* zlib options */
    int compression_level;
    int window_bits;
    int mem_level;
    enum spngt_zlib_strategy strategy;

    enum spng_filter_choice filter_choice;
};

struct spngt_buf_state
{
    unsigned char *data;
    size_t bytes_left;
    int error;
};

#define SPNGT_LIBS(XX) \
    XX(libpng) \
    XX(spng) \
    XX(stb) \
    XX(lodepng) \
    XX(wuffs)

#define XX(lib) \
    void spngt_print_version_##lib(void); \
    int spngt_decode_##lib(struct spngt_params *params); \
    int spngt_encode_##lib(struct spngt_params *params);

    SPNGT_LIBS(XX)
#undef XX

#endif /* SPNGT_H */