// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/serialize/json/klv/kwiver_serialize_json_klv_plugin_export.h>

#include <arrows/serialize/json/klv/algo/metadata_map_io.h>

#include <vital/plugin_management/plugin_manager.h>

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
  auto fact =
    vpm.add_factory< vital::algo::metadata_map_io,
      metadata_map_io_klv >( "klv-json" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.klv" );
}

} // namespace json

} // namespace serialize

} // namespace arrows

} // namespace kwiver
