# SPNGT

SPNGT is a benchmarking and testing utility for [libspng](https://libspng.org),
[libpng](http://www.libpng.org/pub/png/libpng.html),
[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) and
[lodepng](https://github.com/lvandeve/lodepng).

This is a standalone project, it is also used as a subproject in libspng for benchmarking
and for hosting non-libspng code.

Benchmark results are at available at https://libspng.org.

## Dependencies

* Git LFS for downloading the benchmark images
* [Meson](https://mesonbuild.com)
* zlib
* lodepng and stb_image are included in the project

The following are automatically downloaded as [Meson subprojects](https://mesonbuild.com/Wrap-dependency-system-manual.html):
* [benchmark images](https://github.com/libspng/benchmark_images/)
* libspng
* libpng (depending on configuration)

Switch between building libpng from source and the host system's libpng with `meson -Dlibpng_variant=download/system`.

## Creating a build

```
meson --buildtype=release build
cd build
```

## Running the benchmark

```
ninja
ninja benchmark
cat meson-logs/benchmarklog.txt
```

## Compile with Profile-guided optimization (PGO)

```
meson build
cd build
meson configure --buildtype=release -Db_pgo=generate
ninja benchmark
meson configure -Db_pgo=use
ninja benchmark
cat meson-logs/benchmarklog.txt
```

## Cross-build for Android / ARM

Compiling for Android requires the NDK.

Edit the path for the binaries in `cross_arm.txt`, these must be absolute paths.

Specify the the cross file when creating the cross build

```
meson --cross-file=cross_arm.txt --buildtype=release arm_build
```
