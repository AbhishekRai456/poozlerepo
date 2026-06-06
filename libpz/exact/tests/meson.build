exe = executable(
    'test_fm_index',
    'test_fm_index.cpp',
    include_directories : [
        include_directories('../../include'),
        include_directories('..')
    ],
    # Use the dependency local to the exact library
    dependencies : [libpz_exact_dep], 
    install : false,
)
test('FM-Index Correctness Tests', exe)