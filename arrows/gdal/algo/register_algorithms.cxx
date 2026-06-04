// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief GDAL algorithm registration implementation

#include <arrows/gdal/kwiver_algo_gdal_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/gdal/algo/image_io.h>

namespace kwiver {

namespace arrows {

namespace gdal {

extern "C"
KWIVER_ALGO_GDAL_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact =
    vpm.add_factory< vital::algo::image_io, image_io >( "gdal" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.gdal" );
}

} // end namespace gdal

} // end namespace arrows

} // end namespace kwiver
