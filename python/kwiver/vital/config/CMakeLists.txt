set( vital_config_python_headers
     module_config_helpers.h
    )

set( vital_config_python_sources
     module_config.cxx
     module_config_helpers.cxx
   )

kwiver_add_python_library( _config
      vital/config
      SOURCES
      ${vital_config_python_headers}
      ${vital_config_python_sources}
      PRIVATE pybind11::pybind11
              vital_config
              vital
    )

kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
                          vital/config
                          __init__
)
