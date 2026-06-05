project('poozle', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++17',
    'warning_level=2',
  ]
)

subdir('libpz')
subdir('cli')

smoke_exe = executable(
  'smoke_test',
  'smoke_test.cpp',
  dependencies : [libpz_dep],
)
test('Smoke Test', smoke_exe)