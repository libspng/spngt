# Overview

png_bench is a benchmarking utility, it tests the PNG decoding performance of [libspng](https://libspng.org), [libpng](http://www.libpng.org/pub/png/libpng.html), [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) and [lodepng](https://github.com/lvandeve/lodepng).

Benchmark results are hosted at [libspng.org/comparison.html#peformance](https://libspng.org/benchmarks)

# Dependencies

* [meson](https://mesonbuild.com)
* zlib
* lodepng and stb_image is included in the project

The following are automatically downloaded as [meson subprojects](https://mesonbuild.com/Wrap-dependency-system-manual.html):
* [benchmark images](https://gitlab.com/randy408/benchmark_images/) 
* libspng
* libpng (depending on configuration)

Switch between the system-provided libpng or download it as a subproject with `meson -Dlibpng_variant=download/system`.

# Running the benchmark

```
meson --buildtype=release build
cd build
ninja
ninja benchmark
cat meson-logs/benchmarklog.txt
```

# Run with Profile-guided optimization (PGO)

```
meson build
cd build
meson configure --buildtype=release -Db_pgo=generate
ninja benchmark
meson configure -Db_pgo=use
ninja benchmark
cat meson-logs/benchmarklog.txt
```