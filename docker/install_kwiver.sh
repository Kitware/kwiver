#!/usr/bin/sh

set -e

readonly kwiver_repo="https://gitlab.kitware.com/kwiver/kwiver.git"
readonly kwiver_commit=$1

echo "Installing KWIVER from $kwiver_repo at commit $kwiver_commit"

readonly kwiver_root="$HOME/kwiver"
readonly kwiver_src="$kwiver_root/src"
readonly kwiver_build="$kwiver_root/build"
readonly kwiver_prefix="/opt/kitware/kwiver"

# check for existing dir - helpful for debugging build failures interacively
if [ ! -d "$kwiver_src" ]; then
  git clone "$kwiver_repo" "$kwiver_src"
  git -C "$kwiver_src" checkout "$kwiver_commit"
fi

# install a modern version of cmake, used by CI
cd "$kwiver_src"
.gitlab/ci/cmake.sh
export PATH=$PWD/.gitlab:$PWD/.gitlab/cmake/bin:$PATH
cmake --version

# fletch didn't install python build requirements
# TODO inside venv to allow env cleanup? Numpy (others?) still required for tool execution.
pip3 install -r python/requirements_dev.txt

cmake \
  -B "$kwiver_build" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$kwiver_prefix" \
  -S "$kwiver_src" \
      -Dfletch_DIR:PATH=/opt/fletch/share/cmake \
      -DKWIVER_ENABLE_ARROWS=ON \
      -DKWIVER_ENABLE_CERES=ON \
      -DKWIVER_ENABLE_CUDA=OFF \
      -DKWIVER_ENABLE_EXTRAS=OFF \
      -DKWIVER_ENABLE_LOG4CPLUS=OFF \
      -DKWIVER_ENABLE_OPENCV=ON \
      -DKWIVER_ENABLE_FFMPEG=ON \
      -DKWIVER_ENABLE_KLV=ON \
      -DKWIVER_ENABLE_MVG=ON \
      -DKWIVER_ENABLE_PROCESSES=OFF \
      -DKWIVER_ENABLE_PROJ=ON \
      -DKWIVER_ENABLE_PYTHON=ON \
      -DKWIVER_ENABLE_SERIALIZE_JSON=ON \
      -DKWIVER_ENABLE_SERIALIZE_PROTOBUF=ON \
      -DKWIVER_ENABLE_SPROKIT=OFF \
      -DKWIVER_ENABLE_TESTS=OFF \
      -DKWIVER_ENABLE_TOOLS=ON \
      -DKWIVER_ENABLE_VXL=ON \
      -DKWIVER_ENABLE_DOCS=ON \
      -DKWIVER_INSTALL_DOCS=ON \
      -DKWIVER_USE_BUILD_TREE=ON \
      -DKWIVER_INSTALL_SET_UP_SCRIPT=ON

cmake --build "$kwiver_build" --parallel "$(nproc)"
cmake --install "$kwiver_build"

# # Clean up.
rm -rf "$kwiver_root"
