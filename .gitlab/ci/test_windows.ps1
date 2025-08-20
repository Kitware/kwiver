# Regular Windows test script
# Used for standard CI tests (not wheels)

# Set up build environment (CMake, Ninja, Qt, Fletch)
. .gitlab/ci/setup_build_env_windows.ps1
$pwdpath = $pwd.Path

# Set up Python
cmake -P .gitlab/ci/download_python.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python;$env:PATH"
Set-Item -Force -Path "env:PYTHONHOME" -Value "$pwdpath\.gitlab\python"

# Activate virtual environment
. .\build\ci-venv\Scripts\Activate.ps1

# Set up Visual Studio environment
Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

# Set up KWIVER environment
. build/setup_KWIVER.ps1
Set-Item -Force -Path "env:KWIVER_DEFAULT_LOG_LEVEL" -Value "DEBUG"

# Run tests
ctest --output-on-failure -V -S .gitlab/ci/ctest_test.cmake
