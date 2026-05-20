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
# Override the wheel toolchain explicitly for symmetry with the Linux wheel job.
# Keep setuptools new enough to include the Windows detached-buffer logging
# fix, but below 74 to stay compatible with the current scikit-build wheel path.
pip install "scikit-build==0.17.6" delvewheel "setuptools>=65.6.3,<74"

Invoke-Expression -Command .gitlab/ci/vcvarsall.ps1

Invoke-Expression -Command .gitlab/ci/buildcache.ps1
Set-Item -Force -Path "env:PATH" -Value "$env:PATH;$pwdpath\.gitlab\buildcache\bin"
buildcache --show-stats

python setup.py bdist_wheel -- -C $env:GIT_CLONE_PATH/.gitlab/ci/configure_wheel.cmake > "$env:GIT_CLONE_PATH/skbuild_output.log"
if ($LASTEXITCODE -ne 0) {
    throw "python setup.py bdist_wheel failed with exit code $LASTEXITCODE"
}

New-Item -ItemType Directory -Force -Path "dist-$env:WHEEL_TYPE"
$wheelPath = Get-ChildItem dist\*.whl | % FullName
delvewheel show --add-path "$env:SCIKIT_BUILD_DIR\bin;$env:CI_PROJECT_DIR\.gitlab\fletch\bin" $wheelPath > "$env:GIT_CLONE_PATH\wheel_output.log"
if ($LASTEXITCODE -ne 0) {
    throw "delvewheel show failed with exit code $LASTEXITCODE"
}
delvewheel repair --add-path "$env:SCIKIT_BUILD_DIR\bin;$env:CI_PROJECT_DIR\.gitlab\fletch\bin" --wheel-dir "dist-$env:WHEEL_TYPE" $wheelPath >> "$env:GIT_CLONE_PATH\wheel_output.log"
if ($LASTEXITCODE -ne 0) {
    throw "delvewheel repair failed with exit code $LASTEXITCODE"
}

buildcache --show-stats
