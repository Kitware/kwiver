project(sprokit_python_test)

sprokit_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/test.py
  sprokit/test
  test)

sprokit_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
                          sprokit/test __init__)

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(python)
endif ()
