#include "spngt.h"

#include "lodepng.h"

void spngt_print_version_lodepng(void)
{
    printf("lodepng %s\n", LODEPNG_VERSION_STRING);
}

int spngt_encode_lodepng(struct spngt_params *params)
{
    return SPNGT_ENOTSUPP;
    unsigned char *out;
    size_t encoded_size;

    unsigned int e;
    int width = params->ihdr.width;
    int height = params->ihdr.height;

    e = lodepng_encode_memory(&out, &encoded_size, params->image, width, height, params->ihdr.color_type, params->ihdr.bit_depth);

    if(e) return SPNGT_ERROR;

    return 0;
}

int spngt_decode_lodepng(struct spngt_params *params)
{
    if(params->fmt != SPNG_FMT_RGBA8) return SPNGT_ENOTSUPP;

    unsigned int width, height;
    unsigned char *img_out = NULL;

    unsigned int e = lodepng_decode32(&img_out, &width, &height, params->png, params->png_size);

    if(e) return SPNGT_ERROR;

    params->image = img_out;
    params->ihdr.width = width;
    params->ihdr.height = height;

    return 0;
}
