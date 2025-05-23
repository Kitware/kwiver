# Optional find and configure ZLib dependency

option( KWIVER_ENABLE_ZLIB
  "Enable zlib dependent code and plugins (Arrows)"
  ${fletch_ENABLED_ZLib}
  )

if( KWIVER_ENABLE_ZLIB )
  find_package( ZLIB MODULE REQUIRED )
endif()
