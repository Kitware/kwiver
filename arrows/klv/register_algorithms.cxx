// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Register KLV algorithms implementation
 */

#include <arrows/klv/kwiver_algo_klv_export.h>
#include <arrows/klv/kwiver_algo_klv_plugin_export.h>

#include <vital/algo/algorithm_factory.h>

#include <arrows/klv/convert_metadata.h>

namespace kwiver {
namespace arrows {
namespace klv {

extern "C"
KWIVER_ALGO_KLV_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  ::kwiver::vital::algorithm_registrar reg( vpm, "arrows.klv" );

  if (reg.is_module_loaded())
  {
    return;
  }

  // No algorithms

  reg.mark_module_as_loaded();
}

} // end namespace klv
} // end namespace arrows
} // end namespace kwiver
