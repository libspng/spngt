project('wuffs', 'c')

wuffs_inc = include_directories('release/c')
wuffs_src = 'release/c/wuffs-v0.3.c'
#wuffs_lib = library('wuffs', wuffs_src)

wuffs_dep = declare_dependency(
    #sources : wuffs_src
    include_directories : wuffs_inc
)

if meson.version().version_compare('>= 0.54.0')
    meson.override_dependency('wuffs', wuffs_dep)
endif