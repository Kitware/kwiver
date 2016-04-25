# Optionally find and configure Caffe dependency

option( KWIVER_ENABLE_CAFFE
  "Enable Caffe dependent code and plugins"
  OFF
  )

if( KWIVER_ENABLE_CAFFE )
  find_package( Caffe REQUIRED )
  include_directories(SYSTEM ${Caffe_INCLUDE_DIRS})
endif()
