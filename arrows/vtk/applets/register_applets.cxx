// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief register vtk applets into a plugin

#include <arrows/vtk/applets/kwiver_algo_vtk_applets_export.h>
#include <vital/plugin_management/plugin_loader.h>
#include <vital/applets/applet_registrar.h>

#ifdef VTK_ENABLE_COLOR_MESH
#include <arrows/vtk/applets/color_mesh.h>
#endif
#include <arrows/vtk/applets/estimate_depth.h>
#include <arrows/vtk/applets/fuse_depth.h>

namespace kwiver {
namespace arrows {
namespace vtk {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_VTK_APPLETS_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  kwiver::applet_registrar reg( vpm, "arrows.vtk.applets" );

  if (reg.is_module_loaded())
  {
    return;
  }

  // -- register applets --
#ifdef VTK_ENABLE_COLOR_MESH
  reg.register_tool< color_mesh >();
#endif
  reg.register_tool< estimate_depth >();
  reg.register_tool< fuse_depth >();

  reg.mark_module_as_loaded();
}

} // end namespace vtk
} // end namespace arrows
} // end namespace kwiver
