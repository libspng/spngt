project('spngt', 'c',
    version : '1.0',
    meson_version : '>= 0.55.0',
    default_options : [
        'c_std=c99',
        'buildtype=release'
    ]
)

if get_option('benchmarks') == false
    subdir_done()
endif

spngt_deps = [
    dependency('lua'),
    dependency('spng'),
    dependency('libpng'),
    dependency('stb'),
    dependency('lodepng'),
    dependency('wuffs'),
]

spngt_src = files(
    'spngt.c',
    'spngt_lua.c',
    'spngt_libpng.c',
    'spngt_spng.c',
    'spngt_lodepng.c',
    'spngt_stb.c',
    'spngt_wuffs.c',
)

spngt_exe = executable('spngt', spngt_src, dependencies : spngt_deps)

benchmark('info', spngt_exe, args : 'info')

images = subproject('benchmark_images').get_variable('all_images')

foreach img : images
    benchmark('image_decode', spngt_exe, suite : 'decode', args : img)
endforeach

foreach img : images
    benchmark('image_encode', spngt_exe, suite : 'encode',  args : [ img, 'enc' ])
endforeach