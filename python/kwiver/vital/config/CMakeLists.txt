kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py "vital/config" __init__ )

set( module_headers
  config_module_helpers.h
  )
set( module_sources
  config_module.cxx
  config_module_helpers.cxx
  )

kwiver_add_python_library(
  _config
  vital/config
  SOURCES ${module_headers}
          ${module_sources}
  PRIVATE pybind11::pybind11
          vital
)
