project(sprokit_test_python)


# Include the python helper functions previously defined here
include("${sprokit_source_dir}/conf/sprokit-macro-python-tests.cmake")


add_subdirectory(test)

add_subdirectory(sprokit)
add_subdirectory(modules)


# Add __init__ file for sprokit.tests (TODO: the python tests should be in the source tree)
set(python_botharch TRUE)
sprokit_create_python_init(sprokit/tests)
