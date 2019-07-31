# SPNGT

SPNGT is a test suite and benchmarking utility for [libspng](https://libspng.org),
[libpng](http://www.libpng.org/pub/png/libpng.html),
[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) and
[lodepng](https://github.com/lvandeve/lodepng).

Benchmark results are at available at https://libspng.org/comparison.

## Dependencies

* Git LFS  for cloning benchmark images
* [meson](https://mesonbuild.com)
* zlib
* lodepng and stb_image are included in the project

The following are automatically downloaded as [meson subprojects](https://mesonbuild.com/Wrap-dependency-system-manual.html):
* [benchmark images](https://gitlab.com/randy408/benchmark_images/)
* libspng
* libpng (depending on configuration)

Switch between building libpng from source and the system-provided libpng with `meson -Dlibpng_variant=download/system`.

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
