# Build Python wheel for Windows
# Environment variables expected:
#   PYTHON_VERSION: Python version (e.g., "39", "310")
#   WHEEL_TYPE: Wheel type ("burnout" or "full")
#   GIT_CLONE_PATH: Git clone path

$ErrorActionPreference = "Stop"

Write-Host "Building Windows wheel - Python version: $env:PYTHON_VERSION, wheel type: $env:WHEEL_TYPE"

. .gitlab/ci/setup_wheel_env_windows.ps1

$env:SCIKIT_BUILD_DIR = "${env:GIT_CLONE_PATH}\_skbuild\win-amd64-$env:PYTHON_VERSION_DOTTED\cmake-build"
Write-Host "SCIKIT_BUILD_DIR set to: $env:SCIKIT_BUILD_DIR"

. .gitlab/ci/setup_build_env_windows.ps1

$pwdpath = $pwd.Path

python -m venv $pwdpath\build\ci-venv
. .\build\ci-venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -qq -r $pwdpath\.gitlab\ci\requirements_dev-windows.txt
pip install scikit-build delvewheel

Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

Invoke-Expression -Command .gitlab/ci/buildcache.ps1
Set-Item -Force -Path "env:PATH" -Value "$env:PATH;$pwdpath\.gitlab\buildcache\bin"
buildcache --show-stats

python setup.py bdist_wheel -- -C $env:GIT_CLONE_PATH/.gitlab/ci/configure_wheel.cmake > "$env:GIT_CLONE_PATH/skbuild_output.log"

New-Item -ItemType Directory -Force -Path "dist-$env:WHEEL_TYPE"
$wheelPath = Get-ChildItem dist\*.whl | % FullName
delvewheel show --add-path "$env:SCIKIT_BUILD_DIR\bin;$env:CI_PROJECT_DIR\.gitlab\fletch\bin" $wheelPath > "$env:GIT_CLONE_PATH\wheel_output.log"
delvewheel repair --add-path "$env:SCIKIT_BUILD_DIR\bin;$env:CI_PROJECT_DIR\.gitlab\fletch\bin" --wheel-dir "dist-$env:WHEEL_TYPE" $wheelPath >> "$env:GIT_CLONE_PATH\wheel_output.log"

buildcache --show-stats
