// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <vital/plugin_management/plugin_manager.h>

namespace py = pybind11;
namespace kv = kwiver::vital;

// Wrap class so destructor is public
// class deletable_plugin_manager : public kv::plugin_manager
// {
// public:
//   ~deletable_plugin_manager() = default;
// };

// ----------------------------------------------------------------------------
PYBIND11_MODULE( _plugin_management, m )
{
  // -------------------------------------------------------------------------
  m.def( "plugin_manager_instance", &kv::plugin_manager::instance,
         py::return_value_policy::reference,
         // doc-string
         "Returns the plugin manager singleton"
         );

  py::enum_<kv::plugin_manager::plugin_type>( m, "PluginType" )
      .value( "PROCESSES", kv::plugin_manager::plugin_type::PROCESSES )
      .value( "ALGORITHMS", kv::plugin_manager::plugin_type::ALGORITHMS )
      .value( "APPLETS", kv::plugin_manager::plugin_type::APPLETS )
      .value( "EXPLORER", kv::plugin_manager::plugin_type::EXPLORER )
      .value( "OTHERS", kv::plugin_manager::plugin_type::OTHERS )
      .value( "LEGACY", kv::plugin_manager::plugin_type::LEGACY )
      .value( "DEFAULT", kv::plugin_manager::plugin_type::DEFAULT )
      .value( "ALL", kv::plugin_manager::plugin_type::ALL )
      .export_values();

// -------------------------------------------------------------------------
  py::class_< kv::plugin_manager, std::unique_ptr< kv::plugin_manager, py::nodelete > >(
    m, "PluginManager",
    // doc-string
    "Main plugin manager for all kwiver components."
    )
    // Use lambda function for now so we don't have to deal bitflags type
    .def( "load_all_plugins", []( kv::plugin_manager &self ) {
            return kv::plugin_manager::instance().load_all_plugins();
          },
          py::doc( "Loads all plugins that can be discovered on the "
                   "currently active search path. " )
    )
    .def( "reload_all_plugins", &kv::plugin_manager::reload_all_plugins,
          py::doc( "Clears the factory list and reloads plugins.")
    );
}