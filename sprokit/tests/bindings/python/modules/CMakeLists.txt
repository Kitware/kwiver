project(sprokit_test_python_modules)

##############################
# Module tests
##############################

sprokit_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/test-pymodules.py
  sprokit/tests test-pymodules)
sprokit_discover_python_tests(pymodules test-pymodules.py)
