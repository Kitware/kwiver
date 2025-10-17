# Common setup for Windows builds
# Sets up CMake, Ninja, Qt, and Fletch

$pwdpath = $pwd.Path

powershell -File ".gitlab/ci/cmake.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\cmake\bin;$env:PATH"

powershell -File ".gitlab/ci/ninja.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab;$env:PATH"

cmake --version
ninja --version

cmake -P .gitlab/ci/download_qt.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\qt\bin;$env:PATH"

cmake -P .gitlab/ci/download_fletch.cmake
Set-Item -Force -Path "env:PATH" -Value "$env:PATH;$pwdpath\.gitlab\fletch\bin;$pwdpath\.gitlab\fletch\x64\vc17\bin"
