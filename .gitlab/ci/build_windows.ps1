# Regular Windows build script
# Used for standard CI builds (not wheels)

# Set up build environment (CMake, Ninja, Qt, Fletch)
. .gitlab/ci/setup_build_env_windows.ps1
$pwdpath = $pwd.Path

# Set up Python
cmake -P .gitlab/ci/download_python.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python;$env:PATH"
Set-Item -Force -Path "env:PYTHONHOME" -Value "$pwdpath\.gitlab\python"

# Set up Python virtual environment
python -m venv $pwdpath\build\ci-venv
. .\build\ci-venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -qq -r $pwdpath\.gitlab\ci\requirements_dev-windows.txt

# Set up Visual Studio environment
Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

# Set up and start buildcache
Invoke-Expression -Command .gitlab/ci/buildcache.ps1
Set-Item -Force -Path "env:PATH" -Value "$env:PATH;$pwdpath\.gitlab\buildcache\bin"
buildcache --show-stats

# Configure and build
ctest -VV -S .gitlab/ci/ctest_configure.cmake
ctest -VV -S .gitlab/ci/ctest_build.cmake

# Show buildcache stats
buildcache --show-stats
