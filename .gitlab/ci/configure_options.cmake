# Options that can be overridden based on the
# configuration name.
function (configuration_flag variable configuration)
  if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "${configuration}")
    set("${variable}" ON CACHE BOOL "")
  else ()
    set("${variable}" OFF CACHE BOOL "")
  endif ()
endfunction ()

# Shared/static
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "shared")
  set(KWIVER_BUILD_SHARED ON CACHE BOOL "")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "static")
  set(KWIVER_BUILD_SHARED OFF CACHE BOOL "")
endif ()

# Examples
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "opencv" AND
    "$ENV{CMAKE_CONFIGURATION}" MATCHES "vxl")
  set(KWIVER_ENABLE_EXAMPLES ON CACHE BOOL "")
else ()
  set(KWIVER_ENABLE_EXAMPLES OFF CACHE BOOL "")
endif ()

# Arrows.
configuration_flag(KWIVER_ENABLE_CERES "ceres")
configuration_flag(KWIVER_ENABLE_CUDA "cuda")
configuration_flag(KWIVER_ENABLE_DBOW2 "dbow2")
configuration_flag(KWIVER_ENABLE_FFMPEG "ffmpeg")
# disable CUDA with ffmpeg, until test failures are resolved
set(KWIVER_ENABLE_FFMPEG_CUDA OFF CACHE BOOL "")
configuration_flag(KWIVER_ENABLE_GDAL "gdal")
configuration_flag(KWIVER_ENABLE_KPF "kpf")
configuration_flag(KWIVER_ENABLE_OPENCV "opencv")
configuration_flag(KWIVER_ENABLE_PDAL "pdal")
configuration_flag(KWIVER_ENABLE_PROJ "proj")
configuration_flag(KWIVER_ENABLE_QT "qt")
configuration_flag(KWIVER_ENABLE_UUID "uuid")
configuration_flag(KWIVER_ENABLE_VXL "vxl")
# super3d goes along with vxl
configuration_flag(KWIVER_ENABLE_SUPER3D "vxl")
configuration_flag(KWIVER_ENABLE_VTK "vtk")

# Python settings.
configuration_flag(KWIVER_ENABLE_PYTHON "python")
configuration_flag(KWIVER_ENABLE_PYTHON_TESTS "python")
