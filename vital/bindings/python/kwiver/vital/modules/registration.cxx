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

#include <vital/bindings/python/kwiver/vital/modules/modules_python_export.h>
#include <vital/bindings/python/kwiver/vital/util/python_exceptions.h>
#include <vital/bindings/python/kwiver/vital/util/pybind11.h>
#include <kwiversys/SystemTools.hxx>
#include <pybind11/stl.h>
#include <vital/plugin_loader/plugin_loader.h>
#include "module_helpers.h"

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

static bool is_suppressed();
static void load_python_modules();
static void load_additional_cpp_modules(kwiver::vital::plugin_loader& vpm);

namespace py = pybind11;

extern "C"
MODULES_PYTHON_EXPORT
void
register_factories(kwiver::vital::plugin_loader& vpm)
{
  if (is_suppressed())
  {
    return;
  }
  check_and_initialize_python_interpretor();
  std::string python_library_path = "";
  {
    kwiver::vital::python::gil_scoped_acquire acquire;
    (void)acquire;
    python_library_path = find_python_library();
  }
  load_python_library_symbols(python_library_path);

  // Load python modules
  {
    kwiver::vital::python::gil_scoped_acquire acquire;
    (void)acquire;
    VITAL_PYTHON_IGNORE_EXCEPTION(load_python_modules());
  }
  // Load cpp modules
  {
    kwiver::vital::python::gil_scoped_acquire acquire;
    (void)acquire;
    VITAL_PYTHON_IGNORE_EXCEPTION(load_additional_cpp_modules(vpm));
  }
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
  auto logger = kwiver::vital::get_logger("vital.load_addtional_cpp_modules");
  py::object const modules =  py::module::import("kwiver.vital.modules.module_loader");
  py::object const get_cpp_paths_from_entrypoints = modules.attr("get_cpp_paths_from_entrypoints");
  py::object py_additional_cpp_paths = get_cpp_paths_from_entrypoints();
  auto additional_cpp_paths = py_additional_cpp_paths.cast<std::vector<std::string>>();
  for(auto additional_cpp_path : additional_cpp_paths)
  {
    LOG_INFO(logger,"Addtional_cpp_path: " << additional_cpp_path.c_str());
  }
  vpm.load_plugins(additional_cpp_paths);
}
