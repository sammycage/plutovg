project('plutovg', 'c',
    version: '0.0.3',
    license: 'MIT',
    meson_version: '>=0.59.0',
    default_options: ['c_std=c99']
)

plutovg_sources = [
    'source/plutovg-blend.c',
    'source/plutovg-canvas.c',
    'source/plutovg-font.c',
    'source/plutovg-matrix.c',
    'source/plutovg-paint.c',
    'source/plutovg-path.c',
    'source/plutovg-rasterize.c',
    'source/plutovg-surface.c',
    'source/plutovg-ft-math.c',
    'source/plutovg-ft-raster.c',
    'source/plutovg-ft-stroker.c'
]

plutovg_compile_args = []
if get_option('default_library') == 'static'
    plutovg_compile_args += ['-DPLUTOVG_BUILD_STATIC']
endif

plutovg_lib = library('plutovg', plutovg_sources,
    include_directories: include_directories('include', 'source'),
    version: meson.project_version(),
    c_args: ['-DPLUTOVG_BUILD'] + plutovg_compile_args,
    cpp_args: ['-DPLUTOVG_BUILD'] + plutovg_compile_args,
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
install_headers('include/plutovg.h', subdir: 'plutovg')

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
