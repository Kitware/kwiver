/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ARROWS_SERIALIZATION_PROTOBUF_TIMESTAMP_H
#define ARROWS_SERIALIZATION_PROTOBUF_TIMESTAMP_H

#include <arrows/serialize/protobuf/kwiver_serialize_protobuf_export.h>
#include <vital/algo/data_serializer.h>
#include <vital/types/timestamp.h>
#include <vital/types/protobuf/timestamp.pb.h>

namespace kwiver {
namespace arrows {
namespace serialize {
namespace protobuf {

class KWIVER_SERIALIZE_PROTOBUF_EXPORT timestamp
  : public vital::algorithm_impl< timestamp, vital::algo::data_serializer >
{
public:
  static constexpr char const* name = "kwiver:timestamp";
  static constexpr char const* description =
    "Serializes a timestamp using protobuf notation.";

  timestamp();
  virtual ~timestamp();

  virtual std::shared_ptr< std::string > serialize( const vital::any& element ) override;
  virtual vital::any deserialize( const std::string& message ) override;

  static void convert_protobuf( const kwiver::protobuf::timestamp&  proto_tstamp,
                                kwiver::vital::timestamp&           tstamp );

  static void convert_protobuf( const kwiver::vital::timestamp& tstamp,
                                kwiver::protobuf::timestamp&    proto_tstamp );
};

} } } }

#endif // ARROWS_SERIALIZATION_PROTOBUF_TIMESTAMP_H
