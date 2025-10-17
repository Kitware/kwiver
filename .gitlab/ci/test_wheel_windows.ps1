# Test Python wheel for Windows
# Environment variables expected:
#   PYTHON_VERSION: Python version (e.g., "38", "39", "310")
#   WHEEL_TYPE: Wheel type ("burnout" or "full")

$ErrorActionPreference = "Stop"

Write-Host "Testing Windows wheel - Python version: $env:PYTHON_VERSION, wheel type: $env:WHEEL_TYPE"

. .gitlab/ci/setup_wheel_env_windows.ps1

$pwdpath = $pwd.Path

python -m venv $pwdpath\build\ci-test-venv
. .\build\ci-test-venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install "pytest<=8.1"

$wheelPath = Get-ChildItem dist-$env:WHEEL_TYPE\*-cp$env:PYTHON_VERSION-cp$env:PYTHON_VERSION-*.whl | % FullName
pip install $wheelPath

if ($env:WHEEL_TYPE -eq "burnout") {
    Write-Host "Running burnout smoke test..."
    pytest "./python/kwiver/vital/tests/burnout_smoke_test.py"
} else {
    Write-Host "Running full test suite..."
    pytest
}
