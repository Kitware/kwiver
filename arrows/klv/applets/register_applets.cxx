// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Register KLV applets.

#include <arrows/klv/applets/compare_klv.h>
#include <arrows/klv/applets/kwiver_algo_klv_applets_export.h>

#include <vital/applets/applet_registrar.h>
#include <vital/plugin_management/plugin_loader.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_KLV_APPLETS_EXPORT
void
register_factories( vital::plugin_loader& vpm )
{
  applet_registrar registrar( vpm, "arrows.klv.applets" );

  if( registrar.is_module_loaded() )
  {
    return;
  }

  registrar.register_tool< compare_klv >();

  registrar.mark_module_as_loaded();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
