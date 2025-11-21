# c++ CI tests

$ErrorActionPreference = "Stop"

. .gitlab/ci/setup_build_env_windows.ps1
$pwdpath = $pwd.Path

cmake -P .gitlab/ci/download_python.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python;$env:PATH"
Set-Item -Force -Path "env:PYTHONHOME" -Value "$pwdpath\.gitlab\python"

. .\build\ci-venv\Scripts\Activate.ps1

Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

. build/setup_KWIVER.ps1
Set-Item -Force -Path "env:KWIVER_DEFAULT_LOG_LEVEL" -Value "DEBUG"

ctest --output-on-failure -V -S .gitlab/ci/ctest_test.cmake
if ($LASTEXITCODE -ne 0) {
    throw "ctest failed with exit code $LASTEXITCODE"
}
