// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/geocalc/algo/derive_corner_points.h>
#include <arrows/geocalc/geo_conv.h>
#include <arrows/geocalc/kwiver_algo_geocalc_plugin_export.h>

#include <vital/plugin_management/plugin_factory.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/geodesy.h>

namespace kwiver {

namespace arrows {

namespace geocalc {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_GEOCALC_PLUGIN_EXPORT
void
register_factories( vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpl.add_factory< vital::algo::metadata_filter,
    derive_corner_points >( "derive_corner_points" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.geocalc" );
}

}   // end namespace proj

}   // end namespace arrows

}   // end namespace kwiver
