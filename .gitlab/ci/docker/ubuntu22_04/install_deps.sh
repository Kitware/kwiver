#!/bin/sh

set -e

export DEBIAN_FRONTEND=noninteractive
apt-get update

# Install system dependencies
apt-get install -y --no-install-recommends \
    ca-certificates curl zlib1g-dev libcurl4-openssl-dev libssl-dev

# Install Qt system dependencies. Derived from https://wiki.qt.io/Building_Qt_5_from_Git
apt-get install -y libpng-dev libglx-dev freeglut3-dev libfontconfig1-dev \
    '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev \
    libxcursor-dev libicu-dev

# Install Git requirements.
apt-get install -y --no-install-recommends \
    git

# Development tools
apt-get install -y --no-install-recommends \
    make gcc g++ pkg-config nano

# Python dependencies
apt-get install -y --no-install-recommends \
    python3 libpython3-dev python3-distutils python3-pip \
    python3-venv python3-numpy python-is-python3
# Qt 5.12.8 Qtqml requires a "python", provided by python-is-python3

# Install more recent version of cmake
pip install cmake==3.27.9

# metis for colmap
apt-get install -y --no-install-recommends \
    libmetis-dev

# setuptools pinned to 58.0.0 to fix GDAL build error
pip install setuptools==58.0.0

# Remove unnecessary files
apt-get clean
