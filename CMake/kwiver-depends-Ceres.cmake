# Optionally find and configure Ceres dependency

option( KWIVER_ENABLE_CERES
  "Enable Ceres dependent code and plugins (Arrows)"
  ${fletch_ENABLED_Ceres}
  )

if( KWIVER_ENABLE_CERES )
  find_package( Ceres REQUIRED )
  if(NOT CERES_INCLUDE_DIRS)
    # Ceres v2 doesn't define CERES_INCLUDE_DIRS, but the ceres algo plugin still relies on it
    get_target_property(CERES_INCLUDE_DIRS Ceres::ceres INTERFACE_INCLUDE_DIRECTORIES)
  endif()
  include_directories( SYSTEM ${CERES_INCLUDE_DIRS} )
endif()
