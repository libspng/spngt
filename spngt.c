#if defined(_WIN32)
    #include "windows.h"
#else
    #define _POSIX_C_SOURCE 199309L
    #include <time.h>
#endif

#include <spngt.h>

typedef int spngt_fn(struct spngt_params *params);

struct spngt__library
{
    const char *name;
    spngt_fn *decode_fn;
    spngt_fn *encode_fn;
    uint64_t best;
    uint64_t encoded_size;
};

static int decode_runs = SPNGT_DEFAULT_DECODE_RUNS;
static int encode_runs = SPNGT_DEFAULT_ENCODE_RUNS;

static struct spngt__library libraries[10];
static int library_count = 0;

static void add_library_struct(const struct spngt__library *lib)
{
    libraries[library_count] = *lib;
    libraries[library_count].best = UINT64_MAX;
    library_count++;
}

#define add_library(lib_name) add_library_struct(&(struct spngt__library)\
    {\
        .name = #lib_name, \
        .decode_fn = spngt_decode_##lib_name, \
        .encode_fn = spngt_encode_##lib_name, \
        .best = UINT64_MAX, \
        .encoded_size = UINT64_MAX \
    })

static void print_times(void)
{
    const struct spngt__library *lib = libraries;

    int i;
    for(i=0; i < library_count; i++, lib++)
    {
        if(lib->best == UINT64_MAX) continue;

        char pad_str[] = "      ";
        pad_str[sizeof(pad_str) - strlen(lib->name) + 2] = '\0';

        printf("%s %s%" PRIu64 " usec\n", lib->name, pad_str, lib->best / (uint64_t)1000);
    }
}

static void print_encode_results(void)
{
    const struct spngt__library *lib = libraries;

    int i;
    for(i=0; i < library_count; i++, lib++)
    {
        if(lib->best == UINT64_MAX) continue;

        char pad_str[] = "      ";
        pad_str[sizeof(pad_str) - strlen(lib->name) + 2] = '\0';

        printf("%s %s", lib->name, pad_str);
        printf("%" PRIu64 " usec    ", lib->best / (uint64_t)1000);
        printf("%" PRIu64 " KB\n", lib->encoded_size / (uint64_t)1024);
    }
}

static const char *zstrategy_str(enum spngt_zlib_strategy strategy)
{
    switch(strategy)
    {
        case SPNGT_Z_FILTERED: return "Z_FILTERED";
        case SPNGT_Z_HUFFMAN_ONLY: return "Z_HUFFMAN_ONLY";
        case SPNGT_Z_RLE: return "Z_RLE";
        case SPNGT_Z_FIXED: return "Z_FIXED";
        case SPNGT_Z_DEFAULT_STRATEGY: return "Z_DEFAULT_STRATEGY";
        default: return "(UNKNOWN)";
    }
}

static void print_encode_params(struct spngt_params *params)
{
    if(params->override_defaults)
    {
        printf("compression level: %d\n", params->compression_level);
        printf("zlib strategy: %s\n", zstrategy_str(params->strategy));
        printf("filters: ");

        if(params->filter_choice != SPNG_FILTER_CHOICE_ALL)
        {
            if(params->filter_choice & SPNG_FILTER_CHOICE_NONE) printf("NONE ");
            if(params->filter_choice & SPNG_FILTER_CHOICE_SUB) printf("SUB ");
            if(params->filter_choice & SPNG_FILTER_CHOICE_UP) printf("UP ");
            if(params->filter_choice & SPNG_FILTER_CHOICE_AVG) printf("AVG ");
            if(params->filter_choice & SPNG_FILTER_CHOICE_PAETH) printf("PAETH ");
            if(params->filter_choice == SPNG_DISABLE_FILTERING) printf("(NO FILTERING)");
        }
        else printf("ALL");

        printf("\n\n");
    }
    else printf("Encode parameters: (library defaults)\n\n");
}

uint64_t spngt_time(void)
{
#if defined(_WIN32)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER time = { .LowPart = ft.dwLowDateTime, .HighPart = ft.dwHighDateTime };
    return time.QuadPart * 100;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
#endif
}

void spngt_measure(uint64_t start, uint64_t end, uint64_t *best)
{
    uint64_t elapsed = end - start;

    if(*best > elapsed) *best = elapsed;
}

const char *spngt_strerror(enum spngt_errno err)
{
    switch(err)
    {
        case SPNGT_OK: return "success";
        case SPNGT_EINVAL: return "invalid argument";
        case SPNGT_EMEM: return "out of memory";
        case SPNGT_ERROR: return "generic error";
        case SPNGT_ENOTSUPP: return "operation not supported";
        default: return "unknown error";
    }

    return "unknown error";
}

static int decode_and_measure(enum spngt_library library, struct spngt_params *params)
{
    uint64_t a, b;
    enum spngt_errno e = 0;
    struct spngt__library *lib = &libraries[library];

    int i;
    for(i=0; i < decode_runs; i++)
    {
        a = spngt_time();
        e = lib->decode_fn(params);
        b = spngt_time();

        spngt_measure(a, b, &lib->best);

        free(params->image);
        params->image = NULL;

        if(e)
        {
            printf("ERROR: %s decode failed: %s\n", lib->name, spngt_strerror(e));
            return e;
        }
    }

    return 0;
}

int encode_and_measure(enum spngt_library library, struct spngt_params *params)
{
    uint64_t a, b;
    enum spngt_errno e = 0;
    struct spngt__library *lib = &libraries[library];
    size_t encode_buffer_size = params->png_size;

    int i;
    for(i=0; i < encode_runs; i++)
    {
        a = spngt_time();
        e = lib->encode_fn(params);
        b = spngt_time();

        if(e)
        {
            printf("ERROR: %s encode failed: %s\n", lib->name, spngt_strerror(e));
            return e;
        }

        spngt_measure(a, b, &lib->best);
        lib->encoded_size = params->png_size;
        params->png_size = encode_buffer_size;
    }

    return 0;
}

static int decode_benchmark(struct spngt_params *params)
{
    enum spngt_errno e = 0;

    int i;
    for(i=0; i < library_count; i++)
    {
        e = decode_and_measure(i, params);

        if(e) return e;
    }

    print_times();

    return 0;
}

static int encode_benchmark(struct spngt_params *params)
{
    enum spngt_errno ret = 0;

    print_encode_params(params);

    /* Conservative estimate that doesn't require realloc()'s */
    size_t encode_buffer_size = params->image_size + (1 << 20);

    params->png_size = encode_buffer_size;
    params->png = malloc(params->png_size);

    if(params->png == NULL)
    {
        printf("ERROR: failed to preallocate encode buffer\n");
        return SPNGT_EMEM;
    }

    int i;
    for(i=0; i < library_count; i++)
    {
        encode_and_measure(i, params);
    }

    free(params->png);

    print_encode_results();

    return ret;
}

static void print_png_info(struct spng_ihdr *ihdr, const char *path, size_t siz_pngbuf)
{
    const char *clr_type_str;
    if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE)
        clr_type_str = "GRAY";
    else if(ihdr->color_type == SPNG_COLOR_TYPE_TRUECOLOR)
        clr_type_str = "RGB";
    else if(ihdr->color_type == SPNG_COLOR_TYPE_INDEXED)
        clr_type_str = "INDEXED";
    else if(ihdr->color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA)
        clr_type_str = "GRAY-ALPHA";
    else
        clr_type_str = "RGBA";

    const char *name = strrchr(path, '/');

    if(!name) name = strrchr(path, '\\');

    if(name) name++;

    if(!name) name = path;

    const char *unit = "B";

    if(siz_pngbuf > 1024)
    {
        unit = "KB";
        siz_pngbuf /= 1024;
    }

    printf("%s %zu %s  %s %" PRIu8 "-bit, %" PRIu32 "x%" PRIu32 " %s",
            name, siz_pngbuf, unit, clr_type_str, ihdr->bit_depth, ihdr->width, ihdr->height,
            ihdr->interlace_method ? "interlaced" : "non-interlaced\n\n");
}

int spngt_prefetch_file(const char *path, struct spngt_params *params)
{
    FILE *png;
    unsigned char *pngbuf;
    png = fopen(path, "rb");

    if(png == NULL)
    {
        printf("error opening input file %s\n", path);
        return 1;
    }

    fseek(png, 0, SEEK_END);
    long siz_pngbuf = ftell(png);
    rewind(png);

    if(siz_pngbuf < 1) return 1;

    pngbuf = malloc(siz_pngbuf);

    if(pngbuf == NULL)
    {
        printf("malloc() failed\n");
        return 1;
    }

    if(fread(pngbuf, siz_pngbuf, 1, png) != 1)
    {
        printf("fread() failed\n");
        return 1;
    }

    params->png = pngbuf;
    params->png_size = siz_pngbuf;
    return 0;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("no input file\n");
        return 1;
    }

    int do_encode = 0;

    const char *filepath = argv[1];
    const char *dot = strrchr(filepath, '.');

    if(dot && !strcmp(dot, ".lua"))
    {
        return spngt_exec_script(argc, argv);
    }

    if(argc > 2)
    {
        if(!strcmp(argv[2], "enc")) do_encode = 1;
        else printf("unrecognized option: %s\n", argv[2]);
    }

    if(!strcmp(argv[1], "info"))
    {
        #define XX(lib) spngt_print_version_##lib();
            SPNGT_LIBS(XX)
        #undef XX

        if(do_encode) printf("\nencode times are the best of %d runs\n", encode_runs);
        printf("\ndecode times are the best of %d runs\n", decode_runs);

        return 0;
    }

    enum spngt_errno ret = 0;

    struct spngt_params params =
    {
        .fmt = SPNG_FMT_PNG,
        .encode_runs = encode_runs
    };

    ret = spngt_prefetch_file(filepath, &params);

    if(ret) goto err;

#define XX(lib) add_library(lib);
    SPNGT_LIBS(XX)
#undef XX

    ret = spngt_decode_spng(&params);

    if(ret)
    {
        printf("failed to fetch PNG info: %s\n", spngt_strerror(ret));
        goto err;
    }

    print_png_info(&params.ihdr, argv[1], params.png_size);

    if(do_encode)
    {
        free(params.png); // not needed at this point, will be overwriten by encode_benchmark()

        params.override_defaults = 0;

        params.compression_level = 6;
        params.window_bits = 15;
        params.mem_level = 8;
        params.strategy = SPNGT_Z_FILTERED;

        params.filter_choice = SPNG_FILTER_CHOICE_ALL;

        ret = encode_benchmark(&params);

        free(params.image);
        params.png = NULL;
    }
    else
    {
        free(params.image);
        params.image = NULL;
        params.image_size = 0;

        params.fmt = SPNG_FMT_RGBA8;

        ret = decode_benchmark(&params);
    }

    if(ret) printf("%s benchmark failed (%s)\n", do_encode ? "encode" : "decode", spngt_strerror(ret));

err:
    free(params.png);

    return ret;
}
