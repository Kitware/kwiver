#!/bin/bash
set -e

# Regular Linux build script
# Used for standard CI builds (not wheels)

# Set up build environment (CMake and Ninja)
# Source to ensure PATH exports affect current shell
. .gitlab/ci/setup_build_env_linux.sh

# Set up Python virtual environment
python3 -m venv build/ci-venv
. build/ci-venv/bin/activate
pip install -r ./python/requirements_dev.txt

# Set up and start sccache
.gitlab/ci/sccache.sh
sccache --start-server
sccache --show-stats

# Configure and build
$LAUNCHER ctest -VV -S .gitlab/ci/ctest_configure.cmake
$LAUNCHER ctest -VV -S .gitlab/ci/ctest_build.cmake &> $GIT_CLONE_PATH/compile_output.log

# Show sccache stats
sccache --show-stats

# Check for warnings
exec .gitlab/ci/check_warnings.sh .
