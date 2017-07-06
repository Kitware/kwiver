set(pybind11_library
  pybind11)

find_package(Boost ${sprokit_boost_version} REQUIRED
  COMPONENTS
    iostreams)

find_package(pybind11)

find_package(PythonLibs ${PYTHON_VERSION} REQUIRED)
include_directories(SYSTEM ${PYTHON_INCLUDE_DIR})

find_package(PythonInterp ${PYTHON_VERSION})

add_subdirectory(util)
