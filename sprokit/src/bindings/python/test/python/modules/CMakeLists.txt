project(sprokit_python_test_python_modules)

kwiver_add_python_module(${CMAKE_CURRENT_SOURCE_DIR}/extras.py
  sprokit/test/python/modules
  extras)

kwiver_create_python_plugin_init(sprokit/test/python/modules)
