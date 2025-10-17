#!/bin/bash
set -e

# Regular Linux test script
# Used for standard CI tests (not wheels)

# Set up build environment (CMake and Ninja)
# Source to ensure PATH exports affect current shell
. .gitlab/ci/setup_build_env_linux.sh

# Activate virtual environment
. build/ci-venv/bin/activate

# Set up KWIVER environment
. build/setup_KWIVER.sh

# Run tests
ctest -V --output-on-failure -S .gitlab/ci/ctest_test.cmake
