#include "spngt.h"

#include <png.h>

#if defined(TEST_SPNG_ANALYZE_MALLOC)
void *malloc_fn(png_structp png_ptr, png_alloc_size_t size)
{
    void *mem = malloc(size);
    printf("alloc: %zu\n", size);
    return mem;
}

void free_fn(png_structp png_ptr, png_voidp ptr)
{
    printf("dealloc\n");
    free(ptr);
}
#endif

static void read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct spngt_buf_state *state = png_get_io_ptr(png_ptr);

#if defined(TEST_SPNG_STREAM_READ_INFO)
    printf("libpng bytes read: %lu\n", length);
#endif

    if(length > state->bytes_left)
    {
        png_error(png_ptr, "read_fn error");
    }

    memcpy(data, state->data, length);
    state->bytes_left -= length;
    state->data += length;
}

static void write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct spngt_buf_state *state = png_get_io_ptr(png_ptr);

    if(length > state->bytes_left)
    {
        png_error(png_ptr, "write_fn error");
    }

    memcpy(state->data + length, data, length);
    state->bytes_left -= length;
    state->data += length;
}

static void flush_fn(png_structp png_ptr)
{

}

void spngt_print_version_libpng(void)
{
    unsigned int png_ver = png_access_version_number();

    printf("png header version: %u.%u.%u, library version: %u.%u.%u\n",
            PNG_LIBPNG_VER_MAJOR, PNG_LIBPNG_VER_MINOR, PNG_LIBPNG_VER_RELEASE,
            png_ver / 10000, png_ver / 100 % 100, png_ver % 100);
}

static unsigned char *getimage_libpng(unsigned char *buf, size_t size, size_t *out_size, int fmt, int flags)
{
    png_infop info_ptr;
    png_structp png_ptr;
    struct spngt_buf_state state;

    state.data = buf;
    state.bytes_left = size;

    unsigned char *image = NULL;
    png_bytep *row_pointers = NULL;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL)
    {
        printf("libpng init failed\n");
        return NULL;
    }
#if defined(TEST_SPNG_ANALYZE_MALLOC)
    png_set_mem_fn(png_ptr, NULL, malloc_fn, free_fn);
#endif

    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL)
    {
        printf("png_create_info_struct failed\n");
        return NULL;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
        printf("libpng error\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        if(image != NULL) free(image);
        if(row_pointers != NULL) free(row_pointers);
        return NULL;
    }

    png_set_read_fn(png_ptr, &state, read_fn);

    if(png_sig_cmp(buf, 0, 8))
    {
        printf("libpng: invalid signature\n");
        return NULL;
    }

    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, colour_type, interlace_type, compression_type;
    int filter_method;

    if(!png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                     &colour_type, &interlace_type, &compression_type, &filter_method))
    {
        printf("png_get_IHDR failed\n");
        return NULL;
    }

    if(flags & SPNG_DECODE_USE_GAMA)
    {
        double gamma;
        if(png_get_gAMA(png_ptr, info_ptr, &gamma))
            png_set_gamma(png_ptr, 2.2, gamma);
    }

    if(fmt == SPNG_FMT_RGBA16)
    {
        png_set_gray_to_rgb(png_ptr);

        png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);

        /* png_set_palette_to_rgb() + png_set_tRNS_to_alpha() */
        png_set_expand_16(png_ptr);

        png_set_swap(png_ptr);
    }
    else if(fmt == SPNG_FMT_RGBA8)
    {
        png_set_gray_to_rgb(png_ptr);

        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

        /* png_set_palette_to_rgb() + png_set_expand_gray_1_2_4_to_8() + png_set_tRNS_to_alpha() */
        png_set_expand(png_ptr);

        png_set_strip_16(png_ptr);
    }

    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    /* avoid calling malloc() for each row */
    size_t image_size = height * rowbytes;
    memcpy(out_size, &image_size, sizeof(size_t));

    image = malloc(image_size);

    if(image == NULL)
    {
        printf("libpng: malloc() failed\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    row_pointers = malloc(height * sizeof(png_bytep));
    if(row_pointers == NULL)
    {
        printf("libpng: malloc() failed\n");
        free(image);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    int k;
    for(k=0; k < height; k++)
    {
        row_pointers[k] = image + k * rowbytes;
    }

    png_read_image(png_ptr, row_pointers);

    png_read_end(png_ptr, info_ptr);

    free(row_pointers);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return image;
}

int spngt_decode_libpng(struct spngt_params *params)
{
    params->image = getimage_libpng(params->png, params->png_size, &params->image_size, params->fmt, 0);

    if(params->image == NULL) return SPNGT_ERROR;

    return 0;
}

int spngt_encode_libpng(struct spngt_params *params)
{
    png_infop info_ptr;
    png_structp png_ptr;
    png_bytep *row_pointers = NULL;
    struct spng_ihdr *ihdr = &params->ihdr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    if(png_ptr == NULL)
    {
        printf("libpng init failed\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return SPNGT_ERROR;
    }

    if(setjmp(png_jmpbuf(png_ptr)))
    {
        printf("libpng error\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        free(row_pointers);
        return SPNGT_ERROR;
    }

    if(params->override_defaults)
    {
        png_set_compression_level(png_ptr, params->compression_level);
        png_set_compression_window_bits(png_ptr, params->window_bits);
        png_set_compression_mem_level(png_ptr, params->mem_level);
        png_set_compression_strategy(png_ptr, params->strategy);
        png_set_filter(png_ptr, 0, params->filter_choice);
    }

    struct spngt_buf_state state = { .data = params->png, .bytes_left = params->png_size };

    png_set_write_fn(png_ptr, &state, write_fn, flush_fn);

    png_set_IHDR(png_ptr, info_ptr, ihdr->width, ihdr->height, ihdr->bit_depth, ihdr->color_type,
                 ihdr->interlace_method, ihdr->compression_method, ihdr->filter_method);

    if(ihdr->color_type == 3)
    {
        struct spng_plte *plte = &params->plte;
        struct png_color_struct png_plte[256];

        int i;
        for(i=0; i < plte->n_entries; i++)
        {
            png_plte[i].red = plte->entries[i].red;
            png_plte[i].green = plte->entries[i].green;
            png_plte[i].blue = plte->entries[i].blue;
        }

        png_set_check_for_invalid_index(png_ptr, 0);

        png_set_PLTE(png_ptr, info_ptr, png_plte, plte->n_entries);
    }

    row_pointers = malloc(ihdr->height * sizeof(png_bytep));
    size_t rowbytes = params->image_size / ihdr->height;

    uint32_t k;
    for(k=0; k < ihdr->height; k++)
    {
        row_pointers[k] = (unsigned char*)params->image + k * rowbytes;
    }

    png_write_info(png_ptr, info_ptr);

    png_write_image(png_ptr, row_pointers);

    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    free(row_pointers);

    params->png_size = params->png_size - state.bytes_left;

    return 0;
}
