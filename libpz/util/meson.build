util_sources = [
  'pz_analysis.cpp',
  'pz_buffer.cpp',
  'pz_core.cpp'
]

libpz_util = static_library(
  'pz_util',
  util_sources,
  include_directories : inc_dir
)

libpz_util_dep = declare_dependency(
  include_directories : inc_dir,
  link_with : [libpz_util]
)