project(sprokit_test_python_test)

kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/test-test.py
                            sprokit/tests/test test-test)

include(kwiver-setup-tests-python)
kwiver_add_pytest(test-test python)
# kwiver_discover_python_tests(test test-test.py)
