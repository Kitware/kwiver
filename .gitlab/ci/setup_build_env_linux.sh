#!/bin/bash
# Common setup for Linux builds
# Sets up CMake, Ninja, and adds them to PATH
set -e

.gitlab/ci/cmake.sh
.gitlab/ci/ninja.sh
export PATH=$PWD/.gitlab:$PWD/.gitlab/cmake/bin:$PATH
