#include <arrows/kpf/yaml/kpf_reader.h>
#include <arrows/kpf/yaml/kpf_yaml_parser.h>
#include <arrows/kpf/yaml/kpf_yaml_writer.h>

#include <vital/types/homography.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>

using std::cerr;
using std::cout;
using std::string;
using std::ifstream;
using std::pair;
using std::make_pair;
using std::vector;

namespace KPF=::kwiver::vital::kpf;

typedef ::kwiver::vital::homography_< double > homography_t;

pair<bool, homography_t>  load_homography( const string& fn );
kpf::packet_t warp_g0( const kpf::packet_t& p, const homography_t& h );
kpf::packet_t warp_ts0( const kpf::packet_t& p, int ts_offset );

int main( int argc, char *argv[] )
{
  if (argc != 4)
  {
    cerr << "Usage: " << argv[0] << " source-file.kpf homog-file frame-offset > new-file.kpf\n";
    return EXIT_FAILURE;
  }

  pair< bool, homography_t > h = load_homography( argv[2] );
  if (!h.first)
  {
    return EXIT_FAILURE;
  }

  ifstream is( argv[1] );
  if ( ! is )
  {
    cerr << "Couldn't open source KPF '" << argv[1] << "'; exiting\n";
    return EXIT_FAILURE;
  }

  const KPF::packet_header_t g0_header( KPF::packet_style::GEOM, 0 ), ts0_header( KPF::packet_style::TS, 0 );

  KPF::kpf_yaml_parser_t parser( is );
  KPF::kpf_reader_t reader( parser );
  while (reader.next())
  {
    // copy out any metadata
    vector< string > meta = reader.get_meta_packets();
    writer.set_schema( KPF::schema_style::META );
    for (auto m: meta)
    {
      writer << m << KPF::record_yaml_writer::endl;
    }

    const KPF::packet_buffer_t& packets = reader.get_packet_buffer();
    for (auto p: packets)
    {
      // is this a G0 (bounding box)? If so, apply the homography and write it out
      if (p.header == g0_header)
      {
        writer << warp_g0( p, h.second );
      }

      // is this a frame timestamp? If so, apply the offset and write it out
      else if (p.header == ts0_header)
      {
        writer << warp_ts0( p, ts_offset );
      }

      // otherwise, just write it out
      else
      {
        writer << p;
      }
    }

    writer << KPF::record_yaml_writer::endl;
    reader.flush();
  }

}

pair< bool, homography_t >
load_homography( const string& fn )
{
  ifstream is( fn.c_str() );
  if ( ! is )
  {
    cerr << "Couldn't open homography file '" << fn << "'; exiting\n";
    return make_pair( false, homography_t() );
  }

  vector<double> d;
  string line;
  while ( std::getline( is, line ))
  {
    std::istringstream iss( line );
    double tmp;
    while ((iss >> tmp )) d.push_back( tmp );
  }
  if ( d.size() != 9 )
  {
    cerr << "Homography error reading '" << fn << "': read " << d.size() << " elements; expected 9; exiting\n";
    return make_pair( false, homography_t() );
  }

  homography_t ret;
  size_t i = 0;
  for (auto r=0; r<3; ++r)
  {
    for (auto c=0; c<3; ++c)
    {
      ret.get_matrix()(r, c) = d[i++];
    }
  }
  return make_pair( true, ret );
}

kpf::packet_t
warp_g0( const kpf::packet_t& p, const homography_t& h )
{
  return p;
}

kpf::packet_t warp_ts0( const kpf::packet_t& p, int ts_offset )
{
  
}
