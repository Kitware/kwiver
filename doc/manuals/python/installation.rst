Python Binding Installation
===========================

The Kwiver Python bindings can be aquired one of two ways.

The first and much simpler option is to install the Kwiver python wheel
with the pip python package manager. The most current (and functional) wheels are currently hosted by a github pages instance compatible with
PYPA standards. The wheel can be installed via the python pip package manager. Pip typically sources wheels from PYPI, the python package index.
However by providing the `--extra-url` argument, pip will pull wheel files from all PYPA compatible repositories specified by the urls passed to the extra-url flag.
The github pages host for the wheel can be found [here](https://johnwparent.github.io/kwiver-wheels/). The wheels currently support all systems supported by the
PEP 599 pypa provided manylinux2014 docker image. Support for MacOS and Windows is untested and in no way guaranteed to exist/function as expected.
Support for Windows and MacOS wheels is an ongoing effort, and this documentation will be updated when support is available.

The second (and only for MacOS and Windows) option is to build Kwiver from [source](https://github.com/Kitware/kwiver) and to enable python bindings during CMake configuration.
More detailed, comprehensive build instructions can be found at the root of the Kwiver source tree. Once Kwiver has finished building from source, it can be installed or
the bindings can be utilized from the build tree by executing the setup_KWIVER script. Alternatively, the user can manually set the requisite environment variables required
to locate and run Kwiver python Sporkit processes and arrows. Said variables are discussed in more detail at a later point.

Once installation is complete, Kwiver can be used as normal, with the added bonus of support for Python Processes, Arrows, and Types.

See further documentation for instructions and examples regarding the implementation and extension of Kwiver C++ features in Python.