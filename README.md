# Overview

png_bench is a benchmarking utility, it tests the PNG decoding performance of [libspng](https://libspng.org), [libpng](http://www.libpng.org/pub/png/libpng.html), [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) and [lodepng](https://github.com/lvandeve/lodepng).

Benchmark results can be viewed at [libspng.org/comparison.html#peformance](https://libspng.org/benchmarks)

# Dependencies

* [meson](https://mesonbuild.com)
* zlib
* libpng
* lodepng and stb_image is included in the project

libspng and the [benchmark images](https://gitlab.com/randy408/benchmark_images/) are automatically downloaded as [meson subprojects](https://mesonbuild.com/Wrap-dependency-system-manual.html).

# Running the benchmark

```
meson --buildtype=release build
cd build
ninja
ninja benchmark
cat meson-logs/benchmarklog.txt
```