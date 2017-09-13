project(sprokit_test_python_test)


sprokit_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/test-test.py
  sprokit/tests
  test-test)
sprokit_discover_python_tests(test test-test.py)
