project(sprokit_python_test)

#set(sprokit_test_data_directory
#  "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(sprokit_test_pipelines_directory
  "${CMAKE_SOURCE_DIR}/sprokit/tests/data/pipelines")

# Write a file that locates the pipeline directory for tests
sprokit_add_python_resource(
  configure_info.py
  sprokit/test
  sprokit_test_pipelines_directory
  CMAKE_SOURCE_DIR
  CMAKE_BINARY_DIR
)


sprokit_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/test.py
  sprokit/test
  test)

sprokit_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
  sprokit/test
  __init__)


if (KWIVER_ENABLE_TESTS)
  add_subdirectory(python)
endif ()
