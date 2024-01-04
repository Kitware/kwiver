# Install KWIVER to /opt/kitware/kwiver
# Use latest Fletch as base image (Ubuntu 20.04)

ARG BASE_IMAGE=kitware/fletch:latest-ubuntu20.04-py3
ARG ENABLE_CUDA=OFF

FROM ${BASE_IMAGE}

# Install system dependencies
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y --no-install-recommends xvfb

# Remove unnecessary files
RUN apt-get clean
RUN rm -rf /var/lib/apt/lists/*

# Setup build environment
COPY . /kwiver
RUN cd /kwiver && mkdir build

# Configure
RUN cd /kwiver/build && \
    cmake .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/kitware/kwiver \
      -Dfletch_DIR:PATH=/opt/kitware/fletch/share/cmake \
      -DKWIVER_ENABLE_ARROWS=ON \
      -DKWIVER_ENABLE_C_BINDINGS=ON \
      -DKWIVER_ENABLE_CERES=ON \
      -DKWIVER_ENABLE_CUDA=${ENABLE_CUDA} \
      -DKWIVER_ENABLE_EXTRAS=ON \
      -DKWIVER_ENABLE_LOG4CPLUS=ON \
      -DKWIVER_ENABLE_OPENCV=ON \
      -DKWIVER_ENABLE_FFMPEG=ON \
      -DKWIVER_ENABLE_KLV=ON \
      -DKWIVER_ENABLE_MVG=ON \
      -DKWIVER_ENABLE_PROCESSES=ON \
      -DKWIVER_ENABLE_PROJ=ON \
      -DKWIVER_ENABLE_PYTHON=ON \
      -DKWIVER_ENABLE_SERIALIZE_JSON=ON \
      -DKWIVER_ENABLE_SPROKIT=ON \
      -DKWIVER_ENABLE_TESTS=ON \
      -DKWIVER_ENABLE_TOOLS=ON \
      -DKWIVER_ENABLE_VXL=ON \
      -DKWIVER_ENABLE_DOCS=ON \
      -DKWIVER_INSTALL_DOCS=ON \
      -DKWIVER_PYTHON_MAJOR_VERSION=3 \
      -DKWIVER_USE_BUILD_TREE=ON \
      -DKWIVER_INSTALL_SET_UP_SCRIPT=ON

# Build
RUN cd /kwiver/build && \
    . ./setup_KWIVER.sh && \
    make -j$(nproc) -k && \
    make install

# Install python build requirements if they exist
ENV PYTHON_REQS="/kwiver/build/python/requirements.txt"
RUN [ ! -f "$PYTHON_REQS" ] || pip3 install -r "$PYTHON_REQS"

# Remove source
RUN rm -rf /kwiver

# Configure entrypoint
RUN bash -c '\
#!/bin/bash\n\
echo -e "source /opt/kitware/kwiver/setup_KWIVER.sh\n\
\n\
# Set up X virtual framebuffer (Xvfb) to support running VTK headless\n\
Xvfb :1 -screen 0 1024x768x16 -nolisten tcp > xvfb.log &\n\
export DISPLAY=:1.0\n\
\n\
/opt/kitware/kwiver/bin/kwiver \$@" > /entrypoint.sh' && \
    chmod +x /entrypoint.sh

ENTRYPOINT [ "bash", "/entrypoint.sh" ]
CMD [ "help" ]
