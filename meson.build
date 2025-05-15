project('plutovg', 'c',
    version: files('version.txt'),
    license: 'MIT',
    meson_version: '>=0.59.0',
    default_options: ['c_std=c99']
)

plutovg_deps = []
plutovg_compile_args = []

cc = meson.get_compiler('c')

plutovg_c_args = ['-DPLUTOVG_BUILD']
plutovg_c_args += cc.get_supported_arguments(
    '-Wno-sign-compare',
    '-Wno-unused-function'
)

math_dep = cc.find_library('m', required: false)
if math_dep.found()
    plutovg_deps += [math_dep]
endif

version_parts = meson.project_version().split('.')
configure_file(
    input: 'include/plutovg.h.in',
    output: 'plutovg.h',
    configuration: {
        'plutovg_VERSION_MAJOR': version_parts[0],
        'plutovg_VERSION_MINOR': version_parts[1],
        'plutovg_VERSION_PATCH': version_parts[2],
    }
)

if get_option('default_library') == 'static'
    plutovg_compile_args += ['-DPLUTOVG_BUILD_STATIC']
    plutovg_libtype = 'STATIC'
    plutovg_libext = 'a'
else
    plutovg_libtype = 'SHARED'
    plutovg_libext = 'so.' + version_parts[0]
endif

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

plutovg_lib = library('plutovg', plutovg_sources,
    include_directories: include_directories('include', 'source'),
    dependencies: plutovg_deps,
    soversion: version_parts[0],
    c_args: plutovg_c_args,
    gnu_symbol_visibility: 'hidden',
    install: true
)

plutovg_dep = declare_dependency(
    link_with: plutovg_lib,
    include_directories: include_directories('include', '.'),
    compile_args: plutovg_compile_args
)

meson.override_dependency('plutovg', plutovg_dep)
install_headers(meson.build_root() / 'plutovg.h', subdir: 'plutovg')

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

cmake = import('cmake')
cmake.write_basic_package_version_file(
    name: 'plutovg',
    version: meson.project_version(),
    compatibility: 'SameMajorVersion'
)
cmake.configure_package_config_file(
    name: 'plutovg',
    input: 'cmake/plutovgConfig.cmake.in',
    configuration: configuration_data()
)

configure_file(
    input: 'cmake/plutovgTargets.cmake.in',
    output: 'plutovgTargets.cmake',
    configuration: {
        'INCDIR': get_option('includedir'),
        'LIBDIR': get_option('libdir'),
        'LIBEXT': plutovg_libext,
        'LIBTYPE': plutovg_libtype,
    }
)
install_data(meson.build_root() / 'plutovgTargets.cmake',
    install_dir: get_option('libdir') / 'cmake' / 'plutovg'
)
