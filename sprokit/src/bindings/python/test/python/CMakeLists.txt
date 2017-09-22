project(sprokit_python_test_python)

set(no_install TRUE)

add_subdirectory(modules)

sprokit_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
                          sprokit/test/python __init__)
