// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_SERIALIZATION_JSON_KLV_LOAD_SAVE_H_
#define KWIVER_ARROWS_SERIALIZATION_JSON_KLV_LOAD_SAVE_H_

#include <arrows/serialize/json/kwiver_serialize_json_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

namespace cereal {

class JSONOutputArchive;
class JSONInputArchive;

KWIVER_SERIALIZE_JSON_EXPORT
void save( ::cereal::JSONOutputArchive& archive,
           std::vector< ::kwiver::arrows::klv::klv_packet > const& packets );
KWIVER_SERIALIZE_JSON_EXPORT
void load( ::cereal::JSONInputArchive& archive,
           std::vector< ::kwiver::arrows::klv::klv_packet >& packets );

KWIVER_SERIALIZE_JSON_EXPORT
void save( ::cereal::JSONOutputArchive& archive,
           std::vector< ::kwiver::arrows::klv::klv_timed_packet > const&
             timed_packets );
KWIVER_SERIALIZE_JSON_EXPORT
void load( ::cereal::JSONInputArchive& archive,
           std::vector< ::kwiver::arrows::klv::klv_timed_packet >&
             timed_packets );

} // namespace cereal

#endif
