# Test Python wheel for Windows
# Environment variables expected:
#   PYTHON_VERSION: Python version (e.g., "38", "39", "310")
#   WHEEL_TYPE: Wheel type ("burnout" or "full")

$ErrorActionPreference = "Stop"

Write-Host "Testing Windows wheel - Python version: $env:PYTHON_VERSION, wheel type: $env:WHEEL_TYPE"

# Configure Python version format
$major = $env:PYTHON_VERSION.Substring(0,1)
$minor = $env:PYTHON_VERSION.Substring(1)
$env:PYTHON_VERSION_DOTTED = "$major.$minor"
Write-Host "Python version dotted: $env:PYTHON_VERSION_DOTTED"

$pwdpath = $pwd.Path

# Set up CMake (needed for downloading Python)
powershell -File ".gitlab/ci/cmake.ps1"
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\cmake\bin;$env:PATH"

# Download and set up Python for wheels
cmake -P .gitlab/ci/download_wheel_python.cmake
Set-Item -Force -Path "env:PATH" -Value "$pwdpath\.gitlab\python;$env:PATH"
Set-Item -Force -Path "env:PYTHONHOME" -Value "$pwdpath\.gitlab\python"

# Set up Python test virtual environment
python -m venv $pwdpath\build\ci-test-venv
. .\build\ci-test-venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install "pytest<=8.1"

# Install the built wheel
$wheelPath = Get-ChildItem dist\*-cp$env:PYTHON_VERSION-cp$env:PYTHON_VERSION-*.whl | % FullName
pip install $wheelPath

# Run tests based on wheel type
if ($env:WHEEL_TYPE -eq "burnout") {
    Write-Host "Running burnout smoke test..."
    pytest "./python/kwiver/vital/tests/burnout_smoke_test.py"
} else {
    Write-Host "Running full test suite..."
    pytest
}
