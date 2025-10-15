#!/bin/bash
set -e

# Test Python wheel for Linux
# Environment variables expected:
#   PYTHON_VERSION: Python version (e.g., "38", "39", "310")
#   WHEEL_TYPE: Wheel type ("burnout" or "full")
#   PYTHON_PREFIX: Python installation prefix
#   KWIVER_PYTHON_PLUGIN_PATH: Plugin path for KWIVER

echo "Testing Linux wheel - Python version: $PYTHON_VERSION, wheel type: $WHEEL_TYPE"

"$PYTHON_PREFIX/bin/python" -m venv build/ci-test-venv
. build/ci-test-venv/bin/activate
pip install -U pip
pip install "pytest<=8.1"

pip install dist-${WHEEL_TYPE}/*-cp${PYTHON_VERSION}-cp${PYTHON_VERSION}-*.whl

if [[ "$WHEEL_TYPE" == "burnout" ]]; then
    echo "Running burnout smoke test..."
    pytest "./python/kwiver/vital/tests/burnout_smoke_test.py"
else
    echo "Running full test suite..."
    pytest
fi
