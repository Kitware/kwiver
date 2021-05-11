PyKwiver
========

PyKwiver is a suite of PyBind11 bindings designed to expose C++ functionality to a pythonic API.

Kwiver's Python architecture largely follows the general architecture of the C++ implementation of Kwiver.
The bindings expose Vital, Arrows, and Sprokit.

The current state of Python Arrows may not be a perfect 1-1 analog of the Arrows encompassed by the C++ Arrows, as development
is active and in progress. As a direct result of this, assuming the Python bindings provide an identical suite of Arrows would be incorrect. Refer to
the python/kwiver/arrows directory.

The Python arrows are enabled via CMake during configuration by enabling a given arrow, and additionally enabling Python.


