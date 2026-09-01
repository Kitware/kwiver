#
# Arrows Algorithms CMake file
#

set(CMAKE_FOLDER "Arrows")
set(ARROWS_SOURCE_DIR           "${CMAKE_CURRENT_SOURCE_DIR}")
set(ARROWS_BINARY_DIR           "${CMAKE_CURRENT_BINARY_DIR}")

# Base plug-in with default implementations
add_subdirectory( core )

# Add geocalc plugin if enabled
if( KWIVER_ENABLE_GEOCALC )
  add_subdirectory( geocalc )
endif()

# Add MVG plugin if enabled
if( KWIVER_ENABLE_MVG )
  add_subdirectory( mvg )
endif()

# Add Ceres plugin if enabled
if( KWIVER_ENABLE_CERES )
  add_subdirectory( ceres )
endif()

# Add OpenCV plugin if enabled
if( KWIVER_ENABLE_OPENCV )
  if( KWIVER_ENABLE_DBOW2 )
    add_subdirectory( dbow2 )
  endif()

  add_subdirectory( ocv )
endif()

# Add PROJ plugin if enabled
if( KWIVER_ENABLE_PROJ )
  add_subdirectory( proj )
endif()

# Add Qt plugin if enabled
if( KWIVER_ENABLE_QT )
  add_subdirectory( qt )
endif()

# Add VXL plugin if enabled
if( KWIVER_ENABLE_VXL )
  add_subdirectory( vxl )

  # Super3D depends on VXL
  option( KWIVER_ENABLE_SUPER3D
    "Enable SUPER3D dependent code and plugins (Arrows)"
    OFF
    )

  # Add super3d plugin if enabled
  if( KWIVER_ENABLE_SUPER3D )
    add_subdirectory( super3d )
  endif()
endif()

if ( KWIVER_ENABLE_CUDA )
  add_subdirectory( cuda )
endif()

# if kpf is enabled
if( KWIVER_ENABLE_KPF )
  add_subdirectory( kpf )
endif()

# if klv is enabled
# if matlab is enabled
if( KWIVER_ENABLE_MATLAB )
  add_subdirectory( matlab )
endif()

if( KWIVER_ENABLE_KLV )
  add_subdirectory( klv )
endif()

# if GDAL is enabled
if ( KWIVER_ENABLE_GDAL )
  add_subdirectory( gdal )
endif()

# if PDAL is enabled
if ( KWIVER_ENABLE_PDAL )
  add_subdirectory( pdal )
endif()

# if uuid is enabled
if( KWIVER_ENABLE_UUID )
  add_subdirectory( uuid )
endif()

# if FFmpeg is enabled
if( KWIVER_ENABLE_FFMPEG)
  add_subdirectory( ffmpeg )
endif()

# if VTK is enabled
if( KWIVER_ENABLE_VTK)
  add_subdirectory( vtk )
endif()

add_subdirectory( serialize )

# If Zlib is enabled
if( KWIVER_ENABLE_ZLIB )
  add_subdirectory( zlib )
endif()
