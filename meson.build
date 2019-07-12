project('spngt', 'c', default_options : 'c_std=c11')

spngt_deps = []

spng_sp = subproject('spng')
spngt_deps += spng_sp.get_variable('spng_dep')

if get_option('libpng_variant') == 'download'
    png_sp = subproject('libpng')
    spngt_deps += png_sp.get_variable('png_dep')
else
    spngt_deps += dependency('libpng', version : '>=1.6.0', fallback : [ 'libpng', 'png_dep' ])
endif

benchmark_images = subproject('benchmark_images')
images = benchmark_images.get_variable('all_images')

spngt = executable('spngt','spngt.c', 'lodepng.c', dependencies : spngt_deps)

foreach img : images
    benchmark('decode', spngt, args : img)
endforeach
