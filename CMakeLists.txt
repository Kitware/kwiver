# ==============================================================================
# Top level build script for kwiver
cmake_minimum_required(VERSION 3.15)

project(KWIVER)

# ==============================================================================
# KWIVER version
file( READ "VERSION" KWIVER_VERSION )
string( STRIP "${KWIVER_VERSION}" KWIVER_VERSION )
string( REGEX MATCH "^([0-9]+)\.([0-9]+)\.([0-9]+)" _ "${KWIVER_VERSION}" )
set( KWIVER_VERSION_MAJOR "${CMAKE_MATCH_1}" )
set( KWIVER_VERSION_MINOR "${CMAKE_MATCH_2}" )
set( KWIVER_VERSION_PATCH "${CMAKE_MATCH_3}" )

# Organize target into folders for IDEs that support it
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ==============================================================================
# Look for and use Fletch to find dependencies
#
set( fletch_DIR "" CACHE PATH "Path to FLETCH packages" )
if ( IS_DIRECTORY ${fletch_DIR} )
  find_package( fletch NO_MODULE )
  if ( fletch_FOUND )
    # Search for libraries in the fletch install root
    list(APPEND CMAKE_PREFIX_PATH "${fletch_ROOT}")
  endif()
endif()

# ==============================================================================
# project specific directories
#
set(KWIVER_CMAKE_DIR            "${CMAKE_SOURCE_DIR}/CMake")

# root directories
set(KWIVER_SOURCE_DIR           "${CMAKE_SOURCE_DIR}")
set(KWIVER_BINARY_DIR           "${CMAKE_BINARY_DIR}")

# Default library dir
set(KWIVER_DEFAULT_LIBRARY_DIR "lib" CACHE STRING
    "Default library directory for kwiver")
mark_as_advanced(KWIVER_DEFAULT_LIBRARY_DIR)

set( LIB_SUFFIX "" CACHE STRING
     "Library directory suffix. e.g. suffix=\"kwiver\" will install libraries in \"libkwiver\" rather than \"lib\"")
mark_as_advanced( LIB_SUFFIX )

# ==============================================================================
# kwiver module config file.
set(KWIVER_CONFIG_FILE          "${KWIVER_BINARY_DIR}/kwiver-config.cmake")

# prepend our CMake resources
list(INSERT CMAKE_MODULE_PATH 0 ${KWIVER_CMAKE_DIR})

# Import other CMake modules that are back-ports from "future" versions.
# "Future" generally meaning versions of CMake that are greater than the one
# currently used.
include(kwiver-cmake-future)

# ==============================================================================
# Project global includes and links
include_directories("${KWIVER_SOURCE_DIR}")
include_directories("${KWIVER_SOURCE_DIR}/sprokit/src")
include_directories("${KWIVER_BINARY_DIR}")
include_directories("${KWIVER_BINARY_DIR}/sprokit/src")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
# include vital internal headers for private use only
include_directories("${KWIVER_SOURCE_DIR}/vital/internal")
# kwiversys includes are generated in the binary directory
include_directories("${KWIVER_BINARY_DIR}/vital/kwiversys")

include_directories( SYSTEM ${CMAKE_CURRENT_SOURCE_DIR} )

link_directories( ${KWIVER_BINARY_DIR}/lib )

# ==============================================================================
# User options
include(CMakeDependentOption)

# TODO: Add a comment documenting why we have a separate build-shared flag.
OPTION(KWIVER_BUILD_SHARED     "Build KWIVER components shared or not" TRUE )
MARK_AS_ADVANCED( KWIVER_BUILD_SHARED )
set(BUILD_SHARED_LIBS ${KWIVER_BUILD_SHARED})

OPTION(KWIVER_EXPORT_COMPILE_COMMANDS "Export compile commands to support LSP development." FALSE )
MARK_AS_ADVANCED( KWIVER_EXPORT_COMPILE_COMMANDS )
if (KWIVER_EXPORT_COMPILE_COMMANDS )
  set(CMAKE_EXPORT_COMPILE_COMMANDS ${KWIVER_EXPORT_COMPILE_COMMANDS})
endif()

# TODO: Enable-* flags will need clean-up with reorganization.
OPTION(KWIVER_ENABLE_TESTS                "Enable testing" OFF )
OPTION(KWIVER_ENABLE_ARROWS               "Enable building arrows" OFF )
OPTION(KWIVER_ENABLE_TOOLS                "Enable tools" OFF )
OPTION(KWIVER_ENABLE_TRACK_ORACLE         "Enable track_oracle" OFF )
OPTION(KWIVER_ENABLE_EXAMPLES             "Enable examples" OFF )

OPTION(KWIVER_ENABLE_SERIALIZE_PROTOBUF   "Enable protobuf serialization" OFF )
OPTION(KWIVER_ENABLE_SERIALIZE_JSON       "Enable json serialization" OFF )

# if sprokit enabled
OPTION(KWIVER_ENABLE_SPROKIT              "Enable building sprokit" OFF )

if(KWIVER_ENABLE_ARROWS)
  OPTION(KWIVER_ENABLE_ARROW_MVG          "Enable Multi-View Geometry Arrow" ON )
  OPTION(KWIVER_ENABLE_ARROW_KLV          "Enable Key-Length-Value Metadata Arrow" ON)
endif()

CMAKE_DEPENDENT_OPTION(KWIVER_ENABLE_PROCESSES
  "Should the KWIVER Sprokit Processes Be Built?" OFF
  "KWIVER_ENABLE_ARROWS AND KWIVER_ENABLE_SPROKIT" OFF )

OPTION(KWIVER_USE_BUILD_TREE "Include build-tree plugin directory to the default plugin path list" OFF )
MARK_AS_ADVANCED(KWIVER_USE_BUILD_TREE)

option( KWIVER_ENABLE_EXTRAS "Enable extras content" OFF )

OPTION( KWIVER_ENABLE_PYTHON "Enable python code" OFF )
if (KWIVER_ENABLE_PYTHON)
  if (NOT WIN32)
    OPTION(KWIVER_SYMLINK_PYTHON "Symlink python files instead of copying." OFF)
  endif()
endif()

if (KWIVER_ENABLE_SERIALIZE_PROTOBUF)
  if (WIN32)
    message( WARNING "Protobuf serialization not supported for windows. Being disabled." )
    set(KWIVER_ENABLE_SERIALIZE_PROTOBUF OFF )
  endif()
endif()

# ==============================================================================
# KWIVER CMake config
#
include( kwiver-utils )

# Set directories where loadable modules are stored.
# These subdirs are under .../lib/
set( kwiver_plugin_subdir                         kwiver/plugins )
set( kwiver_plugin_process_subdir                 ${kwiver_plugin_subdir}/processes )
set( kwiver_plugin_algorithm_subdir               ${kwiver_plugin_subdir}/algorithms )
set( kwiver_plugin_process_instrumentation_subdir ${kwiver_plugin_subdir}/processes )
set( kwiver_plugin_scheduler_subdir               ${kwiver_plugin_subdir}/processes )
set( kwiver_plugin_module_subdir                  ${kwiver_plugin_subdir}/modules )
set( kwiver_plugin_plugin_explorer_subdir         ${kwiver_plugin_subdir}/plugin_explorer )
set( kwiver_plugin_logger_subdir                  ${kwiver_plugin_subdir}/logger )
set( kwiver_plugin_applets_subdir                 ${kwiver_plugin_subdir}/applets )

# other well known subdirs
set( kwiver_config_subdir                         share/kwiver/${KWIVER_VERSION}/config)

add_definitions(-DKWIVER_DEFAULT_PLUGIN_ORGANIZATION="Kitware Inc.")

# ==============================================================================
# System specific compiler flags and support
include( kwiver-flags )
include( kwiver-configcheck )

# set the name for our package export
set( kwiver_export_name      kwiver_exports )

# ==============================================================================
# External Dependencies and Modules - Most Will be supplied by Fletch
include( kwiver-depends )
# TODO: devise something more clear than this pattern: dependencies are too far
#   detached from what requires them. This makes it very unclear what depends
#   on what. Easy to create wild global flags.

# ==============================================================================
# Configure setup scripts
include( kwiver-setup-scripts )

if(MSVC)
  # Generate files to inform msvc of the kwiver runtime environment.
  # The "list" of batch files for use in setup is defined in
  # "kwiver-setup-scripts" (`SETUP_BATCH_FILES`).
  kwiver_setup_msvc_env() # Setup .bat scripts need to be complete and on disk
endif()

# ==============================================================================
# Add tests if enabled
if (KWIVER_ENABLE_TESTS)
  enable_testing()
  include(CTest)
  set(BUILD_TESTING    ON)  # needed for compatibility with kwiversys
  mark_as_advanced(BUILD_TESTING)

  add_subdirectory( tests )
  include_directories( tests )
endif()

if (NOT IS_ABSOLUTE "${CMAKE_INSTALL_PREFIX}")
  message(FATAL_ERROR "KWIVER does not support a relative install prefix "
    "(${CMAKE_INSTALL_PREFIX}).")
endif ()

# ==============================================================================
# Descend into project directories noq.
add_subdirectory( vital )

#if (KWIVER_ENABLE_TRACK_ORACLE)
#  add_subdirectory( track_oracle )
#endif()
#
if (KWIVER_ENABLE_ARROWS)
  add_subdirectory( arrows )
endif()
#
#if (KWIVER_ENABLE_SPROKIT)
#  add_subdirectory(sprokit)
#endif()
#
#if (KWIVER_ENABLE_EXTRAS)
#  add_subdirectory(extras)
#endif()
#
#if (KWIVER_ENABLE_EXAMPLES)
#  add_subdirectory(examples)
#endif()
#
#if (KWIVER_ENABLE_TOOLS)
#  add_subdirectory(tools)
#endif()
add_subdirectory(tools)

if (KWIVER_ENABLE_PYTHON)
  add_subdirectory( python/kwiver )
endif()

## this is where the algorithm default configuration files live
#add_subdirectory(config)


# ==============================================================================
# Marking source root for documentation generation
#
kwiver_create_doxygen( kwiver "${CMAKE_CURRENT_LIST_DIR}" )
kwiver_create_sphinx()


# ==============================================================================
# Top level installation
#
set(kwiver_cmake_install_dir lib${LIB_SUFFIX}/cmake/kwiver)

# Install rules for CMake utilities
include( kwiver-install-utils )

# Prepare space-separated list of library names for config
get_property(kwiver_libs GLOBAL PROPERTY kwiver_libraries)
string(REPLACE ";" " " kwiver_libs "${kwiver_libs}")

# setup general paths to includes and libs for packages we use/provide
set( KWIVER_INCLUDE_DIRS "")
set( KWIVER_LIBRARY_DIRS "")

# TODO: use global property to capture found, external-to-fletch eigen dep
if ( NOT fletch_ENABLED_Eigen )
  list(APPEND KWIVER_INCLUDE_DIRS "${EIGEN3_INCLUDE_DIR}")
endif ()

# TODO: use global property to capture found, external-to-fletch boost dep
if ( NOT fletch_ENABLED_Boost)
  # define which boost we are using
  if (DEFINED BOOST_ROOT)
    set(BOOST_ROOT_ARG ${BOOST_ROOT})
  elseif (DEFINED Boost_LIBRARY_DIR)
    set(BOOST_ROOT_ARG ${Boost_LIBRARY_DIR}/..)
  endif()

  if (DEFINED BOOST_ROOT_ARG)
    list( APPEND KWIVER_INCLUDE_DIRS "${BOOST_ROOT_ARG}/include")
    list( APPEND KWIVER_LIBRARY_DIRS "${BOOST_ROOT_ARG}/lib")
  endif()
endif()

if (fletch_FOUND)
  # since we have fletch, add include and lib dirs to export
  list( APPEND KWIVER_INCLUDE_DIRS "${fletch_ROOT}/include")
  list( APPEND KWIVER_LIBRARY_DIRS "${fletch_ROOT}/lib")
endif()

configure_file(
  "${KWIVER_SOURCE_DIR}/CMake/kwiver-config-build.cmake.in"
  "${KWIVER_CONFIG_FILE}"
  @ONLY
  )

# Configure install-tree CMake config file and export associated targets file
set(KWIVER_CONFIG_INSTALL_FILE "${KWIVER_BINARY_DIR}/kwiver-config-install.cmake")
configure_file(
  "${KWIVER_SOURCE_DIR}/CMake/kwiver-config-install.cmake.in"
  "${KWIVER_CONFIG_INSTALL_FILE}"
  @ONLY
  )

kwiver_export_targets("${KWIVER_BINARY_DIR}/kwiver-config-targets.cmake")

if(NOT SKBUILD)
  kwiver_install(
    FILES       "${KWIVER_CONFIG_INSTALL_FILE}"
    DESTINATION "${kwiver_cmake_install_dir}"
    RENAME      kwiver-config.cmake
    )

  kwiver_install(
    EXPORT      ${kwiver_export_name}
    NAMESPACE   kwiver::
    DESTINATION "${kwiver_cmake_install_dir}"
    FILE        kwiver-config-targets.cmake
    )
endif()

# ==============================================================================
# link compile commands to root of source tree
if ( CMAKE_EXPORT_COMPILE_COMMANDS )
  # link file to root of source tree
  kwiver_make_symlink( ${CMAKE_BINARY_DIR}/compile_commands.json
                       ${CMAKE_SOURCE_DIR}/compile_commands.json )
endif()

# ==============================================================================
# Configure header file defining plugin path defaults.
#
# Build a default set of plugin path dirs
# Provide that list as the default value for the path option.
#
# By this point, all eligible paths should have been added to the kwiver_plugin_path
# property via calls to the `kwiver_add_module_path` cmake function.
#
# This is here at the project level because it is currently assumed that more
# than VITAL will add paths.
#
# TODO: Revisit the above assumption.
#       If no longer true, this may be moved into the vital sub-project.
#
if (WIN32)
  set(path_sep "\\073")
else()  # Other Unix systems
  set(path_sep "\\072")
endif()

set(KWIVER_DEFAULT_MODULE_PATHS ""
  CACHE STRING "The default paths for module scanning. Separate paths with ';' character." FORCE)
mark_as_advanced( KWIVER_DEFAULT_MODULE_PATHS )

# add all user supplied paths to the property
foreach( p IN LISTS KWIVER_DEFAULT_MODULE_PATHS )
  kwiver_add_module_path( ${p} )
endforeach(p)

kwiver_make_module_path( ${CMAKE_INSTALL_PREFIX} ${kwiver_plugin_subdir} )
kwiver_add_module_path(  "${kwiver_module_path_result}" )

set(sprokit_default_pipe_include_paths
  "${CMAKE_INSTALL_PREFIX}/${kwiver_config_subdir}/pipelines/include")

set(sprokit_default_cluster_paths
  "${CMAKE_INSTALL_PREFIX}/${kwiver_config_subdir}/pipelines/clusters")

if (KWIVER_USE_BUILD_TREE)
  kwiver_make_module_path( ${KWIVER_BINARY_DIR} ${kwiver_plugin_subdir} )
  kwiver_add_module_path(  "${kwiver_module_path_result}" )

  set(sprokit_default_pipe_include_paths
    "${sprokit_default_pipe_include_paths}${path_sep}${KWIVER_SOURCE_DIR}/config/pipelines")

  set(sprokit_default_cluster_paths
    "${sprokit_default_cluster_paths}${path_sep}${KWIVER_SOURCE_DIR}/config/clusters")
endif()

set(SPROKIT_DEFAULT_PIPE_INCLUDE_PATHS "${sprokit_default_pipe_include_paths}"
  CACHE STRING "The default paths to search for pipe includes in" FORCE)

set(SPROKIT_DEFAULT_CLUSTER_PATHS "${sprokit_default_cluster_paths}"
  CACHE STRING "The default paths to search for clusters in" FORCE)

# need to retrieve the GLOBAL PROPERTY kwiver_plugin_path and
# formulate the default module path
get_property(plugin_path GLOBAL PROPERTY kwiver_plugin_path)

# add all user configured paths to our list
foreach( p IN LISTS plugin_path )
  if ( VITAL_MODULE_PATH )
    set( VITAL_MODULE_PATH "${VITAL_MODULE_PATH}${path_sep}${p}" )
  else()
    set( VITAL_MODULE_PATH "${p}" )
  endif()
endforeach(p)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vital/kwiver-include-paths.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vital/kwiver-include-paths.h" )
# END -- possible move to vital sub-directory
