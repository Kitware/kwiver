/*ckwg +29
 * Copyright 2011-2012 by Kitware, Inc.
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

#include <vital/plugin_loader/plugin_manager.h>

#include <pybind11/pybind11.h>

#include <algorithm>
/**
 * \file module_loader.cxx
 *
 * \brief Python bindings for module loading.
 */

namespace py = pybind11;

namespace kwiver {
namespace vital {
namespace python {

const std::string get_initial_plugin_path()
{
  py::object const initial_plugin_path_module =
    py::module::import("kwiver.vital.util.initial_plugin_path");
  std::string const initial_plugin_path =
    initial_plugin_path_module.attr("get_initial_plugin_path")().cast<std::string>();
  return initial_plugin_path;
}

//@todo Alternative is to provide C bindings for the plugin manager.
void load_known_modules()
{
  const std::string initial_plugin_path =
                kwiver::vital::python::get_initial_plugin_path();
  auto plugin_search_paths =
                kwiver::vital::plugin_manager::instance().search_path();
  if( std::find( plugin_search_paths.begin(),
                 plugin_search_paths.end(),
                 initial_plugin_path ) ==  plugin_search_paths.end() )
  {
    kwiver::vital::plugin_manager::instance().add_search_path( initial_plugin_path );
  }
  kwiver::vital::plugin_manager::instance().load_all_plugins();
}

bool is_module_loaded(std::string module_name)
{
  const std::string initial_plugin_path =
                kwiver::vital::python::get_initial_plugin_path();
  auto plugin_search_paths =
                kwiver::vital::plugin_manager::instance().search_path();
  if( std::find( plugin_search_paths.begin(),
                 plugin_search_paths.end(),
                 initial_plugin_path ) ==  plugin_search_paths.end() )
  {
    kwiver::vital::plugin_manager::instance().add_search_path( initial_plugin_path );
  }
  return kwiver::vital::plugin_manager::instance().is_module_loaded(module_name);
}

PYBIND11_MODULE(modules, m)
{
  m.def("load_known_modules", &kwiver::vital::python::load_known_modules
    , "Loads modules to populate the process and scheduler registries.");
  m.def("is_module_loaded", &kwiver::vital::python::is_module_loaded,
      "Check if a module has been loaded");
}

} } }  // end namespace
