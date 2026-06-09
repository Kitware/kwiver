// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// VTK algorithm registration implementation

#include <arrows/vtk/kwiver_algo_vtk_plugin_export.h>

#include <vital/plugin_management/plugin_manager.h>

#include <vital/algo/mesh_filter.h>

#include <arrows/vtk/algo/decimate_mesh_filter.h>
#include <arrows/vtk/algo/smooth_mesh_filter.h>
#include <arrows/vtk/algo/triangulate_mesh_filter.h>

namespace kwiver {

namespace arrows {

namespace vtk {

extern "C"
KWIVER_ALGO_VTK_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpl.add_factory< vital::algo::mesh_filter,
    smooth_mesh_filter >( "vtk_smooth" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_vtk" );

  fact = vpl.add_factory< vital::algo::mesh_filter,
    decimate_mesh_filter >( "vtk_decimate" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_vtk" );

  fact = vpl.add_factory< vital::algo::mesh_filter,
    triangulate_mesh_filter >( "vtk_triangulate" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_vtk" );
}

} // namespace vtk

} // namespace arrows

} // namespace kwiver
