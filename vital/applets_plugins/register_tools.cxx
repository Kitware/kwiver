// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "vital/applets_plugins/config_explorer_export.h"

#include <vital/plugin_management/plugin_loader.h>

#include "vital/applets_plugins/config_explorer.h"

// ============================================================================
extern "C"
CONFIG_EXPLORER_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using namespace kwiver::tools;
  using kvpf = ::kwiver::vital::plugin_factory;

  vpl.add_factory<kwiver_applet, config_explorer>( "explore-config" )
    ->add_attribute( kvpf::PLUGIN_DESCRIPTION,
                     "Explore configuration loading process.\n\n"
                     "This program assists in debugging config loading "
                     "problems. It loads a configuration and displays the "
                     "contents or displays the search path." )
    .add_attribute( kvpf::PLUGIN_MODULE_NAME, "vital_tool_group" )
    .add_attribute( kvpf::ALGORITHM_CATEGORY, kvpf::APPLET_CATEGORY );

}
