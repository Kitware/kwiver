# Optionally find and configure PDAL dependency

if(DEFINED fletch_ENABLED_PDAL)
  set(_default ${fletch_ENABLED_PDAL})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_PDAL
  "Enable PDAL dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_PDAL )
  find_package( PDAL 1.0.0 REQUIRED )

  # PDAL library names are improperly exported in PDAL 1.7.2 for Linux
  if (${PDAL_VERSION} VERSION_EQUAL "1.7.2" AND NOT (WIN32 OR APPLE))
    set(PDAL_LIBRARIES "pdal_base")
  endif()

endif()
