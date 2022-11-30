// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/serialize/json/klv/metadata_map_io.h>

#include <arrows/serialize/json/klv/load_save_klv.h>

#include <arrows/klv/klv_demuxer.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/klv/klv_packet.h>

#include <vital/internal/cereal/archives/json.hpp>
#include <vital/internal/cereal/types/vector.hpp>

#include <vital/logger/logger.h>

#include <cfloat>

namespace kwiver {

namespace arrows {

namespace serialize {

namespace json {

namespace {

// ----------------------------------------------------------------------------
// Indent JSON using tabs; save the full precision of all floating-point values
auto const output_options = cereal::JSONOutputArchive::Options{
  cereal::JSONOutputArchive::Options::MaxPrecision(),
  cereal::JSONOutputArchive::Options::IndentChar::tab, 1 };
}

// ----------------------------------------------------------------------------
class metadata_map_io_klv::priv
{};

// ----------------------------------------------------------------------------
metadata_map_io_klv
::metadata_map_io_klv() : d{ nullptr } {}

// ----------------------------------------------------------------------------
metadata_map_io_klv
::~metadata_map_io_klv()
{}

// ----------------------------------------------------------------------------
vital::metadata_map_sptr
metadata_map_io_klv
::load_( std::istream& fin, VITAL_UNUSED std::string const& filename ) const
{
  // Load KLV from JSON
  cereal::JSONInputArchive archive( fin );
  std::vector< klv::klv_timed_packet > packets;
  cereal::load( archive, packets );

  // Group KLV by frame
  std::map< vital::frame_id_t, std::vector< klv::klv_packet > > packet_map;
  for( auto& packet : packets )
  {
    if( packet.timestamp.has_valid_frame() )
    {
      auto const it = packet_map.emplace(
        packet.timestamp.get_frame(),
        std::vector< klv::klv_packet >{} ).first;
      it->second.emplace_back( std::move( packet.packet ) );
    }
    else
    {
      LOG_DEBUG( vital::get_logger( "json" ),
                 "load_(): dropping KLV packet with no associated frame" );
    }
  }

  // Add KLV for each frame to vital::metadata structures
  vital::metadata_map::map_metadata_t result;
  for( auto& entry : packet_map )
  {
    // Put KLV into vital::metadata
    auto const metadata_klv = new klv::klv_metadata;
    metadata_klv->set_klv( std::move( entry.second ) );
    vital::metadata_sptr metadata_vital{ metadata_klv };

    // Record the frame number
    vital::timestamp ts;
    ts.set_frame( entry.first );
    metadata_vital->set_timestamp( ts );

    // Package it up
    vital::metadata_vector metadata_vector{ std::move( metadata_vital ) };
    result.emplace( entry.first, std::move( metadata_vector ) );
  }

  // Convert to vital::metadata_map_sptr
  return std::make_shared< vital::simple_metadata_map >( std::move( result ) );
}

// ----------------------------------------------------------------------------
void
metadata_map_io_klv
::save_( std::ostream& fout, vital::metadata_map_sptr data,
         VITAL_UNUSED std::string const& filename ) const
{
  // Extract KLV from vital::metadata structures
  std::vector< klv::klv_timed_packet > packets;
  for( auto const& entry : data->metadata() )
  {
    for( auto const& metadata_vital : entry.second )
    {
      // See if this vital::metadata holds KLV under the hood
      auto const metadata_klv =
        dynamic_cast< klv::klv_metadata const* >( metadata_vital.get() );
      if( !metadata_klv )
      {
        LOG_DEBUG( vital::get_logger( "json" ),
                   "save_(): dropping metadata with no associated KLV" );
        continue;
      }

      // Unpack the KLV
      for( auto const& packet : metadata_klv->klv() )
      {
        auto ts = metadata_klv->timestamp();
        ts.set_frame( entry.first );
        klv::klv_timed_packet timed_packet{ packet, ts };
        auto const stream_index =
          metadata_klv->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX );
        if( stream_index )
        {
          timed_packet.stream_index = stream_index.get< int >();
        }
        packets.emplace_back( timed_packet );
      }
    }
  }

  // Save KLV to JSON
  cereal::JSONOutputArchive archive( fout, output_options );
  cereal::save( archive, packets );
}

} // namespace json

} // namespace serialize

} // namespace arrows

} // namespace kwiver
