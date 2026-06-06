bench_sources = ['bench_main.cpp']
bench_deps = [libpz_dep]
bench_args = ['-O2', '-DNDEBUG'] # Critical for L3 cache performance

if get_option('with_baselines')
  # Links against system installations of legacy engines if flag is true
  bench_deps += [dependency('re2'), dependency('libpcre2-8')]
  bench_args += ['-DPOOZLE_WITH_BASELINES']
endif

executable(
  'poozle_bench',
  bench_sources,
  include_directories : [
    include_directories('../libpz/include'),
    include_directories('../libpz/exact'),
    include_directories('../libpz/regex'),
    include_directories('../libpz/util') # Added to prevent linker errors
  ],
  dependencies : bench_deps,
  cpp_args : bench_args,
  install : false
)