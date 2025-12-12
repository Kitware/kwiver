FROM quay.io/pypa/manylinux_2_28_x86_64:2024-04-23-ef7507e
MAINTAINER Christos Tsolakis <christos.tsolakis@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

COPY install_fletch.sh /root/install_fletch.sh
COPY configure_fletch.cmake /root/configure_fletch.cmake
RUN sh /root/install_fletch.sh
