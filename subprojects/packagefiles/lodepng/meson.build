project('lodepng', 'c')

lodepng_inc = include_directories('.')

# Avoid C++ name mangling
lodepng_src = configure_file(
    input : 'lodepng.cpp',
    output : 'lodepng.c',
    copy : true
)
 
lodepng_lib = library('lodepng', lodepng_src)

lodepng_dep = declare_dependency(
    link_with : lodepng_lib,
    include_directories : lodepng_inc
)

meson.override_dependency('lodepng', lodepng_dep)