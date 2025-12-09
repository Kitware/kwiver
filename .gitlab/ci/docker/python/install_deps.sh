#!/bin/sh

set -e

yum install -y --setopt=install_weak_deps=False \
    git-core openssl-devel libcurl-devel

# Development tools
yum install -y --setopt=install_weak_deps=False \
    make ninja-build

# Alias python to python3 - needed for glew build
ln -s /usr/bin/python3 /usr/bin/python

# System dependencies
yum install -y --setopt=install_weak_deps=False \
    libuuid-devel metis-devel
