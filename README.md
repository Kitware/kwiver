# KWIVER

Kitware Image and Video Exploitation and Retrieval

---

The KWIVER  toolkit is a collection of software tools designed to tackle challenging image and video analysis problems and other related challenges. Recently started by Kitware’s Computer Vision and Scientific Visualization teams, KWIVER is an ongoing effort to transition technology developed over multiple years to the open source domain to further research, collaboration, and product development.

The project is structured with the parent `kwiver` repository working as a CMake "super-build" that pulls in a number of KWIVER and other open source projects. Following the archery analogy, KWIVER works with arrows (i.e., plugins) to provide flexible integration with other libraries and algorithms.

## Building KWIVER
These instructions will build Kwiver and all of its dependencies starting from a fresh Ubuntu 14.04 LTS 64-bit installation.  Start a terminal (ctrl+alt+t) and update the existing packages on Ubuntu:

	sudo apt-get update && sudo apt-get upgrade

Get all the required [build components](http://packages.ubuntu.com/trusty/build-essential):
	
	sudo apt-get install build-essential
	
Get the free implementation of [OpenGL API -- GLX dev](http://packages.ubuntu.com/trusty/libgl1-mesa-dev)

	sudo apt-get install libgl1-mesa-dev

Install [GIT](https://git-scm.com/) for version control and convenient cloning of packages ([package: git](http://packages.ubuntu.com/trusty/git)):

	sudo apt-get install git

Install LAPACK ([package: liblapack-dev](http://packages.ubuntu.com/trusty/liblapack-dev)), which is needed by [SuiteSparse](http://faculty.cse.tamu.edu/davis/suitesparse.html), which is used by the Ceres solver:
	
	sudo apt-get install liblapack-dev

Install X11 toolkit intrinsics library with development headers ([libxt-dev](http://packages.ubuntu.com/precise/libxt-dev)), which is a dependency for at least VTK:

	sudo apt-get install libxt-dev

Install [libexpat1-dev](http://packages.ubuntu.com/trusty/libexpat1-dev), an XML parsing C library, which is needed to build libkml:

	sudo apt-get install libexpat1-dev

Install [libhdf5-dev](http://packages.ubuntu.com/trusty/libhdf5-dev), which is needed for OpenCV's extra modules:

	apt-get install libhdf5-dev

Install [libjpeg-dev](http://packages.ubuntu.com/precise/libjpeg-dev):

	apt-get install libjpeg-devel

Decide on a top-level directory location for KWIVER and its dependencies.  The rest of the documentation will assume a top level directory of ~/libraries:

	mkdir ~/libraries && cd ~/libraries

Install cmake-3.8.0-rc1 from pre-compiled binaries.

	curl -O https://cmake.org/files/v3.8/cmake-3.8.0-rc1-Linux-x86_64.sh
	chmod +x cmake-3.8.0-rc1-Linux-x86_64.sh
	sudo sh ./cmake-3.8.0-rc1-Linux-x86_64.sh --prefix=/opt/cmake
	rm cmake-3.8.0-rc1-Linux-x86_64.sh

Add the CMake bin directory to your system PATH
	
	echo >> ~/.bashrc
	echo 'export PATH=$PATH:/opt/cmake/cmake-3.8.0-rc1-Linux-x86_64/bin' >> ~/.bashrc
	source ~/.bashrc

 Get [Anaconda Python](https://store.continuum.io/cshop/anaconda/).  This is optional but convenient as it has lots of packages.  Make sure to accept when asked if you want to prepend the Anaconda2 install location to PATH.

	curl -O https://repo.continuum.io/archive/Anaconda2-4.3.0-Linux-x86_64.sh
	chmod +x Anaconda2-4.3.0-Linux-x86_64.sh
	./Anaconda2-4.3.0-Linux-x86_64.sh
	rm Anaconda2-4.3.0-Linux-x86_64.sh
	source ~/.bashrc

### Fletch

KWIVER has (and will have more) a number of dependencies on 3rd party Open Source libraries.  To make it easier to build KWIVER, especially on systems like Microsoft Windows that don't have package manager, [Fletch](https://github.com/Kitware/fletch) was developed to gather, configure and build those packages for use with KWIVER.  Fletch is a [CMake](www.cmake.org) based "super-build" that takes care of most of the build details for you.
	
To build Fletch, clone the Fletch repository:
	
	cd ~/libraries
	git clone https://github.com/Kitware/fletch.git fletch
	cd fletch
	git submodule update --init

We recommend that all builds are done outside of the source directory in a parallel build tree to prevent mixing source files with compiled products

	mkdir ~/libraries/fletch-build && cd ~/libraries/fletch-build

Note: if Anaconda Python was installed and added to PATH, cmake should find it and automatically populate PYTHON_EXECUTABLE, PYTHON_INCLUDE, and PYTHON_LIBRARY.  However, if it does not and you want to use it over the native Python installations, you can pass the following arguements to the cmake command:

* `-DPYTHON_INCLUDE_DIR=/path/to/python/include/directory`  For example, for a default Python 2.7 Anaconda install on Linux, this would be `${HOME}/anaconda/include/python2.7`
* `-DPYTHON_EXECUTABLE=/path/to/executable/python` For example, for a default Python 2.7 Anaconda install on Linux, this would `${HOME}/anaconda/bin/python`
* `-DPYTHON_LIBRARY=/path/to/python/library` For example, for a default Python 2.7 Anaconda install on Linux, this would be `${HOME}/anaconda/lib/libpython2.7.so`

Run the following 'cmake' command to perform a default build cofiguration with all packages selected:

	cmake -Dfletch_BUILD_WITH_PYTHON:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Release -Dfletch_ENABLE_Boost:BOOL=TRUE -Dfletch_ENABLE_OpenCV:BOOL=TRUE -Dfletch_ENABLE_ALL_PACKAGES=TRUE ../fletch

If you would prefer a graphical interface to toggle individual components, swap cmake for ccmake in the above call.  An iterative process of changing desired parameters and pressing 'c' is required until the generate command 'g' is available.  Each time you press 'c', more options open up, which cmake will try to satisfy.

Once your `cmake` command has completed and a Makefile generated, you can build with the following command (using all cores)

	make -j$(nproc)

Fletch does an install by default, so running  `make install` is not necessary.  You will pass the Fletch build directory as an argument to cmake during the KWIVER build configuration.

### KWIVER

Once Fletch has been built, it's possible to build the `kwiver` repository as well.  This repo is also a CMake super-build
and can be fetched with this command:

	cd ~/libraries
	git clone https://github.com/Kitware/kwiver.git

The build can be configured with the following command:

	mkdir ~/libraries/kwiver-build && cd ~/libraries/kwiver-build
	cmake -DKWIVER_ENABLE_PYTHON:BOOL=ON -Dfletch_DIR:PATH=~/libraries/fletch-build ~/libraries/kwiver

As with Fletch, if you want to specify a particular Python installation (such as Anaconda) use the `-DPYTHON...` command arguments as outlined in the Fletch section.  However, CMake should find Anaconda if it has been added to PATH.

Once your `cmake` command has completed, use `make` (on Linux) to build it.

## Running KWIVER

Once you've built KWIVER, you'll want to test that it's working on your system.  From a command prompt, execute the following commands to set up the KWIVER environment:
	
	cd ~/libraries/kwiver-build
	chmod +x setup_KWIVER.sh
	source ./setup_KWIVER.sh

This will set up your PATH, PYTHONPATH and other environment variables to allow KWIVER to work conveniently.

The central component of KWIVER is ([Sprokit](https://github.com/Kitware/sprokit)).  We use Sprokit's pipelining facilities to manage, integrate and run many of KWIVER's modules and capabilities.  To see what modules (called processes in Sprocket) are available, issue the `processopedia` command.  Here's a typical list of modules (note that as KWIVER expands, this list is likely to grow):

	any_source: A process which creates arbitrary data
	collate: Collates data from multiple worker processes
	const: A process with the const flag
	const_number: Outputs a constant number
	data_dependent: A process with a data dependent type
	distribute: Distributes data to multiple worker processes
	duplicate: A process which duplicates input
	expect: A process which expects some conditions
	feedback: A process which feeds data into itself
	flow_dependent: A process with a flow dependent type
	frame_list_process: A process that reads a list of image file names and generates stream of images and associated time stamps
	kw_archive_writer_process: A process to write kw archives
	kw_print_number_process: A Simple Kwiver Test Process
	multiplication: Multiplies numbers
	multiplier_cluster: A constant factor multiplier cluster
	mutate: A process with a mutable flag
	numbers: Outputs numbers within a range
	orphan: A dummy process
	orphan_cluster: A dummy cluster
	pass: Pass a data stream through
	print_number: Print numbers to a file
	shared: A process with the shared flag
	sink: Ignores incoming data
	skip: A process which skips input data
	stabilize_image_process: A process to generate current-to-reference image homographies
	tagged_flow_dependent: A process with a tagged flow dependent types
	take_number: Print numbers to a file
	take_string: Print strings to a file
	tunable: A process with a tunable parameter

This is the list of modules that can be included in a Sprokit pipeline.  We're going to use the `numbers` module and the the `print_number` module to create a very simple pipeline.  To learn more about the `numbers` module we'll again use `processopedia` this time to get details on a particular module.  For `numbers` we'll use the `processopedia -t numbers -d` command:

	Process type: numbers
	  Description: Outputs numbers within a range
	  Properties: _no_reentrant
	  Configuration:
	    Name       : end
	    Default    : 100
	    Description: The value to stop counting at.
	    Tunable    : no

	    Name       : start
	    Default    : 0
	    Description: The value to start counting at.
	    Tunable    : no

	  Input ports:
	  Output ports:
	    Name       : number
	    Type       : integer
	    Flags      : _required
	    Description: Where the numbers will be available.

And for `print_number`, we'll use `processopedia -t print_number -d`:

	Process type: print_number
	  Description: Print numbers to a file
	  Properties: _no_reentrant
	  Configuration:
	    Name       : output
	    Default    :
	    Description: The path of the file to output to.
	    Tunable    : no

	  Input ports:
	    Name       : number
	    Type       : integer
	    Flags      : _required
	    Description: Where numbers are read from.

	  Output ports:

The output of these commands tells us enough about each process to construct a Sprocket ".pipe" file that defines a processing pipeline.  In particular we'll need to know how to configure each process (the "Configuration") and how they can be hooked together (the input and output "Ports").

KWIVER comes with a sample [kwiver/sprokit/pipelines/number_flow.pipe](kwiver/sprokit/pipelines/number_flow.pipe) file that configures and connects the pipeline so that the `numbers` process will generate a set of integers from 1 to 99 and the `print_number` process will write those to a file called `numbers.txt`.  Of particular interest is the section at the end of the file that actually "hooks up" the pipeline.

To run the pipeline, we'll use the Sprokit `pipeline_runner` command:

	pipeline_runner -p </path/to/kwiver/source>/kwiver/pipeline_configs>/number_flow.pipe

After the pipeline completes, you should find a file, `numbers.txt`, in your working directory.

### Python Processes

One KWIVER's great strengths (as provided by Sprokit) is the ability to create hybrid pipelines which combine C++ and Python processes in the same pipeline.  This greatly facilitates prototyping complex processing pipelines.  To test this out we'll still use the `numbers` process, but we'll use a Python version of the `print_number` process called `kw_print_number_process` the code for which can be seen in [kwiver/sprokit/processes/python/kw_print_number_process.py](kwiver/sprokit/processes/python/kw_print_number_process.py).    As usual, we can lean about this process with a `processopedia` command: `processopedia -t kw_print_number_process -d`:

	Process type: kw_print_number_process
	  Description: A Simple Kwiver Test Process
	  Properties: _no_reentrant, _python
	  Configuration:
	    Name       : output
	    Default    : .
	    Description: The path for the output file.
	    Tunable    : no

	  Input ports:
	    Name       : input
	    Type       : integer
	    Flags      : _required
	    Description: Where numbers are read from.

	  Output ports:

As you can see, the process is very similar to the C++ `print_number` process.  As a result, the [".pipe" file is very similar](kwiver/pipeline_configs/number_flow_python.pipe).

In order to get around limitations imposed by the Python Global Interpreter Lock, we'll use a different Sprokit scheduler for this pipeline.  The `pythread_per_process` scheduler which does essentially what it says: it creates a Python thread for every process in the pipeline:

	pipeline_runner -S pythread_per_process -p </path/to/kwiver/source>/kwiver/pipeline_configs>/number_flow_python.pipe

As with the previous pipeline, the numbers will be written to an output file, this time `numbers_from_python.txt`



# vital #

Vital is an open source C++ collection of libraries and tools that supply basic types and services to the Kitware KWIVER imagery tool kit.

## Overview of Directories ##


* CMake -- contains CMake helper scripts
* tests -- contains testing related support code
* vital -- contains the core library source and headers
* vital/algo -- contains abstract algorithm definitions
* vital/bindings -- contains C and Python bindings
* vital/config -- contains configuration support code
* vital/exceptions -- contains the exception class hierarchy
* vital/io -- contains the classes that support reading and writing core data types
* vital/kwiversys -- contains the code that supports the OS abstraction layer
* vital/logger -- contains the classes that provide logging support
* vital/tests -- contains the main testing code
* vital/tools -- contains source for command line utilities
* vital/types -- contains the source for the core data types


# Running CMake #

We recommend building KWIVER outside of the source directory to prevent
mixing source files with compiled products.  Create a build directory
in parallel with the kwiver source directory.  From the command line,
enter the empty build directory and run

    $ ccmake /path/to/kwiver/source

where the path above is the location of your kwiver source tree.  The
ccmake tool allows for interactive selection of CMake options.
Alternatively, using the CMake GUI you can set the source and build
directories accordingly and press the "Configure" button.


# CMake Options #

* CMAKE_BUILD_TYPE -- The compiler mode, usually Debug or Release
* CMAKE_INSTALL_PREFIX -- The path to where you want the kwiver build products to install
* KWIVER_BUILD_SHARED -- Build shared or static libraries
* KWIVER_ENABLE_DOCS -- Turn on building the Doxygen documentation
* KWIVER_ENABLE_LOG4CXX -- Enable log4cxx logger back end
* KWIVER_ENABLE_PYTHON -- Enable the python bindings
* KWIVER_ENABLE_TESTS -- Build the unit tests
* KWIVER_USE_BUILD_TREE -- When building the plugin manager, whether to include the build directory in the search path.
* KWIVER_ENABLE_C_BINDINGS -- Whether to build the C bindings
* fletch_DIR -- Build directory for the Fletch support packages.

There are many more options

## Dependencies ##

Vital has minimal required dependencies. Sprokit pipeline framework
relies on boost.  Arrows and sprokit processes are structured so that
the code that depends on an external package is in a directory with
the major dependency name (e.g. vxl, ocv). The dependencies can be
turned ON or OFF through CMake variables.

### Required ##

All dependencies are supplied by the Fletch package of 3rd party dependencies.

[Eigen](http://eigen.tuxfamily.org/) (>= 3.0)
[log4cxx] (https://logging.apache.org/log4cxx/) (>= 0.10.0)
[Apache Runtime] (https://apr.apache.org/)

# Development #

Branches that are directly public releasable start with the 'dev/' prefix and those that need
public release approval start with 'kw/'.

When developing on vital, please keep to the prevailing style of the code.
Some guidelines to keep in mind for different languages in the codebase are as
follows:

## CMake ##

  * 2-space indentation
  * Lowercase for private variables
  * Uppercase for user-controlled variables
  * Prefer functions over macros
    - They have variable scoping and debugging them is much easier
  * Prefer ``foreach (IN LISTS)`` and ``list(APPEND)``
  * Prefer ``kwiver_configure_file`` over ``configure_file`` when possible to
    avoid adding dependencies to the configure step
  * Use the ``kwiver_`` wrappers of common commands (e.g., ``add_library``,
    ``add_test``, etc.) as they automatically Do The Right Thing with
    installation, compile flags, build locations, and more)
  * Quote *all* paths and variable expansions unless list expansion is required
    (usually in command arguments or optional arguments)

## C++ ##

  * 2-space indentation
  * Use lowercase with underscores for symbol names
  * Store intermediate values into local ``const`` variables so that they are
    easily available when debugging
  * There is no fixed line length, but keep it reasonable
  * Default to using ``const`` everywhere
  * All modifiers of a type go *after* the type (e.g., ``char const*``, not
    ``const char*``)
  * Export symbols (or import them if possible)
  * Use braces around all control (even single-line if) blocks
  * Use typedefs
  * Use exceptions and return values, not error codes and output parameters
    - This allows for chaining functions, works with ``<algorithm>`` better,
      and allows more variables to be ``const``

## Python ##

  * Follow PEP8
  * When catching exceptions, catch the type then use ``sys.exc_info()`` so
    that it works in Python versions from 2.4 to 3.3
  * No metaclasses; they don't work with the same syntax in Python2 and Python3
  * Avoid 'with' since it doesn't work in Python 2.4

## Testing ##

Generally, all new code should come with tests. The goal is sustained
95% coverage and higher (due to impossible-to-generically-create
corner cases such as files which are readable, but error out in the
middle). Tests should be grouped into a single executable for each
class, group of cooperating classes (e.g., types tests), or
higher-level use case. In C++, use the ``TEST_`` macros which will
hook into the testing infrastructure automatically and in Python, name
functions so that they start with ``test_`` and they will be picked up
automatically.
