# Optional confgure DBoW2 dependency

option( KWIVER_ENABLE_DBOW2
  "Enable DBoW2 dependent code and plugins"
  OFF
  )

if( KWIVER_ENABLE_DBOW2 )
  find_package( OpenCV REQUIRED )
endif( KWIVER_ENABLE_DBOW2 )
