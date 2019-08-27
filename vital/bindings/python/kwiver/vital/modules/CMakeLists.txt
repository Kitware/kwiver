project(vital_python_modules)

set(modules_python_srcs
  registration.cxx)

include_directories(${pybind11_INCLUDE_DIR})
if(UNIX)
  add_definitions( -DVITAL_LOAD_PYLIB_SYM )
endif()
if (KWIVER_ENABLE_TOOLS)
  kwiver_add_plugin( modules_python
    SUBDIR          ${kwiver_plugin_module_subdir}
    SOURCES         ${modules_python_srcs}
    PRIVATE         vital_python_util
                    kwiversys
                    vital_logger
                    ${${pybind11_library}}
                    ${PYTHON_LIBRARIES}
                    )
endif()
if (${SKBUILD})
  find_package(PythonExtensions REQUIRED)
  skbuild_add_library( modules
                       vital/modules
                       modules.cxx )
  target_link_libraries( python-vital.modules-modules
              PRIVATE   vital_python_util
                        vital_vpm)
  set_target_properties( python-vital.modules-modules
                         PROPERTIES
                          INSTALL_RPATH "${PYTHON_SITE_PACKAGES_DIR}/kwiver/lib" )

else()
  set(python_noarch FALSE)
  kwiver_create_python_init(vital/modules)
  set(python_noarch TRUE)
  kwiver_create_python_init(vital/modules)
  set(python_noarch FALSE)
  kwiver_add_python_module("${CMAKE_CURRENT_SOURCE_DIR}/module_loader.py"
    vital/modules
    module_loader)

  kwiver_add_python_module("${CMAKE_CURRENT_SOURCE_DIR}/loaders.py"
    vital/modules
    loaders)

  kwiver_add_python_library(modules
                            vital/modules
                            modules.cxx)

  target_link_libraries(python-vital.modules-modules
      PRIVATE   vital_python_util
                vital_vpm)
endif()

