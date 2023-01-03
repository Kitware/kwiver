FROM ubuntu:22.04

# Install System Dependencies
#
RUN apt-get update && \
    apt-get install --no-install-recommends -y build-essential \
                                               libgl1-mesa-dev \
                                               libexpat1-dev \
                                               libgtk2.0-dev \
                                               libxt-dev \
                                               libxml2-dev \
                                               libssl-dev \
                                               liblapack-dev \
                                               openssl \
                                               curl \
                                               wget \
                                               git \
                                               libreadline-dev \
                                               zlib1g-dev

# Install python
RUN apt-get install --no-install-recommends -y python3 \
                                               python3-dev \
                                               python3-pip

RUN ln -s /usr/bin/python3 /usr/local/bin/python

# Install CMake 3.25
RUN wget https://apt.kitware.com/kitware-archive.sh \
  && chmod +x kitware-archive.sh \
  && ./kitware-archive.sh \
  && rm kitware-archive.sh

RUN apt-get install -y cmake

RUN apt-get clean \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p fletch/build

RUN cd fletch \
  && git clone https://github.com/chetnieter/fletch src \
  && cd src \
  && git checkout dev/update-pybind11-version

RUN cd fletch/build \
  && cmake -DCMAKE_BUILD_TYPE=Release \
    -Dfletch_BUILD_CXX11=ON \
    -Dfletch_BUILD_WITH_PYTHON=ON \
    -Dfletch_PYTHON_MAJOR_VERSION=3 \
    -Dfletch_ENABLE_Eigen=ON \
    -Dfletch_ENABLE_pybind11=ON \
    ../src \
  && make -j 4

RUN mkdir -p kwiver/build
COPY . /kwiver/src

RUN cd kwiver/build \
  && cmake -DCMAKE_BUILD_TYPE=Release \
    -Dfletch_DIR=/fletch/build \
    -DKWIVER_ENABLE_PYTHON=ON \
    -DKWIVER_ENABLE_TOOLS=ON \
    ../src \
  && make -j 4
  
