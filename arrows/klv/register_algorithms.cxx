// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Register KLV algorithm implementations.

#include <vital/algo/algorithm_factory.h>

#include <arrows/klv/kwiver_algo_klv_plugin_export.h>
#include <arrows/klv/apply_child_klv.h>
#include <arrows/klv/update_klv.h>

namespace kwiver {

namespace arrows {

namespace klv {

extern "C"
KWIVER_ALGO_KLV_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  ::kwiver::vital::algorithm_registrar reg( vpm, "arrows.klv" );

  if( reg.is_module_loaded() )
  {
    return;
  }

  reg.register_algorithm< apply_child_klv >();
  reg.register_algorithm< update_klv >();

  reg.mark_module_as_loaded();
}

} // end namespace ffmpeg

} // end namespace arrows

} // end namespace kwiver
