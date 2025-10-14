# Common setup for Windows wheel builds and tests
# Sets up Python environment for wheel operations
# Environment variables expected:
#   PYTHON_VERSION: Python version (e.g., "38", "39", "310")
#   WHEEL_TYPE: Wheel type ("burnout" or "full")

$pwdpath = $pwd.Path

$major = $env:PYTHON_VERSION.Substring(0,1)
$minor = $env:PYTHON_VERSION.Substring(1)
$env:PYTHON_VERSION_DOTTED = "$major.$minor"
Write-Host "Python version dotted: $env:PYTHON_VERSION_DOTTED"

if ($env:WHEEL_TYPE -eq "burnout") {
    $env:CMAKE_CONFIGURATION = "wheel_windows${env:PYTHON_VERSION}_x86_64_vs2022_ninja_ffmpeg_opencv_python"
} else {
    $env:CMAKE_CONFIGURATION = "wheel_windows${env:PYTHON_VERSION}_x86_64_vs2022_ninja_ceres_ffmpeg_gdal_opencv_pdal_proj_python_qt_vtk_vxl"
}
Write-Host "CMAKE_CONFIGURATION set to: $env:CMAKE_CONFIGURATION"

powershell -File ".gitlab/ci/cmake.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\cmake\bin;$env:PATH"

cmake -P .gitlab/ci/download_wheel_python.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python;$env:PATH"
Set-Item -Force -Path "env:PYTHONHOME" -Value "$pwdpath\.gitlab\python"
