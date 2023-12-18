// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief register core applets into a plugin
 */

#include <arrows/core/applets/kwiver_algo_core_applets_export.h>
#include <vital/plugin_management/plugin_loader.h>

#include <arrows/core/applets/dump_klv.h>
//#include <arrows/core/applets/render_mesh.h>
//#include <arrows/core/applets/transcode.h>

namespace kwiver::arrows::core {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_CORE_APPLETS_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using namespace kwiver::tools;
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact =
    vpl.add_factory< kwiver_applet, dump_klv >( "dump-klv");
  fact->add_attribute( kvpf::PLUGIN_DESCRIPTION,
                       "Kviwer algorithm core applets")
    .add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core_applets" )
    .add_attribute( kvpf::ALGORITHM_CATEGORY, kvpf::APPLET_CATEGORY );

}

} // end namespace
