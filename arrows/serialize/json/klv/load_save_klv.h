// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_SERIALIZATION_JSON_KLV_LOAD_SAVE_H_
#define KWIVER_ARROWS_SERIALIZATION_JSON_KLV_LOAD_SAVE_H_

#include <arrows/serialize/json/klv/kwiver_serialize_json_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

namespace cereal {

class JSONOutputArchive;
class JSONInputArchive;

KWIVER_SERIALIZE_JSON_KLV_EXPORT
void save( JSONOutputArchive& archive,
           kwiver::arrows::klv::klv_packet const& packet );
KWIVER_SERIALIZE_JSON_KLV_EXPORT
void load( JSONInputArchive& archive,
           kwiver::arrows::klv::klv_packet& packet );

KWIVER_SERIALIZE_JSON_KLV_EXPORT
void save( JSONOutputArchive& archive,
           kwiver::arrows::klv::klv_timed_packet const& packet );
KWIVER_SERIALIZE_JSON_KLV_EXPORT
void load( JSONInputArchive& archive,
           kwiver::arrows::klv::klv_timed_packet& packet );

} // namespace cereal

#endif
