project('stb', 'c')

cc = meson.get_compiler('c')

stb_deps = cc.find_library('m', required : false)

stb_inc = include_directories('.')

stb_dep = declare_dependency(
    include_directories : stb_inc,
    dependencies : stb_deps
)

meson.override_dependency('stb', stb_dep)