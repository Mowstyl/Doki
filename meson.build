project(
  'Doki',
  'c',
  version: run_command('src/_build_utils/version.py', check: true).stdout().strip(),
  license: 'GPL-3.0-or-later',
  meson_version: '>=1.2.99',
  default_options: [
    'buildtype=debugoptimized',
    'b_ndebug=if-release',
    'c_std=c11',
    'pkgconfig.relocatable=true',
  ]
)

compiler = meson.get_compiler('c')
if compiler.get_id() != 'msvc'
  add_project_arguments('-Wall', language : 'c')
  add_project_arguments('-Wextra', language : 'c')
  # add_project_arguments('-Werror', language : 'c')
  add_project_arguments('-pedantic', language : 'c')
else
  add_project_arguments('-Wall', language : 'c')
  # add_project_arguments('-WX', language : 'c')
endif

if host_machine.system() != 'darwin'
  omp = dependency('openmp')
else
  libomp = meson.get_compiler('c').find_library('libomp')
  omp = declare_dependency(
    compile_args:['-Xpreprocessor', '-fopenmp'],
    dependencies : [libomp]
  )
endif


py = import('python').find_installation(pure: false)

# NumPy include directory - needed in all submodules
incdir_numpy = run_command(py,
  [
    '-c',
    'import os; os.chdir(".."); import numpy; print(numpy.get_include())'
  ],
  check: true
).stdout().strip()

inc_np = include_directories(incdir_numpy)

subdir('src')

py.extension_module(
    'doki',
    sources,
    headers,
	include_directories: inc_np,
	dependencies: omp,
    install: true,
)
