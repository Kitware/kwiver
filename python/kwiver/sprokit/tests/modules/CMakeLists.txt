##############################
# Module tests
##############################
kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/test-pymodules.py sprokit/tests/modules test-pymodules)
include(kwiver-setup-tests-python)
kwiver_add_pytest(test-pymodules python)
# kwiver_discover_python_tests(pymodules test-pymodules.py)
