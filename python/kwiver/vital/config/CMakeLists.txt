set( THIS_MODULE vital/config )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py "${THIS_MODULE}" __init__ )

################################################################################
# Python Module Library: _config
set( module_headers
  module_config_helpers.h
  )
set( module_sources
  module_config.cxx
  module_config_helpers.cxx
  )

kwiver_add_python_library(
  _config
  ${THIS_MODULE}
  SOURCES ${module_headers}
          ${module_sources}
  PRIVATE pybind11::pybind11
          vital
)
