// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Default plugin algorithm registration interface implementation.

#include <arrows/serialize/json/kwiver_serialize_json_plugin_export.h>

#include <vital/plugin_management/plugin_manager.h>

#include <arrows/serialize/json/algo/activity.h>
#include <arrows/serialize/json/algo/activity_type.h>
#include <arrows/serialize/json/algo/bounding_box.h>
#include <arrows/serialize/json/algo/detected_object.h>
#include <arrows/serialize/json/algo/detected_object_set.h>
#include <arrows/serialize/json/algo/detected_object_type.h>
#include <arrows/serialize/json/algo/image.h>
#include <arrows/serialize/json/algo/metadata.h>
#include <arrows/serialize/json/algo/metadata_map_io.h>
#include <arrows/serialize/json/algo/object_track_set.h>
#include <arrows/serialize/json/algo/object_track_state.h>
#include <arrows/serialize/json/algo/string.h>
#include <arrows/serialize/json/algo/timestamp.h>
#include <arrows/serialize/json/algo/track.h>
#include <arrows/serialize/json/algo/track_set.h>
#include <arrows/serialize/json/algo/track_state.h>

namespace kwiver {

namespace arrows {

namespace serialize {

namespace json {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_SERIALIZE_JSON_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  using namespace kwiver::arrows::serialize::json;

  auto fact =
    vpm.add_factory< vital::algo::data_serializer,
      activity >( "kwiver:json:activity" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    activity_type >( "kwiver:json:activity_type" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    bounding_box >( "kwiver:json:bounding_box" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    detected_object >( "kwiver:json:detected_object" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    detected_object_set >( "kwiver:json:detected_object_set" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    detected_object_type >( "kwiver:json:detected_object_type" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    image >( "kwiver:json:image" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    image >( "kwiver:json:mask" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    metadata >( "kwiver:json:metadata" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::metadata_map_io,
    metadata_map_io >( "json" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    object_track_set >( "kwiver:json:object_track_set" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    object_track_state >( "kwiver:json:object_track_state" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    string >( "kwiver:json:string" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    timestamp >( "kwiver:json:timestamp" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    track >( "kwiver:json:track" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    track_set >( "kwiver:json:track_set" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    track_state >( "kwiver:json:track_state" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    string >( "kwiver:json:file_name" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    string >( "kwiver:json:image_name" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    string >( "kwiver:json:video_name" );
  fact->add_attribute(
    kwiver::vital::plugin_factory::PLUGIN_MODULE_NAME,
    "arrows.serialize.json" );
}

} // end namespace json

} // end namespace serialize

} // end namespace arrows

} // end namespace kwiver
