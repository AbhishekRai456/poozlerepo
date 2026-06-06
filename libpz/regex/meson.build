inc_dir = include_directories('../include')

libpz_regex_sources = [
  'NfaBuilder.cpp',
  'NfaMatcher.cpp',
  'RegexPostfix.cpp',
  'RegexTokenizer.cpp',
]

libpz_regex = static_library(
  'pz_regex',
  libpz_regex_sources,
  include_directories : inc_dir,
)

libpz_regex_dep = declare_dependency(
  link_with : libpz_regex,
  include_directories : inc_dir,
)

# Wire the existing test suites so meson test can run them
test_match_exe = executable(
  'test_match',
  'tests/test_0_match.cpp',
  include_directories : inc_dir,
  dependencies : [libpz_regex_dep],
)
test('NFA Match Tests', test_match_exe)

test_escape_exe = executable(
  'test_escape',
  'tests/test_1_escape.cpp',
  include_directories : inc_dir,
  dependencies : [libpz_regex_dep],
)
test('NFA Escape Tests', test_escape_exe)

test_findall_exe = executable(
  'test_findall',
  'tests/test_2_findall.cpp',
  include_directories : inc_dir,
  dependencies : [libpz_regex_dep],
)
test('NFA FindAll Tests', test_findall_exe)

test_extra_exe = executable(
  'test_extra',
  'tests/test_3_extra.cpp',
  include_directories : inc_dir,
  dependencies : [libpz_regex_dep],
)
test('NFA Extra Edge Case Tests', test_extra_exe)