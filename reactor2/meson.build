project('reactor2', 'c',
        version : '1.0.0',
        default_options : ['warning_level=3', 'c_std=c2x'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_MESON_LIBRARY']

e = executable('db_test', 'db_test.c', 'database.c', 'symbol.c')
test('db_test', e)

shared_library('reactor2',
               'main.c',
               'database.c',
               'symbol.c',
               install : true,
               c_args : lib_args
)