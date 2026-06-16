# Optional find and configure ZLib dependency

if(DEFINED fletch_ENABLED_ZLib)
  set(_default ${fletch_ENABLED_ZLib})
else()
  set(_default OFF)
endif()
option( KWIVER_ENABLE_ZLIB
  "Enable zlib dependent code and plugins (Arrows)"
  ${_default}
  )
unset(_default)

if( KWIVER_ENABLE_ZLIB )
  find_package( ZLIB MODULE REQUIRED )
endif()
