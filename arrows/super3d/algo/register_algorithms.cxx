// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Register depth algorithms implementation

#include <arrows/super3d/kwiver_algo_super3d_plugin_export.h>
#include <vital/plugin_management/plugin_factory.h>

#include <arrows/super3d/algo/compute_depth.h>

namespace kwiver {

namespace arrows {

namespace super3d {

extern "C"
KWIVER_ALGO_SUPER3D_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpl.add_factory< vital::algo::compute_depth,
    kwiver::arrows::super3d::compute_depth >( "super3d" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.super3d" );
}

} // end namespace super3d

} // end namespace arrows

} // end namespace kwiver
