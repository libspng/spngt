#include "spngt.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


static void write_func(void *context, void *data, int size)
{
    struct spngt_buf_state *state = context;

    if(size > state->bytes_left)
    {
        state->error = SPNGT_ERROR;
        return;
    }

    memcpy(state->data, data, size);

    state->bytes_left -= size;
}

void spngt_print_version_stb(void)
{
    printf("stb_image v2.27, stb_image_write v1.16\n");
}

int spngt_encode_stb(struct spngt_params *params)
{
    return SPNGT_ENOTSUPP;
    enum spngt_errno ret = 0;
    struct spngt_buf_state buf_state = { .data = params->png, .bytes_left = params->png_size };

    int comp;
    int w = params->ihdr.width;
    int h = params->ihdr.height;
    int stride = params->image_size / params->ihdr.height;

    if(params->ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR) comp = 3;
    else if(params->ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA) comp = 4;
    else return SPNGT_ENOTSUPP;

    if(params->override_defaults)
    {
        stbi_write_png_compression_level = params->compression_level;
    }
    else
    {
        stbi_write_png_compression_level = 8; /* (re)set global to default value */
    }

    int e = stbi_write_png_to_func(write_func, &buf_state, w, h, comp, params->png, stride);

    if(e) ret = SPNGT_ERROR;

    return ret;
}

int spngt_decode_stb(struct spngt_params *params)
{
    enum spngt_errno ret = 0;

    int x, y, bpp;

    params->image = stbi_load_from_memory(params->png, params->png_size, &x, &y, &bpp, 4);

    if(params->image == NULL) return SPNGT_ERROR;

    params->ihdr.width = x;
    params->ihdr.height = y;

    return ret;
}
