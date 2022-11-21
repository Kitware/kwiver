// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef ARROWS_SERIALIZATION_PROTO_DETECTED_OBJECT_TYPE_H
#define ARROWS_SERIALIZATION_PROTO_DETECTED_OBJECT_TYPE_H

#include <arrows/serialize/protobuf/kwiver_serialize_protobuf_export.h>
#include <vital/algo/data_serializer.h>
namespace kwiver {
namespace arrows {
namespace serialize {
namespace protobuf {

class KWIVER_SERIALIZE_PROTOBUF_EXPORT detected_object_type
  : public vital::algo::data_serializer
{
public:
  PLUGIN_INFO( "kwiver:detected_object_type",
               "Serializes a detected_object_type using protobuf notation. " );

  detected_object_type();
  virtual ~detected_object_type();

  std::shared_ptr< std::string > serialize( const std::any& element ) override;
  std::any deserialize( const std::string& message ) override;
};

} } } }       // end namespace kwiver

#endif // ARROWS_SERIALIZATION_PROTO_DETECTED_OBJECT_TYPEH
