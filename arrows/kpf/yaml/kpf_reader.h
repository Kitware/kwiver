/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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

/**
 * @file
 * @brief Base class for KPF readers.
 *
 * The KPF reader maintains the packet buffer of KPF packets.
 *
 * The general workflow is:
 *
 * 1) A reader object pulls for a packet of a particular style and domain
 *    (say, style ID and domain 2.)
 *
 * 2) If the packet buffer is empty, call the parser's parse_next_record()
 *    method to refill the packet buffer.
 *
 * 3) If the packet exists, copy it into the bounce buffer (if it's a simple
 *    packet) or into the io_adapter_base (if it's complex.)
 *
 * When the client is done for this record, call flush() to empty the packet
 * buffer and trigger another parse_next_record() call.
 *
 */

#ifndef KWIVER_VITAL_KPF_READER_H_
#define KWIVER_VITAL_KPF_READER_H_

#include <arrows/kpf/yaml/kpf_packet.h>
#include <arrows/kpf/yaml/kpf_canonical_io.h>
#include <arrows/kpf/yaml/kpf_parse_utils.h>
#include <arrows/kpf/yaml/kpf_parser_base.h>
#include <arrows/kpf/yaml/kpf_bounce_buffer.h>
#include <arrows/kpf/yaml/kpf_canonical_io_adapter_base.h>

#include <utility>
#include <iostream>
#include <map>
#include <sstream>

namespace kwiver {
namespace vital {
namespace kpf {

class KPF_YAML_EXPORT kpf_reader_t
{
public:

  explicit kpf_reader_t( kpf_parser_base_t& parser );
  explicit operator bool() const;

  // load more packets, if necessary
  bool next();

  // push packets into the text_reader
  friend KPF_YAML_EXPORT kpf_reader_t& operator>>( kpf_reader_t& t, packet_bounce_t& b );

  // pull packets into the text_reader
  bool process( packet_bounce_t& b );
  bool process( kpf_canonical_io_adapter_base& io );

  // mystery: fails to link if this is not inline?
  const packet_buffer_t& get_packet_buffer() const { return this->packet_buffer; }

  // clear the packet buffer
  void flush() { this->packet_buffer.clear(); this->meta_buffer.clear(); }

  // look for a packet matching the header; if found,
  // return true, remove from buffer, return the packet
  // if not found, return false
  std::pair< bool, packet_t > transfer_packet_from_buffer( const packet_header_t& h );

  // like above, but specifically for kv (key/value) packets with a
  // particular key
  std::pair< bool, packet_t > transfer_kv_packet_from_buffer( const std::string& key );

  // return any meta packets
  std::vector< std::string > get_meta_packets() const;

private:
  bool process_reader( packet_bounce_t& b );
  bool parse_next_line();
  bool verify_reader_status();

  packet_buffer_t packet_buffer;
  std::vector< std::string > meta_buffer;
  bool reader_status;

  kpf_parser_base_t& parser;
};

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, packet_bounce_t& b );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::bbox_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::poly_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::activity_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::id_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::timestamp_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::kv_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::conf_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::meta_t >& r );

KPF_YAML_EXPORT
kpf_reader_t& operator>>( kpf_reader_t& t, const reader< canonical::timestamp_range_t >& r );

} // ...kpf
} // ...vital
} // ...kwiver

#endif
