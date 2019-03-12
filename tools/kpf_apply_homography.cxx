/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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

/**
 * \file
 * \brief Utility to apply a homography and timestamp offset to KPF files.
 */

#include <arrows/kpf/yaml/kpf_reader.h>
#include <arrows/kpf/yaml/kpf_yaml_parser.h>
#include <arrows/kpf/yaml/kpf_yaml_writer.h>

#include <vital/types/homography.h>
#include <vital/util/file_md5.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <stdexcept>

using std::cerr;
using std::cout;
using std::string;
using std::ostringstream;
using std::ifstream;
using std::pair;
using std::make_pair;
using std::vector;
using std::runtime_error;
using std::to_string;

namespace KPF=::kwiver::vital::kpf;
namespace KPFC=::kwiver::vital::kpf::canonical;

typedef ::kwiver::vital::homography_< double > homography_t;

homography_t load_homography( const string& fn );
KPF::packet_t warp_g0( const KPF::packet_t& p, const homography_t& h );
KPF::packet_t warp_ts0( const KPF::packet_t& p, int ts_offset );
KPF::packet_t warp_act2( const KPF::packet_t& p, int ts_offset );
void apply_offset_to_tsr( vector< KPFC::scoped< KPFC::timestamp_range_t> >& tsr_list,
                          int domain,
                          int ts_offset );

int main( int argc, char *argv[] )
{
  homography_t h;
  int frame_offset(0);
  ifstream is;

  // setup the inputs
  try
  {
    if (argc != 4) throw runtime_error("Usage: " + string(argv[0])
                                       + " source-file.kpf homog-file frame-offset > new-file.kpf");

    is.open( argv[1] );
    if ( ! is ) throw runtime_error("Couldn't open source KPF '" + string(argv[1]) + "'; exiting");

    h = load_homography( argv[2] );

    frame_offset = std::stoi( argv[3] );

  }
  catch (const std::exception& e )
  {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  // set up the writer; record some stats about the transformation as metadata
  KPF::record_yaml_writer kpf_out( cout );

  {
    kpf_out << KPF::writer< KPFC::meta_t>( "\"Transform source: '" + string(argv[1]) + "'\"" )
            << KPF::record_yaml_writer::endl;
    kpf_out << KPF::writer< KPFC::meta_t>( "\"Transform source md5: "
                                           +kwiver::vital::file_md5( argv[1] )+ "\"")
            << KPF::record_yaml_writer::endl;
    ostringstream oss;
    for (auto r=0; r<3; ++r)
    {
      for (auto c=0; c<3; ++c)
      {
        oss << h.get_matrix()(r, c) << " ";
      }
    }
    kpf_out << KPF::writer< KPFC::meta_t>( "\"Transform homography: " + oss.str() + "\"" )
            << KPF::record_yaml_writer::endl;

    kpf_out << KPF::writer< KPFC::meta_t>( "\"Transform frame offset: " + to_string(frame_offset) + "\"" )
            << KPF::record_yaml_writer::endl;
  }

  const KPF::packet_header_t g0_header( KPF::packet_style::GEOM, 0 );
  const KPF::packet_header_t ts0_header( KPF::packet_style::TS, 0 );
  const KPF::packet_header_t act2_header( KPF::packet_style::ACT, 2 );

  KPF::kpf_yaml_parser_t parser( is );
  KPF::kpf_reader_t reader( parser );
  while (reader.next())
  {
    // copy out any metadata
    vector< string > meta = reader.get_meta_packets();
    kpf_out.set_schema( KPF::schema_style::META );
    for (auto m: meta)
    {
      kpf_out << m << KPF::record_yaml_writer::endl;
    }

    kpf_out.set_schema( parser.get_current_record_schema() );
    const KPF::packet_buffer_t& packets = reader.get_packet_buffer();
    for (auto p: packets)
    {
      // is this a G0 (bounding box)? If so, apply the homography and write it out
      if (p.first == g0_header)
      {
        kpf_out << warp_g0( p.second, h );
      }

      // is this a frame timestamp? If so, apply the offset and write it out
      else if (p.first == ts0_header)
      {
        kpf_out << warp_ts0( p.second, frame_offset );
      }

      // is this a DIVA activity? If so, apply the offset and write it out
      else if (p.first == act2_header )
      {
        kpf_out << warp_act2( p.second, frame_offset );
      }

      // otherwise, just write it out
      else
      {
        kpf_out << p.second;
      }
    }

    kpf_out << KPF::record_yaml_writer::endl;
    reader.flush();
  }

  // all done!
}

homography_t
load_homography( const string& fn )
{
  ifstream is( fn.c_str() );
  if ( ! is ) throw runtime_error( "Couldn't open homography file '" + fn + "'; exiting\n" );

  vector<double> d;
  string line;
  while ( std::getline( is, line ))
  {
    std::istringstream iss( line );
    double tmp;
    while ((iss >> tmp )) d.push_back( tmp );
  }
  if ( d.size() != 9 ) throw runtime_error( "Homography error reading '" + fn  + "': read "
                                            + to_string(d.size()) + " elements; expected 9; exiting\n" );

  homography_t ret;
  size_t i = 0;
  for (auto r=0; r<3; ++r)
  {
    for (auto c=0; c<3; ++c)
    {
      ret.get_matrix()(r, c) = d[i++];
    }
  }
  return ret;
}

KPF::packet_t
warp_g0( const KPF::packet_t& p, const homography_t& h )
{
  KPF::packet_t new_packet( p );
  Eigen::Matrix< double, 2, 1 > pt_ul( p.bbox.x1, p.bbox.y1 ), pt_lr( p.bbox.x2, p.bbox.y2 );
  auto new_ul = h.map_point( pt_ul );
  new_packet.bbox.x1 = new_ul( 0, 0 );
  new_packet.bbox.y1 = new_ul( 1, 0 );
  auto new_lr = h.map_point( pt_lr );
  new_packet.bbox.x2 = new_lr( 0, 0 );
  new_packet.bbox.y2 = new_lr( 1, 0 );
  return new_packet;
}

KPF::packet_t warp_ts0( const KPF::packet_t& p, int frame_offset )
{
  KPF::packet_t new_packet( p );
  new_packet.timestamp.d += frame_offset;
  return new_packet;
}

void apply_offset_to_tsr( vector< KPFC::scoped< KPFC::timestamp_range_t> >& tsr_list,
                          int domain,
                          int ts_offset )
{
  for (auto& p: tsr_list )
  {
    if (p.domain == domain)
    {
      p.t.start += ts_offset;
      p.t.stop += ts_offset;
    }
  }
}


KPF::packet_t warp_act2( const KPF::packet_t& p, int frame_offset )
{
  KPF::packet_t new_packet( p );
  const int domain=0;
  apply_offset_to_tsr( new_packet.activity.timespan, domain, frame_offset );
  for (auto& a: new_packet.activity.actors )
  {
    apply_offset_to_tsr( a.actor_timespan, domain, frame_offset );
  }
  return new_packet;
}
