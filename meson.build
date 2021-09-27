project('spngt', 'c',
    version : '1.0',
    meson_version : '>= 0.55.0',
    default_options : [
        'c_std=c99',
        'buildtype=release'
    ]
)

cc = meson.get_compiler('c')

spngt_deps = [
    dependency('spng', fallback : ['spng', 'spng_dep']),
    dependency('libpng', fallback : [ 'libpng', 'png_dep']),
    cc.find_library('m', required : false), # stb_image
    dependency('wuffs'),
]

if get_option('benchmarks') == false
    subdir_done()
endif

spngt_exe = executable('spngt', 'spngt.c', 'lodepng.c', dependencies : spngt_deps)

benchmark('info', spngt_exe, args : 'info')

images = subproject('benchmark_images').get_variable('all_images')

foreach img : images
    benchmark('image_decode', spngt_exe, suite : 'decode', args : img)
endforeach
