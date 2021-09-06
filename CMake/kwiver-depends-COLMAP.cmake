# Optionally find and configure COLMAP dependency

option( KWIVER_ENABLE_DEP_COLMAP
  "Enable COLMAP dependent code and plugins (Arrows)"
  OFF
  )

if( KWIVER_ENABLE_DEP_COLMAP )
  find_package( COLMAP REQUIRED )
  include_directories( SYSTEM ${COLMAP_INCLUDE_DIRS} )
endif()
