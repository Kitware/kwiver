// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief PDAL algorithm registration implementation

#include <arrows/pdal/kwiver_algo_pdal_plugin_export.h>
#include <vital/algo/algorithm_factory.h>

#include <arrows/pdal/algo/pointcloud_io.h>

namespace kwiver {
namespace arrows {
namespace pdal {

extern "C"
KWIVER_ALGO_PDAL_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  kwiver::vital::algorithm_registrar reg( vpm, "arrows.pdal" );

  if (reg.is_module_loaded())
  {
    return;
  }

  reg.register_algorithm< pointcloud_io >();

  reg.mark_module_as_loaded();
}

} // end namespace pdal
} // end namespace arrows
} // end namespace kwiver
