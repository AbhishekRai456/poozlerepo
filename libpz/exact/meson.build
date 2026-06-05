inc_dir = include_directories('../include')

libpz_exact_sources = [
  'Alphabet.cpp',
  'FMIndex.cpp',
]

libpz_exact = static_library(
  'pz_exact',
  libpz_exact_sources,
  include_directories : inc_dir,
)

libpz_exact_dep = declare_dependency(
  link_with : libpz_exact,
  include_directories : inc_dir,
)