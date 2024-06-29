project('plutovg', 'c',
    version: '0.0.1',
    license: 'MIT',
    meson_version: '>=0.59.0',
    default_options: ['c_std=c99']
)

plutovg_sources = [
    'source/plutovg-paint.c',
    'source/plutovg-matrix.c',
    'source/plutovg-path.c',
    'source/plutovg-surface.c',
    'source/plutovg-canvas.c',
    'source/plutovg-rasterize.c',
    'source/plutovg-blend.c',
    'source/plutovg-ft-raster.c',
    'source/plutovg-ft-stroker.c',
    'source/plutovg-ft-math.c'
]

plutovg_include_dirs = include_directories(
    'include',
    'source',
    'stb'
)

plutovg_public_headers = [
    'include/plutovg.h'
]

plutovg_compile_args = []
if get_option('default_library') == 'static'
    plutovg_compile_args += ['-DPLUTOVG_BUILD_STATIC']
    add_project_arguments('-DPLUTOVG_BUILD_STATIC', language: ['c', 'cpp'])
endif

plutovg_lib = library('plutovg', plutovg_sources,
    include_directories: plutovg_include_dirs,
    version: meson.project_version(),
    c_args: ['-DPLUTOVG_BUILD'],
    cpp_args: ['-DPLUTOVG_BUILD'],
    link_args: ['-lm'],
    gnu_symbol_visibility: 'hidden',
    install: true
)

plutovg_dep = declare_dependency(
    link_with: plutovg_lib,
    include_directories: include_directories('include'),
    compile_args: plutovg_compile_args,
)

meson.override_dependency('plutovg', plutovg_dep)
install_headers(plutovg_public_headers, subdir: 'plutovg')

if not get_option('examples').disabled()
    subdir('examples')
endif

pkgmod = import('pkgconfig')
pkgmod.generate(plutovg_lib,
    name: 'PlutoVG',
    description: 'Tiny 2D vector graphics library',
    filebase: 'plutovg',
    subdirs: 'plutovg'
)
