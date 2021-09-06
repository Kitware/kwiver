# Optionally find and configure PROJ dependency

option( KWIVER_ENABLE_DEP_PROJ
  "Enable PROJ dependent code and plugins (Arrows)"
  ${fletch_ENABLED_PROJ}
  )

if( KWIVER_ENABLE_DEP_PROJ )
  find_package( PROJ REQUIRED )
endif( KWIVER_ENABLE_DEP_PROJ )
