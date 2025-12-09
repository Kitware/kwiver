#!/usr/bin/sh

set -e

readonly fletch_repo="https://github.com/Kitware/fletch"
readonly fletch_commit="e820b47c3004389ca46b6981cde2e5f416b8a31e"

readonly fletch_root="$HOME/fletch"
readonly fletch_src="$fletch_root/src"
readonly fletch_build="$fletch_root/build"
readonly fletch_prefix="/opt/fletch"

# check for existing dir - helpful for debugging build failures interacively
if [ ! -d "$fletch_src" ]; then
  git clone "$fletch_repo" "$fletch_src"
  git -C "$fletch_src" checkout "$fletch_commit"

  git -C "$fletch_src" config user.name "kwiver Developers"
  git -C "$fletch_src" config user.email "kwiver-developers@kitware.com"
fi

cmake \
  -B "$fletch_build" \
  "-Dfletch_BUILD_INSTALL_PREFIX=$fletch_prefix" \
  -C "$HOME/configure_fletch.cmake" \
  -S "$fletch_src"
cmake --build "$fletch_build" --parallel "$(nproc)"

# Clean up.
rm -rf "$fletch_root"
