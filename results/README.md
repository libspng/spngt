# Benchmark results

This directory contains various benchmark results

* [rpi400-enc.txt](rpi400-enc.txt) - Encode tests on ARM64 (RPi 400)

* [x86-enc.txt](x86-enc.txt) - Encode tests on x86

* [rpi400.txt](../rpi400.txt) - Decode benchmarks with and without ARM NEON optimizations

## Encode experiments

* [kodak.csv](kodak.csv), [tango.csv](tango.csv), [mixed.csv](mixed.csv) - Raw data from encode benchmarks with various encode settings, part of an experiment to find faster-than-default encode settings

* [kodak_params.csv](kodak_params.csv), [tango_params.csv](tango_params.csv), [mixed_params.csv](mixed_params.csv) - Statistics derived from the benchmarks above