# Utility function for kwiver used to install pythn bindings when Scikit-build drives the build
# These functions are similar to function in kwiver-utils-python
# The following functions are defined:
#
# skbuild_add_library
#
# Their syntax is:
#
# skbuild_add_library( name modpath [source ...] )
#   Builds and install a library that would be used as a python module. It is
#   built as a shared library and places in modpath. The modpath is assumed to
#   be relative to the package root of the project in python site directory.


###
# Borrowed from kwiver-utils-python
macro (_skbuild_create_safe_modpath    modpath    result)
  string(REPLACE "/" "." "${result}" "${modpath}")
endmacro ()

###
# skbuild_add_library( name modpath [source ...] )
#
# Builds and install a library that would be used as a python module. It is
# built as a shared library and places in modpath. The modpath is assumed to
# be relative to the package root.
#
# Args:
#   name: Name of the library
#   modpath: Path of the library relative to package root
#   source: List of source files
function (skbuild_add_library    name    modpath)
  _skbuild_create_safe_modpath("${modpath}" safe_modpath)
  pybind11_add_module( "python-${safe_modpath}-${name}"
                        ${ARGN} )
  set_target_properties( "python-${safe_modpath}-${name}"
                         PROPERTIES
                          INSTALL_RPATH "${PYTHON_SITE_PACKAGES_DIR}/kwiver/lib" )
  set_target_properties( "python-${safe_modpath}-${name}"
                         PROPERTIES
                          OUTPUT_NAME "${name}"
                          PREFIX      ""
                          SUFFIX      "${PYTHON_EXTENSION_MODULE_SUFFIX}"
                        )
  install( TARGETS "python-${safe_modpath}-${name}"
           DESTINATION ${modpath} )
endfunction()

