// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/serialize/json/klv/kwiver_serialize_json_klv_plugin_export.h>

#include "metadata_map_io.h"

#include <vital/algo/algorithm_factory.h>

namespace kwiver {

namespace arrows {

namespace serialize {

namespace json {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_SERIALIZE_JSON_KLV_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  auto const module_name = std::string{ "arrows.serialize.json.klv" };
  vital::algorithm_registrar algo_registrar( vpm, module_name );

  if( algo_registrar.is_module_loaded() )
  {
    return;
  }

  algo_registrar.register_algorithm< metadata_map_io_klv >();

  algo_registrar.mark_module_as_loaded();
}

} // namespace json

} // namespace serialize

} // namespace arrows

} // namespace kwiver
