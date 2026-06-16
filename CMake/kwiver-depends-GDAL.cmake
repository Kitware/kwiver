# Optionally find and configure GDAL dependency

if(DEFINED fletch_ENABLED_GDAL)
  set(_default ${fletch_ENABLED_GDAL})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_GDAL
  "Enable GDAL dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_GDAL )
  find_package( GDAL REQUIRED )
  if( GDAL_FOUND )
    # We need to build the file in a line-by-line fashon because of
    # portability with end of line markers.
    file( WRITE  ${CMAKE_BINARY_DIR}/test_gdal_version.cxx "#include <gdal_version.h>\n" )
    file( APPEND ${CMAKE_BINARY_DIR}/test_gdal_version.cxx "#include <iostream>\n" )
    file( APPEND ${CMAKE_BINARY_DIR}/test_gdal_version.cxx "int main() { std::cout")
    file( APPEND ${CMAKE_BINARY_DIR}/test_gdal_version.cxx " << GDAL_VERSION_MAJOR << \".\"" )
    file( APPEND ${CMAKE_BINARY_DIR}/test_gdal_version.cxx " << GDAL_VERSION_MINOR << \".\"" )
    file( APPEND ${CMAKE_BINARY_DIR}/test_gdal_version.cxx " << GDAL_VERSION_REV; }\n" )

    TRY_RUN( GDAL_TEST_RUN
             GDAL_TEST_COMPILE
             ${CMAKE_BINARY_DIR}
             ${CMAKE_BINARY_DIR}/test_gdal_version.cxx
             COMPILE_DEFINITIONS "-I${GDAL_INCLUDE_DIR}"
             RUN_OUTPUT_VARIABLE GDAL_VERSION )

    if ( NOT GDAL_TEST_COMPILE )
      message( FATAL_ERROR "Failed to build GDAL version test executable." )
    endif()

    if( GDAL_VERSION VERSION_LESS 2.3.0 )
      message( FATAL_ERROR "GDAL ${GDAL_VERSION} found, but version must be 2.3.0 or higher." )
      unset(GDAL_INCLUDE_DIR)
    else()
      file( REMOVE ${CMAKE_BINARY_DIR}/test_gdal_version.cxx )
      message(STATUS "Found GDAL ${GDAL_VERSION}")
      include_directories(SYSTEM ${GDAL_INCLUDE_DIR})
    endif()
  endif()
endif()
