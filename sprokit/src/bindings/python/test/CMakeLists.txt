project(sprokit_python_test)

#set(sprokit_test_data_directory
#  "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(sprokit_test_pipelines_directory
  "${CMAKE_SOURCE_DIR}/sprokit/tests/data/pipelines")

# Write a file that locates the pipeline directory for tests
sprokit_add_python_resource(
  pipeline_dir.txt
  sprokit/test
  sprokit_test_pipelines_directory
)


sprokit_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/test.py
  sprokit/test
  test)

sprokit_create_python_init(sprokit/test)


if (KWIVER_ENABLE_TESTS)
  add_subdirectory(python)
endif ()
