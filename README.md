# SPNGT

SPNGT is a benchmarking and testing utility for [libspng](https://libspng.org),
[libpng](http://www.libpng.org/pub/png/libpng.html),
[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h),
[lodepng](https://github.com/lvandeve/lodepng) and
[wuffs](https://github.com/google/wuffs).

This is a standalone project, it is also used as a subproject in libspng for benchmarking
and to host non-libspng code.

## Dependencies

* [Meson](https://mesonbuild.com) 0.55.0 or later
* Git LFS for downloading the [benchmark images](https://github.com/libspng/benchmark_images/)

Meson will fall back to using [wraps](https://mesonbuild.com/Wrap-dependency-system-manual.html)
if the following are not found on the system:
* spng
* libpng
* stb_image
* lodepng
* stb
* zlib
* wuffs
* Lua

## Creating a build

```bash
# Add --default-library=static on Windows
meson build --buildtype=release
```

## Running the benchmark

```bash
cd build
meson test -t 0 --benchmark --suite decode #or encode
cat meson-logs/benchmarklog.txt
```

`ninja benchmark` runs all tests.

## Compile with Profile-guided optimization (PGO)

```bash
meson configure -Db_pgo=generate
ninja benchmark
meson configure -Db_pgo=use
ninja benchmark
cat meson-logs/benchmarklog.txt
```

# Notes

The benchmarks try to exclude system overhead by preloading the PNG into a buffer,
this provides better feedback for development,
real world performance may not be the same but has been accurate been so far.

## Command-line interface

Read one file and print the best decode/encode time for each library

```bash
$ spngt IMAGE.png
$ spngt IMAGE.png enc # For encode benchmarks
$ spngt info # Print library versions and other information
```

Or execute a lua script, the rest of the arguments will be exposed through the `arg` global

```bash
$ spngt main.lua SUITE_NAME FOO.png BAR.png #main.lua takes a base filename (SUITE_NAME) for its output files
```

## Running experiments

Results are written to `.csv` files in the build directory, see [main.lua](main.lua).

```bash
meson test -t 0 --benchmark tango
meson test -t 0 --benchmark kodak
meson test -t 0 --benchmark mixed
```