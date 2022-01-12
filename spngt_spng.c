#include "spngt.h"

static int read_fn(struct spng_ctx *ctx, void *user, void *data, size_t n)
{
    struct spngt_buf_state *state = user;
    if(n > state->bytes_left) return SPNG_IO_EOF;

    unsigned char *dst = data;
    unsigned char *src = state->data;

#if defined(TEST_SPNG_STREAM_READ_INFO)
    printf("libspng bytes read: %lu\n", n);
#endif

    memcpy(dst, src, n);

    state->bytes_left -= n;
    state->data += n;

    return 0;
}

static int write_fn(spng_ctx *ctx, void *user, void *data, size_t length)
{
    struct spngt_buf_state *state = user;

    if(length > state->bytes_left)
    {
        printf("write callback error\n");
        return SPNG_IO_ERROR;
    }

    memcpy(state->data + length, data, length);
    state->bytes_left -= length;
    state->data += length;

    return 0;
}

void spngt_print_version_spng(void)
{
    printf("spng header version: %u.%u.%u, library version: %s\n",
            SPNG_VERSION_MAJOR, SPNG_VERSION_MINOR, SPNG_VERSION_PATCH,
            spng_version_string());
}

int spngt_decode_spng(struct spngt_params *params)
{
    int r;
    spng_ctx *ctx = spng_ctx_new(0);

    if(ctx == NULL)
    {
        printf("spng_ctx_new() failed\n");
        return SPNGT_EMEM;
    }

    /*struct spngt_buf_state state;
    state.data = buf;
    state.bytes_left = size;

    r = spng_set_png_stream(ctx, libspng_read_fn, &state);

    if(r)
    {
        printf("spng_set_png_stream() error: %s\n", spng_strerror(r));
        goto err;
    }*/

    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    r = spng_set_png_buffer(ctx, params->png, params->png_size);

    if(r)
    {
        printf("spng_set_png_buffer() error: %s\n", spng_strerror(r));
        goto err;
    }

    r = spng_get_ihdr(ctx, &params->ihdr);

    if(r)
    {
        printf("spng_get_ihdr() error: %s\n", spng_strerror(r));
        goto err;
    }

    spng_get_plte(ctx, &params->plte);

    r = spng_decoded_image_size(ctx, params->fmt, &params->image_size);
    if(r) goto err;

    params->image = malloc(params->image_size);
    if(params->image == NULL) goto err;

    r = spng_decode_image(ctx, params->image, params->image_size, params->fmt, 0);

    if(r)
    {
        printf("spng_decode_image() error: %s\n", spng_strerror(r));
        goto err;
    }

    spng_ctx_free(ctx);

goto skip_err;

err:
    spng_ctx_free(ctx);
    if(params->image != NULL) free(params->image);
    return SPNGT_ERROR;

skip_err:

    return 0;
}

int spngt_encode_spng(struct spngt_params *params)
{
    enum spng_errno ret = 0;
    spng_ctx *ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    if(ctx == NULL)
    {
        printf("spng_ctx_new() failed\n");
        return 2;
    }

    struct spngt_buf_state state = { .data = params->png, .bytes_left = params->png_size };

    spng_set_png_stream(ctx, write_fn, &state);

    if(params->override_defaults)
    {
        spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL, params->compression_level);
        spng_set_option(ctx, SPNG_IMG_WINDOW_BITS, params->window_bits);
        spng_set_option(ctx, SPNG_IMG_MEM_LEVEL, params->mem_level);
        spng_set_option(ctx, SPNG_IMG_COMPRESSION_STRATEGY, params->strategy);
        spng_set_option(ctx, SPNG_FILTER_CHOICE, params->filter_choice);
    }

    ret = spng_set_ihdr(ctx, &params->ihdr);
    if(ret) goto err;

    if(params->plte.n_entries)
    {
        ret = spng_set_plte(ctx, &params->plte);
        if(ret) goto err;
    }

    ret = spng_encode_image(ctx, params->image, params->image_size, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
    if(ret) goto err;

    //params->png = spng_get_png_buffer(ctx, &params->png_size, &ret);
    //if(ret) goto err;
    params->png_size = params->png_size - state.bytes_left;

err:
    if(ret) printf("encode error (%d): %s\n", ret, spng_strerror(ret));

    spng_ctx_free(ctx);

    return ret;
}
