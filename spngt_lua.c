#include <spngt.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define SPNGT_LUA_FILTERS(XX) \
    XX(FILTER_CHOICE_NONE) \
    XX(FILTER_CHOICE_SUB) \
    XX(FILTER_CHOICE_UP) \
    XX(FILTER_CHOICE_AVG) \
    XX(FILTER_CHOICE_PAETH) \
    XX(FILTER_CHOICE_ALL) \
    XX(DISABLE_FILTERING)

#define SPNGT_LUA_ZSTRATEGY(XX) \
    XX(Z_FILTERED) \
    XX(Z_HUFFMAN_ONLY) \
    XX(Z_RLE) \
    XX(Z_FIXED) \
    XX(Z_DEFAULT_STRATEGY)

#define SPNGT_LUA_ZCOMPRESSION(XX) \
    XX(Z_NO_COMPRESSION) \
    XX(Z_BEST_SPEED) \
    XX(Z_BEST_COMPRESSION) \
    XX(Z_DEFAULT_COMPRESSION)

#define SPNGT_LUA_FMT(XX) \
    XX(RGBA8) \
    XX(RGB8)


/* Initialize and set defaults for values that may be set from Lua */
static void reset_params(struct spngt_params *params)
{
    params->decode_runs = SPNGT_DEFAULT_DECODE_RUNS;
    params->encode_runs = SPNGT_DEFAULT_ENCODE_RUNS;

    params->fmt = SPNG_FMT_PNG;

    params->override_defaults = 0;
    params->compression_level = -1;
    params->window_bits = 15;
    params->mem_level = 8;
    params->strategy = SPNGT_Z_DEFAULT_STRATEGY;
    params->filter_choice = SPNG_FILTER_CHOICE_ALL;
}

static struct spngt_params *check_params(lua_State *L, int arg)
{
    struct spngt_params *params = luaL_checkudata(L, 1, "spngt_file");

    if(params->image == NULL) luaL_argerror(L, 1, "invalid object (file already discarded?)");

    return params;
}

static int read_params(lua_State *L, int arg, struct spngt_params *params)
{
    luaL_checktype(L, arg, LUA_TTABLE);

    /* Values that aren't set will be their default value */
    reset_params(params);

    int fmt = lua_getfield(L, arg, "fmt");

    int type_compression_level = lua_getfield(L, arg, "compression_level");
    int type_window_bits = lua_getfield(L, arg, "window_bits");
    int type_mem_level = lua_getfield(L, arg, "mem_level");
    int type_strategy = lua_getfield(L, arg, "strategy");
    int type_filter_choice = lua_getfield(L, arg, "filter_choice");

    if(fmt) params->fmt = luaL_checkinteger(L, -6);

    if(type_compression_level) params->compression_level = luaL_checkinteger(L, -5);
    if(type_window_bits) params->window_bits = luaL_checkinteger(L, -4);
    if(type_mem_level) params->mem_level = luaL_checkinteger(L, -3);
    if(type_strategy) params->strategy = luaL_checkinteger(L, -2);
    if(type_filter_choice) params->filter_choice = luaL_checkinteger(L, -1);


    if(type_compression_level ||
       type_window_bits ||
       type_mem_level ||
       type_strategy ||
       type_filter_choice)
    {
        params->override_defaults = 1;
    }
    else /* if nothing is set it's probably user error */
    {
        luaL_argerror(L, arg, "table is empty");
    }

    lua_pop(L, 6);

    return 0;
}

static void push_ihdr(lua_State *L, struct spng_ihdr *ihdr)
{
    lua_createtable(L, 0, 5);
    lua_pushinteger(L, ihdr->width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, ihdr->height);
    lua_setfield(L, -2, "height");
    lua_pushinteger(L, ihdr->bit_depth);
    lua_setfield(L, -2, "bit_depth");
    lua_pushinteger(L, ihdr->color_type);
    lua_setfield(L, -2, "color_type");
    lua_pushinteger(L, ihdr->interlace_method);
    lua_setfield(L, -2, "interlace_method");
}

static void push_results(lua_State *L, struct spngt_params *params, uint64_t time)
{
    lua_createtable(L, 0, 3);

    lua_pushinteger(L, params->png_size);
    lua_setfield(L, -2, "png_size");

    push_ihdr(L, &params->ihdr);
    lua_setfield(L, -2, "ihdr");

    lua_pushinteger(L, time);
    lua_setfield(L, -2, "time");
}

static int prefetch_file(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);

    struct spngt_params *params = lua_newuserdata(L, sizeof(struct spngt_params));
    luaL_setmetatable(L, "spngt_file");

    memset(params, 0, sizeof(struct spngt_params));

    int ret = spngt_prefetch_file(filename, params);

    if(ret) return luaL_error(L, "Failed to prefetch: %s", spngt_strerror(ret));

    params->fmt = SPNG_FMT_PNG;

    ret = spngt_decode_spng(params);

    if(ret) return luaL_error(L, "Failed to decode: %s", spngt_strerror(ret));

    /* fields will be reused for encode benchmark */
    free(params->png);
    params->png = NULL;
    params->png_size = 0;

    return 1;
}

static int get_ihdr(lua_State *L)
{
    struct spngt_params *params = check_params(L, 1);

    push_ihdr(L, &params->ihdr);

    return 1;
}

static int file_gc(lua_State *L)
{
    struct spngt_params *params = luaL_checkudata(L, 1, "spngt_file");

    free(params->png);
    free(params->image);

    params->png = NULL;
    params->image = NULL;

    return 0;
}

static int discard_file(lua_State *L)
{
    struct spngt_params *params = check_params(L, 1);

    return file_gc(L);
}

static int encode_bench(lua_State *L)
{
    struct spngt_params *params = check_params(L, 1);

    read_params(L, 2, params);

    params->fmt = SPNG_FMT_PNG; /* For now */

    /* Expected to be large enough for all configurations */
    size_t encode_buffer_size = params->image_size + (1 << 20);

    params->png_size = encode_buffer_size;

    if(params->png == NULL) params->png = malloc(params->png_size);

    if(params->png == NULL) return luaL_error(L, "failed to preallocate encode buffer");

    uint64_t a, b;
    size_t encoded_size;
    enum spngt_errno e = 0;
    uint64_t best = UINT64_MAX;

    int i;
    for(i=0; i < params->encode_runs; i++)
    {
        a = spngt_time();
        e = spngt_encode_spng(params);
        b = spngt_time();

        if(e) return luaL_error(L, "%s encode failed: %s\n","spng", spngt_strerror(e));

        spngt_measure(a, b, &best);

        /* Maintain actual buffer size for subsequent iterations */
        encoded_size = params->png_size;
        params->png_size = encode_buffer_size;
    }

    params->png_size = encoded_size;

    push_results(L, params, best);

    return 1;
}

static int invalid_key(lua_State *L)
{
    return luaL_error(L, "invalid key (%s)", lua_tostring(L, 2));
}

static const luaL_Reg spngt_lib[] =
{
    { "prefetch", prefetch_file },
#define XX(const) { #const, NULL },
    SPNGT_LUA_ZSTRATEGY(XX)
    SPNGT_LUA_FILTERS(XX)
    SPNGT_LUA_FMT(XX)
    SPNGT_LIBS(XX)
#undef XX
    { NULL, NULL }
};

#define SPNGT_STR(x) _SPNG_STR(x)
#define _SPNGT_STR(x) #x

int luaopen_spngt(lua_State *L)
{
    luaL_checkversion(L);
    luaL_newlibtable(L, spngt_lib);

    /* Make it obvious when unset values are accessed */
    lua_newtable(L);
        lua_pushcfunction(L, invalid_key);
        lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    luaL_setfuncs(L, spngt_lib, 0);

    luaL_newmetatable(L, "spngt_file");
        lua_pushcfunction(L, file_gc);
        lua_setfield(L, -2, "__gc");
        lua_newtable(L);
            lua_pushcfunction(L, get_ihdr);
            lua_setfield(L, -2, "get_ihdr");
            lua_pushcfunction(L, discard_file);
            lua_setfield(L, -2, "discard");
            lua_pushcfunction(L, encode_bench);
            lua_setfield(L, -2, "encode_benchmark");
        lua_setfield(L, -2, "__index");
    lua_pop(L, 1);


#define XX(const) lua_pushinteger(L, SPNGT_##const); lua_setfield(L, -2, #const);
    SPNGT_LUA_ZSTRATEGY(XX)
#undef XX

#define XX(const) lua_pushinteger(L, SPNG_##const); lua_setfield(L, -2, #const);
    SPNGT_LUA_FILTERS(XX)
#undef XX

#define XX(const) lua_pushinteger(L, SPNG_FMT_##const); lua_setfield(L, -2, #const);
    SPNGT_LUA_FMT(XX)
#undef XX

#define XX(const) lua_pushinteger(L, const); lua_setfield(L, -2,#const);
    SPNGT_LIBS(XX)
#undef XX

    lua_pushinteger(L, -1);
    lua_setfield(L, -2, "Z_DEFAULT_COMPRESSION");

    lua_pushinteger(L, SPNGT_DEFAULT_DECODE_RUNS);
    lua_setfield(L, -2, "DEFAULT_DECODE_RUNS");

    lua_pushinteger(L, SPNGT_DEFAULT_ENCODE_RUNS);
    lua_setfield(L, -2, "DEFAULT_ENCODE_RUNS");

    return 1;
}

int spngt_exec_script(int argc, char **argv)
{
    lua_State *L = luaL_newstate();
    const char *filename = argv[1];

    luaL_openlibs(L);

    luaL_requiref(L, "spngt", luaopen_spngt, 0);

    lua_createtable(L, argc, 0);

    int i;
    for(i=1; i < argc; i++)
    {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - 1);
    }

    /* arg = { [0] = "<script_path>", ... } */
    lua_setglobal(L, "arg");

    int ret = luaL_dofile(L, filename);

    if(ret)
    {
        const char *err = lua_tostring(L, lua_gettop(L));
        fprintf(stderr, "script error: %s\n", err);
    }

    lua_close(L);

    return ret;
}