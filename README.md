# SPNGT

SPNGT is a benchmarking and testing utility for [libspng](https://libspng.org),
[libpng](http://www.libpng.org/pub/png/libpng.html),
[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) and
[lodepng](https://github.com/lvandeve/lodepng).

This is a standalone project, it is also used as a subproject in libspng for benchmarking
and to host non-libspng code.

## Dependencies

* [Meson](https://mesonbuild.com) 0.54.0 or later
* Git LFS for downloading the [benchmark images](https://github.com/libspng/benchmark_images/)
* lodepng and stb_image are included

Meson will fall back to using [wraps](https://mesonbuild.com/Wrap-dependency-system-manual.html)
if the following are not found on the system:
* libspng
* libpng
* zlib

## Creating a build

```bash
# Add --default-library=static on Windows
meson build 
```

## Running the benchmark

```bash
cd build
ninja benchmark
cat meson-logs/benchmarklog.txt
```

## Compile with Profile-guided optimization (PGO)

```bash
meson configure -Db_pgo=generate
ninja benchmark
meson configure -Db_pgo=use
ninja benchmark
cat meson-logs/benchmarklog.txt
```

## Cross-build for Android with NDK

Edit the path for the binaries in `cross_arm.txt`, these must be absolute paths.

Specify the the cross file when creating the cross build:

```bash
meson --cross-file=cross_arm.txt arm_build
```
