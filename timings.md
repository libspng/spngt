# Timings

## ARM64

### RPi 400 / Cortex-A72 1.8 GHz


```
Indexed-color
- NEON disabled    34624 us
- NEON enabled     28248 us
Speedup            19%

Truecolor
- NEON disabled    77031 us
- NEON enabled     55812 us
Speedup            28%

Truecolor-alpha
- NEON disabled    111641 us
- NEON enabled     75763 us
Speedup            32%
```

Palette expansion to RGBA8 seems to be a lot slower with libpng,
both with the Meson build port and the Debian ARM64 package,
spng uses a refactored version of the same SIMD optimizations.

[Full results](rpi400.txt).