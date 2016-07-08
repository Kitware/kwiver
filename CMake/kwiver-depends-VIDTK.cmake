# Optional find and confgure VIDTK dependency

option( KWIVER_ENABLE_VIDTK
  "Enable VIDTK dependent code and plugins"
  OFF
  )

if( KWIVER_ENABLE_VIDTK )
  find_package( vidtk REQUIRED )
  include_directories( SYSTEM ${VIDTK_INCLUDE_DIRS} )
  link_directories( ${VIDTK_LIBRARY_DIR} )
endif( KWIVER_ENABLE_VIDTK )
