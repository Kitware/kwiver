# Install KWIVER to /opt/kitware/kwiver
# Use latest Fletch as base image (Ubuntu 18.04)

FROM kitware/fletch:latest-ubuntu18.04-py3-cuda10.0-cudnn7-devel

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
                    python3-dev \
                    python3-pip \
 && pip3 install setuptools \
                 scipy \
                 six

#
# Build KWIVER
#

COPY . /kwiver
RUN cd /kwiver \
  && mkdir build \
  && cd build \
  && cmake ../ -DCMAKE_BUILD_TYPE=Release \
    -Dfletch_DIR:PATH=/opt/kitware/fletch/share/cmake \
    -DKWIVER_ENABLE_ARROWS=ON \
    -DKWIVER_ENABLE_C_BINDINGS=ON \
    -DKWIVER_ENABLE_CERES=ON \
    -DKWIVER_ENABLE_CUDA=ON \
    -DKWIVER_ENABLE_EXTRAS=ON \
    -DKWIVER_ENABLE_LOG4CPLUS=ON \
    -DKWIVER_ENABLE_OPENCV=ON \
    -DKWIVER_ENABLE_FFMPEG=ON \
    -DKWIVER_ENABLE_MVG=ON \
    -DKWIVER_ENABLE_PROCESSES=ON \
    -DKWIVER_ENABLE_PROJ=ON \
    -DKWIVER_ENABLE_PYTHON=ON \
    -DKWIVER_ENABLE_SPROKIT=ON \
    -DKWIVER_ENABLE_TESTS=ON \
    -DKWIVER_ENABLE_TOOLS=ON \
    -DKWIVER_ENABLE_VXL=ON \
    -DKWIVER_ENABLE_DOCS=ON \
    -DKWIVER_INSTALL_DOCS=ON \
    -DKWIVER_PYTHON_MAJOR_VERSION=3 \
    -DKWIVER_USE_BUILD_TREE=ON \
  && make -j$(nproc) -k \
  && make install \
  && chmod +x setup_KWIVER.sh
# Optionally install python build requirements if it was generated.
RUN PYTHON_REQS="python/requirements.txt" \
 && cd /kwiver/build \
 && ( ( [ ! -f "$PYTHON_REQS" ] \
        && echo "!!! No build requirements generated, nothing to install." ) \
      || pip3 install -r "$PYTHON_REQS" )

# Configure entrypoint
RUN echo 'source /kwiver/build/setup_KWIVER.sh' >> entrypoint.sh \
 && echo 'kwiver $@' >> entrypoint.sh \
 && chmod +x entrypoint.sh

ENTRYPOINT [ "bash", "/entrypoint.sh" ]
CMD [ "help" ]
