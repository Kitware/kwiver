# Build Python wheel for Windows
# Environment variables expected:
#   PYTHON_VERSION: Python version (e.g., "38", "39", "310")
#   WHEEL_TYPE: Wheel type ("burnout" or "full")
#   GIT_CLONE_PATH: Git clone path

Write-Host "Building Windows wheel - Python version: $env:PYTHON_VERSION, wheel type: $env:WHEEL_TYPE"

# Configure Python version format
$major = $env:PYTHON_VERSION.Substring(0,1)
$minor = $env:PYTHON_VERSION.Substring(1)
$env:PYTHON_VERSION_DOTTED = "$major.$minor"
Write-Host "Python version dotted: $env:PYTHON_VERSION_DOTTED"

# Configure CMAKE_CONFIGURATION based on wheel type
if ($env:WHEEL_TYPE -eq "burnout") {
    $env:CMAKE_CONFIGURATION = "wheel_windows${env:PYTHON_VERSION}_x86_64_vs2022_ninja_ffmpeg_opencv_python"
} else {
    $env:CMAKE_CONFIGURATION = "wheel_windows${env:PYTHON_VERSION}_x86_64_vs2022_ninja_ceres_ffmpeg_gdal_opencv_pdal_proj_python_qt_vtk_vxl"
}
Write-Host "CMAKE_CONFIGURATION set to: $env:CMAKE_CONFIGURATION"

# Set scikit-build directory
$env:SCIKIT_BUILD_DIR = "${env:GIT_CLONE_PATH}\_skbuild\win-amd64-$env:PYTHON_VERSION_DOTTED\cmake-build"
Write-Host "SCIKIT_BUILD_DIR set to: $env:SCIKIT_BUILD_DIR"

# Set up build environment (CMake, Ninja, Qt, Fletch)
. .gitlab/ci/setup_build_env_windows.ps1

$pwdpath = $pwd.Path

# Download and set up Python for wheels
cmake -P .gitlab/ci/download_wheel_python.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python;$env:PATH"
Set-Item -Force -Path "env:PYTHONHOME" -Value "$pwdpath\.gitlab\python"

# Set up Python virtual environment
python -m venv $pwdpath\build\ci-venv
. .\build\ci-venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -qq -r $pwdpath\.gitlab\ci\requirements_dev-windows.txt
pip install scikit-build delvewheel

# Set up Visual Studio environment
Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

# Set up build cache
Invoke-Expression -Command .gitlab/ci/buildcache.ps1
Set-Item -Force -Path "env:PATH" -Value "$env:PATH;$pwdpath\.gitlab\buildcache\bin"
buildcache --show-stats

# Build the wheel
python setup.py bdist_wheel -- -C $env:GIT_CLONE_PATH/.gitlab/ci/configure_wheel.cmake > $env:GIT_CLONE_PATH/skbuild_output.log

# Repair the wheel with delvewheel
$wheelPath = Get-ChildItem dist\*.whl | % FullName
delvewheel show --add-path "$env:SCIKIT_BUILD_DIR\bin;$env:CI_PROJECT_DIR\.gitlab\fletch\bin" $wheelPath | Out-File -FilePath "$env:GIT_CLONE_PATH\wheel_output.log"
delvewheel repair --add-path "$env:SCIKIT_BUILD_DIR\bin;$env:CI_PROJECT_DIR\.gitlab\fletch\bin" $wheelPath | Out-File -Append -FilePath "$env:GIT_CLONE_PATH\wheel_output.log"

# Organize output directories
Rename-Item -Path dist -NewName dist-orig
Rename-Item -Path wheelhouse -NewName dist

# Show final buildcache stats
buildcache --show-stats
