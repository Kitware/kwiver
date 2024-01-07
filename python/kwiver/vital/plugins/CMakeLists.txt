set( THIS_MODULE vital/plugins )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py "${THIS_MODULE}" __init__ )

################################################################################
# Python Module Library: _pluggable
kwiver_add_python_library(
  _pluggable
  "${THIS_MODULE}"
  SOURCES module_pluggable.cxx
  PRIVATE pybind11::pybind11
          vital
)
################################################################################
# Plugin library
kwiver_add_plugin(
  plugins_from_python
  SOURCES   register.cxx
  PRIVATE   ${PYTHON_LIBRARIES}  # makes use of python library functions
            pybind11::pybind11
            vital
  SUBDIR    ${kwiver_plugin_module_subdir}
)
