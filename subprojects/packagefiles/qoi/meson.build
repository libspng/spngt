project('qoi', 'c')

qoi_inc = include_directories('.')

qoi_dep = declare_dependency(include_directories : qoi_inc)

meson.override_dependency('qoi', qoi_dep)