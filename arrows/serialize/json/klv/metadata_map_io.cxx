// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/serialize/json/klv/metadata_map_io.h>

#include <arrows/serialize/json/klv/load_save_klv.h>

#include <arrows/klv/klv_demuxer.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/klv/klv_packet.h>

#include <arrows/zlib/bytestream_compressor.h>

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
{
public:
  priv();

  bool compress;
  vital::bytestream_compressor::compression_type_t compress_type;
};

// ----------------------------------------------------------------------------
metadata_map_io_klv::priv
::priv()
  : compress{ false },
    compress_type{ vital::bytestream_compressor::COMPRESSION_TYPE_DEFLATE }
{}

// ----------------------------------------------------------------------------
metadata_map_io_klv
::metadata_map_io_klv() : d{ new priv } {}

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
  std::vector< klv::klv_timed_packet > packets;
  try
  {
    if( d->compress )
    {
      vital::bytestream_compressor decompressor(
        vital::bytestream_compressor::MODE_DECOMPRESS,
        d->compress_type,
        vital::bytestream_compressor::DATA_TYPE_TEXT );
      vital::compress_istream compress_is( fin, decompressor );

      cereal::JSONInputArchive archive( compress_is );
      cereal::load( archive, packets );
    }
    else
    {
      cereal::JSONInputArchive archive( fin );
      cereal::load( archive, packets );
    }
  }
  catch( cereal::RapidJSONException const& e )
  {
    LOG_ERROR( logger(), "Failed to load KLV JSON: " << e.what() );
    return std::make_shared< vital::simple_metadata_map >();
  }

  // Add KLV for each frame to vital::metadata structures
  vital::metadata_map::map_metadata_t result;
  for( auto& packet : packets )
  {
    // Find entry for this frame
    auto const it =
      result.emplace(
        packet.timestamp.get_frame(),
        std::vector< vital::metadata_sptr >{} ).first;

    // Try to find vital::metadata with this packet's stream index
    klv::klv_metadata* klv_md = nullptr;
    for( auto const& md : it->second )
    {
      auto const index_entry =
        md->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX );
      if( index_entry &&
          index_entry.template get< int >() == packet.stream_index )
      {
        klv_md = dynamic_cast< klv::klv_metadata* >( md.get() );
      }
    }

    // Otherwise, create new vital::metadata with this packet's stream index
    if( !klv_md )
    {
      klv_md = new klv::klv_metadata;
      klv_md->add< vital::VITAL_META_VIDEO_DATA_STREAM_INDEX >(
        static_cast< int >( packet.stream_index ) );
      klv_md->set_timestamp( packet.timestamp );
      it->second.emplace_back( klv_md );
    }

    // Add the KLV packet data
    klv_md->klv().emplace_back( std::move( packet.packet ) );
  }

  // Convert to vital::metadata_map_sptr
  return std::make_shared< vital::simple_metadata_map >( std::move( result ) );
}

// ----------------------------------------------------------------------------
std::ios_base::openmode
metadata_map_io_klv
::load_open_mode( VITAL_UNUSED std::string const& filename ) const
{
  return d->compress
    ? ( std::ios_base::in | std::ios_base::binary )
    : ( std::ios_base::in );
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
  if( d->compress )
  {
    vital::bytestream_compressor compressor(
      vital::bytestream_compressor::MODE_COMPRESS,
      d->compress_type,
      vital::bytestream_compressor::DATA_TYPE_TEXT );
    vital::compress_ostream compress_os( fout, compressor );

    cereal::JSONOutputArchive archive( compress_os, output_options );
    cereal::save( archive, packets );
  }
  else
  {
    cereal::JSONOutputArchive archive( fout, output_options );
    cereal::save( archive, packets );
  }
}

// ----------------------------------------------------------------------------
std::ios_base::openmode
metadata_map_io_klv
::save_open_mode( VITAL_UNUSED std::string const& filename ) const
{
  return d->compress
    ? ( std::ios_base::out | std::ios_base::binary )
    : ( std::ios_base::out );
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
metadata_map_io_klv
::get_configuration() const
{
  auto config = algorithm::get_configuration();

  config->set_value(
    "compress", d->compress,
    "Set to true to read and write compressed JSON instead." );

  return config;
}

// ----------------------------------------------------------------------------
void
metadata_map_io_klv
::set_configuration( vital::config_block_sptr config )
{
  d->compress = config->get_value< bool >( "compress", false );
}

} // namespace json

} // namespace serialize

} // namespace arrows

} // namespace kwiver
