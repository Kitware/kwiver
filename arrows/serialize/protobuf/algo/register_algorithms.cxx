// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Defaults plugin algorithm registration interface impl

#include <arrows/serialize/protobuf/kwiver_serialize_protobuf_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/serialize/protobuf/algo/activity.h>
#include <arrows/serialize/protobuf/algo/activity_type.h>
#include <arrows/serialize/protobuf/algo/bounding_box.h>
#include <arrows/serialize/protobuf/algo/detected_object.h>
#include <arrows/serialize/protobuf/algo/detected_object_set.h>
#include <arrows/serialize/protobuf/algo/detected_object_type.h>
#include <arrows/serialize/protobuf/algo/image.h>
#include <arrows/serialize/protobuf/algo/metadata.h>
#include <arrows/serialize/protobuf/algo/object_track_set.h>
#include <arrows/serialize/protobuf/algo/object_track_state.h>
#include <arrows/serialize/protobuf/algo/string.h>
#include <arrows/serialize/protobuf/algo/timestamp.h>
#include <arrows/serialize/protobuf/algo/track.h>
#include <arrows/serialize/protobuf/algo/track_set.h>
#include <arrows/serialize/protobuf/algo/track_state.h>

namespace kwiver {

namespace arrows {

namespace serialize {

namespace protobuf {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_SERIALIZE_PROTOBUF_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::activity >(
                "kwiver:protobuf:activity" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::activity_type >(
           "kwiver:protobuf:activity_type" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::bounding_box >(
           "kwiver:protobuf:bounding_box" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::detected_object >(
           "kwiver:protobuf:detected_object" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::detected_object_set >(
           "kwiver:protobuf:detected_object_set" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::detected_object_type >(
           "kwiver:protobuf:detected_object_type" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::image >( "kwiver:protobuf:image" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::image >( "kwiver:protobuf:mask" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::metadata >(
           "kwiver:protobuf:metadata" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::object_track_set >(
           "kwiver:protobuf:object_track_set" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::object_track_state >(
           "kwiver:protobuf:object_track_state" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::string >( "kwiver:protobuf:string" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::string >(
           "kwiver:protobuf:file_name" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::string >(
           "kwiver:protobuf:image_name" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::string >(
           "kwiver:protobuf:video_name" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::timestamp >(
           "kwiver:protobuf:timestamp" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::track >( "kwiver:protobuf:track" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::track_state >(
           "kwiver:protobuf:track_state" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );

  fact = vpm.add_factory< vital::algo::data_serializer,
    kwiver::arrows::serialize::protobuf::track_set >(
           "kwiver:protobuf:track_set" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.serialize.protobuf" );
}

} // end namespace protobuf

} // end namespace serialize

} // end namespace arrows

} // end namespace kwiver
