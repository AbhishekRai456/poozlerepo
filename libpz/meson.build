inc_dir = include_directories('include')

subdir('util')
subdir('regex')
subdir('exact')

libpz_sources = [
  'exact/Alphabet.cpp',
  'exact/FMIndex.cpp',
  'regex/NfaBuilder.cpp',
  'regex/NfaMatcher.cpp',
  'regex/RegexPostfix.cpp',
  'regex/RegexTokenizer.cpp'
]

# static_library takes sources and includes, no dependencies here
libpz_core = static_library(
  'pz_core',
  libpz_sources,
  include_directories : inc_dir
)

# The dependency object links the core and util together for consumers
libpz_dep = declare_dependency(
  include_directories : inc_dir,
  link_with : [libpz_core],
  dependencies : [libpz_util_dep]
)

install_headers(
  files([
    'include/pz_analysis.hpp',
    'include/pz_buffer.hpp',
    'include/pz_core.hpp',
    'include/pz_cxx_std.hpp',
    'include/pz_error.hpp',
    'include/pz_std.hpp',
    'include/pz_types.hpp',
    'include/pz_util.hpp',
  ]),
  subdir : 'poozle'
)