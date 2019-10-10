/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <python/kwiver/vital/modules/modules_python_export.h>

#if WIN32
#pragma warning (push)
#pragma warning (disable : 4244)
#endif
#include <pybind11/pybind11.h>
#if WIN32
#pragma warning (pop)
#endif

#include <pybind11/stl.h>
#include <python/kwiver/vital/util/pybind11.h>
#include <python/kwiver/vital/util/python_exceptions.h>
#include <python/kwiver/vital/util/python.h>

#include <vital/plugin_loader/plugin_loader.h>
#include <kwiversys/SystemTools.hxx>
#include <string>

#ifdef VITAL_LOAD_PYLIB_SYM
  #include <dlfcn.h>
#endif

// Undefine macros that will double expand in case an definition has a value
// something like: /usr/lib/x86_64-linux-gnu/libpython2.7.so
#ifdef linux
#define _orig_linux linux
#undef linux
#endif

// for getting the value of a macro as a string literal
#define MACRO_STR_VALUE(x) _TO_STRING0(x)
#define _TO_STRING0(x) _TO_STRING1(x)
#define _TO_STRING1(x) #x

namespace py = pybind11;

static void load_python_modules();
static bool is_suppressed();
static void _load_python_library_symbols(const std::string python_library_path);
static std::string _find_python_library();
static void load_additional_cpp_modules(kwiver::vital::plugin_loader& vpm);
// ==================================================================
/**
 * @brief Python module loader.
 *
 * This function is called by the plugin loader when it is scanning
 * all plugins. It looks like a standard registration entry point for
 * a set or processes, but it activates the python interpreter and
 * causes it to call vital.modules.module_loader.load_python_modules().
 * Addtionally for the python package of kwiver it is used to register external
 * c++ plugins by specifying a search paths for the plugins
 * Also note that setting the environment variable
 * VITAL_NO_PYTHON_MODULES will suppress loading all python modules.
 */

extern "C"
MODULES_PYTHON_EXPORT
void
register_factories(kwiver::vital::plugin_loader& vpm)
{
  if (is_suppressed())
  {
    return;
  }

  // Check if a python interpreter already exists so we don't clobber sys.argv
  // (e.g. if sprokit is initialized from python)
  if (!Py_IsInitialized())
  {
    // Embed a python interpretter if one does not exist
    Py_Initialize();

    // Set Python interpeter attribute: sys.argv = []
    // parameters are: (argc, argv, updatepath)
    PySys_SetArgvEx(0, NULL, 0);
  }

  if (!PyEval_ThreadsInitialized())
  {
    {
      // Let pybind11 initialize threads and set up its internal data structures
      pybind11::detail::get_internals();
    }
    // Release the GIL
    PyEval_SaveThread();
  }

  std::string python_library_path = "";
  {
    kwiver::vital::python::gil_scoped_acquire acquire;
    (void)acquire;
    python_library_path = _find_python_library();
  }
  _load_python_library_symbols(python_library_path);

  // Load python modules
  {
    kwiver::vital::python::gil_scoped_acquire acquire;
    (void)acquire;
    VITAL_PYTHON_IGNORE_EXCEPTION(load_python_modules())
  }

  {
    kwiver::vital::python::gil_scoped_acquire acquire;
    (void)acquire;
    VITAL_PYTHON_IGNORE_EXCEPTION(load_additional_cpp_modules(vpm))
  }
}

// ------------------------------------------------------------------
/*
 * Uses environment variables and compiler definitions to determine where the
 * python shared library is and load its symbols.
 */
void _load_python_library_symbols(const std::string python_library_path)
{
  auto logger = kwiver::vital::get_logger("vital.python_modules");
  if (python_library_path.empty())
  {
    #ifdef VITAL_LOAD_PYLIB_SYM
    const char *env_pylib = kwiversys::SystemTools::GetEnv( "PYTHON_LIBRARY" );

    // cmake should provide this definition
    #ifdef PYTHON_LIBRARY
    const char *default_pylib = MACRO_STR_VALUE(PYTHON_LIBRARY);
    #else
    const char *default_pylib = NULL;
    #endif

    // First check if the PYTHON_LIBRARY environment variable is specified
    if( env_pylib )
    {
      LOG_DEBUG(logger, "Loading symbols from PYTHON_LIBRARY=" << env_pylib );
      void* handle = dlopen( env_pylib, RTLD_LAZY | RTLD_GLOBAL );
      if (!handle) {
        LOG_ERROR(logger, "Cannot load library: " << dlerror());
      }
    }
    else if( default_pylib )
    {
      // If the PYTHON_LIBRARY environment variable is not specified, use the
      // CMAKE definition of PYTHON_LIBRARY instead.
      LOG_DEBUG(logger, "Loading symbols from default PYTHON_LIBRARY=" << default_pylib);
      void* handle = dlopen( default_pylib, RTLD_LAZY | RTLD_GLOBAL );
      if (!handle) {
        LOG_ERROR(logger, "Cannot load library: " << dlerror());
      }
    }
    else
    {
      LOG_DEBUG(logger, "Unable to pre-load python symbols because " <<
                        "PYTHON_LIBRARY is undefined.");
    }
    #else
      LOG_DEBUG(logger, "Not checking for python symbols");
    #endif
  }
  else
  {
    LOG_DEBUG(logger, "Loading symbols from PYTHON_LIBRARY=" << python_library_path.c_str() );
    void *handle = dlopen( python_library_path.c_str(), RTLD_LAZY | RTLD_GLOBAL );
    if (!handle) {
      LOG_ERROR(logger, "Cannot load library: " << dlerror());
    }
  }
}

std::string
_find_python_library()
{
  py::object const module = py::module::import("kwiver.vital.util.find_python_library");
  py::object const python_library_path = module.attr("find_python_library")();
  return python_library_path.cast<std::string>();
}

// ------------------------------------------------------------------
void
load_python_modules()
{
  py::object const modules = py::module::import("kwiver.vital.modules.module_loader");
  py::object const loader = modules.attr("load_python_modules");
  loader();
}

// -------------------------------------------------------------------
void
load_additional_cpp_modules(kwiver::vital::plugin_loader& vpm)
{
  auto logger = kwiver::vital::get_logger("vital.load_additional_cpp_paths");
  py::object const modules =  py::module::import("kwiver.vital.util.entrypoint");
  py::object const get_cpp_paths_from_entrypoint = modules.attr("get_cpp_paths_from_entrypoint");
  py::object py_additional_paths = get_cpp_paths_from_entrypoint();
  auto additional_paths = py_additional_paths.cast<std::vector<std::string>>();
  for(auto& additional_path : additional_paths)
  {
    LOG_INFO( logger, "loading additional cpp plugins from" + additional_path );
  }
  vpm.load_plugins(additional_paths);
}

// ------------------------------------------------------------------
bool
is_suppressed()
{
  const char * python_suppress = kwiversys::SystemTools::GetEnv( "SPROKIT_NO_PYTHON_MODULES" );
  bool suppress_python_modules = false;

  if (python_suppress)
  {
    suppress_python_modules = true;
  }

  return suppress_python_modules;
}


// Redefine values that we hacked away
#ifdef _orig_linux
#define linux _orig_linux
#undef _orig_linux
#endif
