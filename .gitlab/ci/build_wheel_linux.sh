#!/bin/bash
# Build Python wheel for Linux
# Environment variables expected:
#   PYTHON_VERSION: Python version (e.g., "38", "39", "310")
#   WHEEL_TYPE: Wheel type ("burnout" or "full")
#   PYTHON_PREFIX: Python installation prefix
#   GIT_CLONE_PATH: Git clone path

set -e

echo "Building Linux wheel - Python version: $PYTHON_VERSION, wheel type: $WHEEL_TYPE"

if [[ "$WHEEL_TYPE" == "burnout" ]]; then
    export CMAKE_CONFIGURATION="wheel_linux${PYTHON_VERSION}_x86_64_ffmpeg_opencv_python"
else
    export CMAKE_CONFIGURATION="wheel_linux${PYTHON_VERSION}_x86_64_ceres_dbow2_ffmpeg_gdal_kpf_opencv_pdal_proj_python_uuid_vtk_vxl"
fi

echo "CMAKE_CONFIGURATION: $CMAKE_CONFIGURATION"

. .gitlab/ci/setup_build_env_linux.sh

"$PYTHON_PREFIX/bin/python" --version
"$PYTHON_PREFIX/bin/python" -m venv build/ci-venv
. build/ci-venv/bin/activate
pip install -U pip
pip install -r ./python/requirements_dev.txt
pip install scikit-build auditwheel

.gitlab/ci/sccache.sh
sccache --start-server
sccache --show-stats

python setup.py bdist_wheel -- -C "$GIT_CLONE_PATH/.gitlab/ci/configure_wheel.cmake" > "$GIT_CLONE_PATH/skbuild_output.log"

mkdir -p "dist-${WHEEL_TYPE}"
LD_LIBRARY_PATH=/opt/fletch/lib/ auditwheel show dist/*.whl > "$GIT_CLONE_PATH/wheel_output.log"
LD_LIBRARY_PATH=/opt/fletch/lib/ auditwheel repair --wheel-dir "dist-${WHEEL_TYPE}" dist/*.whl >> "$GIT_CLONE_PATH/wheel_output.log"

sccache --show-stats
