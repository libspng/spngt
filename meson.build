project('png_bench', 'c')

png_dep = dependency('libpng', version : '>=1.6.0')

spng_sp = subproject('spng')
spng_dep = spng_sp.get_variable('spng_dep')

pngbench_deps = [ png_dep, spng_dep ]

png_bench = executable('png_bench', 'png_bench.c', 'lodepng.c',
                       dependencies : pngbench_deps,
                       override_options : [ 'c_std=c11' ])

benchmark_images = subproject('benchmark_images')
images = benchmark_images.get_variable('all_images')

foreach img : images
    benchmark('decode', png_bench, args : img)
endforeach
