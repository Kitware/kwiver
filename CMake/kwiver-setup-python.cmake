###
# Finds the python binaries, libs, include, and site-packages paths
#
# The purpose of this file is to export variables that will be used in
# kwiver/CMake/utils/kwiver-utils-python.cmake and
# kwiver/sprokit/conf/sprokit-macro-python.cmake (the latter will eventually be
# consolidated into the former)
#
# User options defined in this file:
#
#    KWIVER_PYTHON_MAJOR_VERSION
#      The major python version to target (either 2 or 3)
#
#
# Calls find_packages to on python interpreter/libraries which defines:
#
#    PYTHON_EXECUTABLE
#    PYTHON_INCLUDE_DIR
#    PYTHON_LIBRARY
#    PYTHON_LIBRARY_DEBUG
#
# Exported variables used by python utility functions are:
#
#    PYTHON_VERSION
#      the major/minor python version
#
#    PYTHON_ABI_FLAGS
#      Python abstract binary interface flags (used internally for defining
#      subsequent variables, but settable by the user as an advanced setting)
#
#    python_site_packages
#      Location where python packages are installed relative to your python
#      install directory. For example:
#        Windows system install: Lib\site-packages
#        Debian system install: lib/python2.7/dist-packages
#        Debian virtualenv install: lib/python3.5/site-packages
#
#    python_sitename
#      The basename of the python_site_packages directory. This is either
#      site-packages (in most cases) or dist-packages (if your python was
#      configured by a debian package manager). If you are using a python
#      virtualenv (you should be) then this will be site-packages
#
#    kwiver_python_subdir
#      basename of the python lib folder (that contains site-packages).
#      Depends on the python major/minor version and the ABI flags
#      (e.g. python2.7, python3.5m)
#
#    kwiver_python_output_path
#      The location in the build tree to copy/symlink python modules Depends on
#      the value of `kwiver_python_subdir`.
#      (e.g. build/lib/python2.7, build/lib/python3.5m)
#
#    kwiver_python_install_path
#      The base location in the install tree where python files/modules are
#      to be installed.
#      (e.g. ${CMAKE_INSTALL_PREFIX}/lib/python3)
#
#    sprokit_python_output_path
#      Similar to `kwiver_python_output_path`. Used by sprokit to define extra
#      python output paths. This may be removed in the future.
#      (e.g. build/lib)
#

###
# Private helper function to execute `python -c "<cmd>"`
#
# Runs a python command and populates an outvar with the result of stdout.
# Be careful of indentation if `cmd` is multiline.
#
function(_pycmd outvar cmd)
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "${cmd}"
    RESULT_VARIABLE _exitcode
    OUTPUT_VARIABLE _output)
  if(NOT ${_exitcode} EQUAL 0)
    message(ERROR "Failed when running python code: \"\"\"
${cmd}\"\"\"")
    message(FATAL_ERROR "Python command failed with error code: ${_exitcode}")
  endif()
  # Remove supurflous newlines (artifacts of print)
  string(STRIP "${_output}" _output)
  set(${outvar} "${_output}" PARENT_SCOPE)
endfunction()


###
# Python major version user option
#

# Respect the PYTHON_VERSION_MAJOR version if it is set
if (PYTHON_VERSION_MAJOR)
  set(DEFAULT_PYTHON_MAJOR ${PYTHON_VERSION_MAJOR})
else()
  set(DEFAULT_PYTHON_MAJOR "3")
endif()


set(KWIVER_PYTHON_MAJOR_VERSION "${DEFAULT_PYTHON_MAJOR}" CACHE STRING "Python version to use: 3 or 2")
set_property(CACHE KWIVER_PYTHON_MAJOR_VERSION PROPERTY STRINGS "3" "2")

option(KWIVER_PYTHON_USE_SYS_PATH "Use installed python version specific path formatting" ON)
mark_as_advanced(KWIVER_PYTHON_USE_SYS_PATH)

###
# Detect major version change (part1)
#
# Clear cached variables when the user changes major python versions.
# When this happens, we need to re-find the bin, include, and libs
#
if (NOT __prev_kwiver_pyversion STREQUAL KWIVER_PYTHON_MAJOR_VERSION)
  # but dont clobber initial settings in the instance they are specified via
  # commandline (e.g  cmake -DPYTHON_EXECUTABLE=/my/special/python)
  if (__prev_kwiver_pyversion)
    message(STATUS "The Python version changed; refinding the interpreter")
    message(STATUS "The previous Python version was: \"${__prev_kwiver_pyversion}\"")
    unset(__prev_kwiver_pyversion CACHE)
    unset(PYTHON_EXECUTABLE CACHE)
    unset(PYTHON_INCLUDE_DIR CACHE)
    unset(PYTHON_LIBRARY CACHE)
    unset(PYTHON_LIBRARY_DEBUG CACHE)
    unset(PYTHON_ABIFLAGS CACHE)
  endif()
endif()


###
#
# Mark the previous version so we can determine when python versions change
#
set(__prev_kwiver_pyversion "${KWIVER_PYTHON_MAJOR_VERSION}" CACHE INTERNAL
  "allows us to determine if the user changes python version")

###
# Python interpreter and libraries
#
if (KWIVER_PYTHON_MAJOR_VERSION STREQUAL "3")
  # note, 3.4 is a minimum version
  find_package(PythonInterp 3.4 REQUIRED)
  find_package(PythonLibs 3.4 REQUIRED)
else()
  find_package(PythonInterp 2.7 REQUIRED)
  find_package(PythonLibs 2.7 REQUIRED)
endif()
include_directories(SYSTEM ${PYTHON_INCLUDE_DIR})


###
# Temporary fix around some kwiver/cmake find python issues
#
if( "${PYTHON_LIBRARIES}" MATCHES "^.*;.*$" )
  list( GET PYTHON_LIBRARIES 1 PYTHON_LIBRARY )
  set( PYTHON_LIBRARIES "${PYTHON_LIBRARY}" CACHE INTERNAL "Auto-convert python library" )
endif()

if(NOT EXISTS "${PYTHON_LIBRARY}")
  string(REPLACE "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}m"
                 "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}"
                 PYTHON_LIBRARY
                 ${PYTHON_LIBRARY})
  set(PYTHON_LIBRARIES "${PYTHON_LIBRARY}")
endif()

###
# Python site-packages
#
# Get canonical directory for python site packages (relative to install
# location).  It varys from system to system.
#
_pycmd(python_site_packages "from distutils import sysconfig; print(sysconfig.get_python_lib(prefix=''))")
message(STATUS "python_site_packages = ${python_site_packages}")

# Current usage determines most of the path in alternate ways.
# All we need to supply is the '*-packages' directory name.
# Customers could be converted to accept a larger part of the path from this function.
get_filename_component(python_sitename ${python_site_packages} NAME)

###
# Python install path
if(NOT KWIVER_PYTHON_USE_SYS_PATH)
  set(kwiver_sitename "site-packages")
  set(kwiver_python_version "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
  set(kwiver_site_packages "lib/python${kwiver_python_version}/${kwiver_sitename}")
  set(kwiver_python_install_path "${CMAKE_INSTALL_PREFIX}/${kwiver_site_packages}")
else()
  set(kwiver_sitename "${python_sitename}")
  set(kwiver_python_install_path "${CMAKE_INSTALL_PREFIX}/${python_site_packages}")
endif()
message(STATUS "kwiver_python_install_path = ${kwiver_python_install_path}")

###
# Python major/minor version
#
# Use the executable to find the major/minor version.
# If you want to change this, then change the executable.
#
_pycmd(PYTHON_VERSION "import sys; print(sys.version[0:3])")
# assert that the right python version was found
if(NOT PYTHON_VERSION MATCHES "^${KWIVER_PYTHON_MAJOR_VERSION}.*")
  message(STATUS "KWIVER_PYTHON_MAJOR_VERSION = ${KWIVER_PYTHON_MAJOR_VERSION}")
  message(STATUS "PYTHON_VERSION = ${PYTHON_VERSION}")
  message(FATAL_ERROR "Requested python \"${KWIVER_PYTHON_MAJOR_VERSION}\" but got \"${PYTHON_VERSION}\"")
endif()


###
# Python ABI Flags
#
# See PEP 3149 - ABI (application binary interface) version tagged .so files
# https://www.python.org/dev/peps/pep-3149/
#
if (KWIVER_PYTHON_MAJOR_VERSION STREQUAL "3")
  # In python 3, we can determine what the ABI flags are
  _pycmd(_python_abi_flags "from distutils import sysconfig; print(sysconfig.get_config_var('ABIFLAGS'))")
else()
  # Not sure if ABI flags are easilly found (or are even used in python2)
  set(_python_abi_flags, "")
endif()
set(PYTHON_ABIFLAGS "${_python_abi_flags}"
  CACHE STRING "The ABI flags for the version of Python being used")
mark_as_advanced(PYTHON_ABIFLAGS)


###
# Python dependencies
#
# Add python packages needed to execute the bindings
# to requirements.txt file
#
list(APPEND PYTHON_REQS "numpy>=1.13.0,<=1.19.0" "six>=1.10.0,<=1.13.0")

if(SKBUILD)
  list(APPEND PYTHON_REQS "scikit-build<=0.11.1")
endif()

if(KWIVER_ENABLE_PYTORCH)
  list(APPEND PYTHON_REQS "opencv-python>=3.4.2.17,<=4.0.0"
                          "pillow>=7.0.0,<=7.1.2"
                          "scipy>=1.2,<=1.5"
                          "torch==1.4.0"
                          "torchvision==0.5.0"
                        )
endif()


###
# PyBind11
#
#
#
  set(pybind11_library     python)
  find_package(pybind11)


###
# Python Dependencies
#
# sets the python dependencies defined in python/requirements.txt
# to be a custom command of the python libraries target
# a venv will be created to encapsulate the pip installed dependencies
# from the larger system, while still providing the tests access to their dependencies
#

if (KWIVER_ENABLE_TESTS)

  message(STATUS "KWIVER_PYTHON and KWIVER_TESTING enabled.")
  message(STATUS "Python tests will be added to CTest.")
  message(STATUS "Tests are executed by PYTEST, for sucessful test execution, please install the appropriate pytest for your python distribution.")
  message(STATUS "Testing dependencies will be added to Python requirements.")
  set(PYTHON_TEST 1)
  set(PYTHON_REQS "${PYTHON_REQS};nose>1.2;coverage>=4.4.1,<5.0.0;pytest>=4.6,<=6.0;multimethod>=1.2,<=1.4")

endif()

string(REPLACE ";" "\n" PYTHON_REQS "${PYTHON_REQS}")
#file(WRITE ${KWIVER_BINARY_DIR}/python/requirements.txt "${PYTHON_REQS}")
###
# Python package build locations
#
# defines paths used to determine where the kwiver/sprokit/vital python
# packages will be generated in the build tree. (TODO: python modules should
# use a setup.py file to install themselves to the right location)
if(NOT KWIVER_PYTHON_USE_SYS_PATH)
  set(kwiver_python_subdir "python${kwiver_python_version}")
else()
  # Instead of contructing the directory with ABIFLAGS just use what python gives
  get_filename_component(python_lib_subdir ${python_site_packages} DIRECTORY)
  get_filename_component(python_subdir ${python_lib_subdir} NAME)

  set(kwiver_python_subdir "${python_subdir}")
endif()
set(kwiver_python_output_path "${KWIVER_BINARY_DIR}/${kwiver_python_subdir}")

# Currently needs to be separate because sprokit may have CONFIGURATIONS that
# are placed between lib and `kwiver_python_subdir`
set(sprokit_python_output_path "${KWIVER_BINARY_DIR}/lib")

set(KWIVER_PYTHON_VERSION "${PYTHON_VERSION}" CACHE STRING "" )
mark_as_advanced(KWIVER_PYTHON_VERSION)

###
# Status string for debugging
#
set(PYTHON_CONFIG_STATUS "
PYTHON_CONFIG_STATUS

  * KWIVER_PYTHON_MAJOR_VERSION = \"${KWIVER_PYTHON_MAJOR_VERSION}\"

  * PYTHON_EXECUTABLE = \"${PYTHON_EXECUTABLE}\"
  * PYTHON_INCLUDE_DIR = \"${PYTHON_INCLUDE_DIR}\"
  * PYTHON_LIBRARY = \"${PYTHON_LIBRARY}\"
  * PYTHON_LIBRARY_DEBUG = \"${PYTHON_LIBRARY_DEBUG}\"

  * PYTHON_ABIFLAGS = \"${PYTHON_ABIFLAGS}\"
  * PYTHON_VERSION = \"${PYTHON_VERSION}\"

  * python_site_packages = \"${python_site_packages}\"
  * python_sitename = \"${python_sitename}\"

  * kwiver_python_subdir = \"${kwiver_python_subdir}\"
  * kwiver_python_install_path = \"${kwiver_python_install_path}\"
  * kwiver_python_output_path = \"${kwiver_python_output_path}\"
  * sprokit_python_output_path = \"${sprokit_python_output_path}\"
")

message(STATUS "${PYTHON_CONFIG_STATUS}")
