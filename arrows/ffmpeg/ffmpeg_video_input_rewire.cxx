// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the ffmpeg_video_input_rewire class.

#include <arrows/ffmpeg/ffmpeg_video_input_rewire.h>

#include <arrows/ffmpeg/ffmpeg_video_raw_metadata.h>
#include <arrows/ffmpeg/ffmpeg_video_uninterpreted_data.h>
#include <arrows/ffmpeg/ffmpeg_util.h>

#include <map>
#include <tuple>

#include <cstdint>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

namespace {

// ----------------------------------------------------------------------------
struct source_video_input
{
  vital::algo::video_input_sptr input;
  std::string filename;
};

// ----------------------------------------------------------------------------
enum { UNMARKED_STREAM = SIZE_MAX };

} // namespace <anonymous>

// ----------------------------------------------------------------------------
class ffmpeg_video_input_rewire::impl
{
public:
  impl();

  std::map<size_t, source_video_input> video_sources;

  // { source index, stream index } : output index
  std::map<std::tuple<size_t, size_t>, size_t> rewire_map;
};

// ----------------------------------------------------------------------------
ffmpeg_video_input_rewire::impl
::impl() : video_sources{}
{}

// ----------------------------------------------------------------------------
ffmpeg_video_input_rewire
::ffmpeg_video_input_rewire() : d{ new impl }
{}

// ----------------------------------------------------------------------------
ffmpeg_video_input_rewire
::~ffmpeg_video_input_rewire()
{}

// ----------------------------------------------------------------------------
vital::config_block_sptr
ffmpeg_video_input_rewire
::get_configuration() const
{
  auto config = video_input::get_configuration();

  for( auto const& [index, source] : d->video_sources )
  {
    auto const prefix = "source-" + std::to_string( index ) + ":";
    config->set_value( prefix + "type", "video" );
    config->set_value( prefix + "filename", source.filename );
    video_input::get_nested_algo_configuration(
      prefix + "input", config, source.input );
  }

  std::map< size_t, std::tuple< size_t, size_t > > reverse_map;
  for( auto& [key, value] : d->rewire_map )
  {
    reverse_map.emplace( value, key );
  }

  auto first = true;
  std::stringstream ss;
  for( auto& [value, key] : reverse_map )
  {
    auto [source_index, stream_index] = key;
    first = first ? false : ( ss << ",", false );
    ss << source_index << "/";
    if( stream_index == UNMARKED_STREAM )
    {
      ss << "unmarked";
    }
    else
    {
      ss << stream_index;
    }
  }
  config->set_value( "streams", ss.str() );

  return config;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_rewire
::set_configuration( vital::config_block_sptr in_config )
{
  auto config = get_configuration();
  config->merge_config( in_config );

  // Video sources come labeled "source-X", where X is the index. All Xs must
  // be sequential starting from 0
  d->video_sources.clear();
  for( size_t i = 0; true; ++i )
  {
    auto const prefix = "source-" + std::to_string( i ) + ":";
    auto const type = config->get_value< std::string >( prefix + "type", "" );

    if( type == "" )
    {
      // Couldn't find type of next source; quit looking
      break;
    }

    if( type != "video" )
    {
      // We may support other (e.g. metadata_map_io) sources in the future
      continue;
    }

    // File to be opened
    auto const filename =
      config->get_value< std::string >( prefix + "filename", "" );

    // Create nested video input
    source_video_input source{ nullptr, filename };
    video_input::set_nested_algo_configuration(
      prefix + "input", config, source.input );
    d->video_sources.emplace( i, std::move( source ) );
  }

  // Parse the rewiring string, which consists of comma-separated "X/Y" pairs.
  // X is the index of the source video, and Y is the stream index within that
  // source video
  size_t i = 1;
  d->rewire_map.clear();
  for( auto const& element :
       config->get_value_as_vector< std::string >( "streams", "," ) )
  {
    std::stringstream ss{ element };
    std::tuple< size_t, size_t > key;
    char dummy;
    std::string stream_index;
    ss >> std::get< 0 >( key ) >> dummy >> stream_index;
    std::get< 1 >( key ) =
      ( stream_index == "unmarked" )
      ? UNMARKED_STREAM
      : std::stoull( stream_index );
    d->rewire_map.emplace( key, i++ );
  }
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_rewire
::check_configuration( vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_rewire
::open( VITAL_UNUSED std::string video_name )
{
  for( auto& [index, source] : d->video_sources )
  {
    source.input->open( source.filename );
  }
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_rewire
::close()
{
  for( auto& [index, source] : d->video_sources )
  {
    source.input->close();
  }
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_rewire
::end_of_video() const
{
  // Video data taken from first video source
  return d->video_sources.at( 0 ).input->end_of_video();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_rewire
::good() const
{
  // Video data taken from first video source
  return d->video_sources.at( 0 ).input->good();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_rewire
::seekable() const
{
  return false;
}

// ----------------------------------------------------------------------------
size_t
ffmpeg_video_input_rewire
::num_frames() const
{
  // Video data taken from first video source
  return d->video_sources.at( 0 ).input->num_frames();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_rewire
::next_frame( vital::timestamp& ts, uint32_t timeout )
{
  auto result = false;
  for( auto& [index, source] : d->video_sources )
  {
    if( index == 0 )
    {
      // Timestamp data taken from first video source
      result = source.input->next_frame( ts, timeout );
    }
    else
    {
      vital::timestamp tmp_ts;
      source.input->next_frame( tmp_ts, timeout );
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_rewire
::seek_frame(
  VITAL_UNUSED vital::timestamp& ts,
  VITAL_UNUSED vital::timestamp::frame_t frame_number,
  VITAL_UNUSED uint32_t timeout )
{
  return false;
}

// ----------------------------------------------------------------------------
vital::timestamp
ffmpeg_video_input_rewire
::frame_timestamp() const
{
  // Timestamp data taken from first video source
  return d->video_sources.at( 0 ).input->frame_timestamp();
}

// ----------------------------------------------------------------------------
vital::image_container_sptr
ffmpeg_video_input_rewire
::frame_image()
{
  // Video data taken from first video source
  return d->video_sources.at( 0 ).input->frame_image();
}

// ----------------------------------------------------------------------------
vital::video_raw_image_sptr
ffmpeg_video_input_rewire
::raw_frame_image()
{
  // Video data taken from first video source
  return d->video_sources.at( 0 ).input->raw_frame_image();
}

// ----------------------------------------------------------------------------
vital::metadata_vector
ffmpeg_video_input_rewire
::frame_metadata()
{
  vital::metadata_vector result;

  // Find the first non-null metadata from the video stream source
  vital::metadata_sptr video_md;
  if( auto const video_metadata =
        d->video_sources.at( 0 ).input->frame_metadata();
      !video_metadata.empty() )
  {
    for( auto const& md : video_metadata )
    {
      if( md )
      {
        video_md = md;
        break;
      }
    }
  }

  for( auto const& [source_index, source] : d->video_sources )
  {
    // Skip sources that have ended
    if( !source.input->good() )
    {
      continue;
    }

    // Map all metadata to its correct destination stream index
    for( auto const& source_md : source.input->frame_metadata() )
    {
      size_t stream_index = UNMARKED_STREAM;
      if( auto const entry =
            source_md->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX ) )
      {
        auto const int_value = entry.get< int >();
        if( int_value < 0 )
        {
          continue;
        }
        stream_index = static_cast< size_t >( int_value );
      }

      auto const key = std::make_tuple( source_index, stream_index );
      if( auto const it = d->rewire_map.find( key ); it != d->rewire_map.end() )
      {
        auto const& md = result.emplace_back( source_md->clone() );

        // Relabel stream index
        md->add< vital::VITAL_META_VIDEO_DATA_STREAM_INDEX >(
          static_cast< int >( it->second ) );

        // Overwrite metadata related to video stream
        for( auto const tag : {
          vital::VITAL_META_VIDEO_KEY_FRAME,
          vital::VITAL_META_VIDEO_FRAME_NUMBER,
          vital::VITAL_META_VIDEO_MICROSECONDS,
          vital::VITAL_META_VIDEO_FRAME_RATE,
          vital::VITAL_META_VIDEO_BITRATE,
          vital::VITAL_META_VIDEO_COMPRESSION_TYPE,
          vital::VITAL_META_VIDEO_COMPRESSION_PROFILE,
          vital::VITAL_META_VIDEO_COMPRESSION_LEVEL,
        } )
        {
          if( video_md && video_md->find( tag ) )
          {
            md->add( tag, video_md->find( tag ).data() );
          }
          else
          {
            md->erase( tag );
          }
        }
      }
    }
  }

  // Sort output metadata by stream index
  auto const cmp =
    []( vital::metadata_sptr const& lhs, vital::metadata_sptr const& rhs )
    {
      if( !lhs && rhs )
      {
        return true;
      }
      if( lhs && !rhs )
      {
        return false;
      }
      return
        lhs->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX ).get< int >() <
        rhs->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX ).get< int >();
    };
  std::sort( result.begin(), result.end(), cmp );

  return result;
}

// ----------------------------------------------------------------------------
vital::video_raw_metadata_sptr
ffmpeg_video_input_rewire
::raw_frame_metadata()
{
  auto result = std::make_shared< ffmpeg_video_raw_metadata >();
  for( auto const& [source_index, source] : d->video_sources )
  {
    // Skip sources that have ended
    if( !source.input->good() )
    {
      continue;
    }

    auto const source_md =
      dynamic_cast< ffmpeg_video_raw_metadata* >(
        source.input->raw_frame_metadata().get() );
    if( !source_md )
    {
      continue;
    }

    // Combine packets from all sources
    for( auto const& packet_info : source_md->packets )
    {
      auto const stream_index = packet_info.packet->stream_index;
      if( stream_index < 0 )
      {
        continue;
      }

      auto const key =
        std::make_tuple( source_index, static_cast< size_t >( stream_index ) );
      if( auto const it = d->rewire_map.find( key ); it != d->rewire_map.end() )
      {
        auto& new_packet_info = result->packets.emplace_back();
        new_packet_info.packet.reset(
          throw_error_null( av_packet_clone( packet_info.packet.get() ),
          "Failed to clone packet" ) );

        // Relabel stream index
        new_packet_info.packet->stream_index = it->second;
        new_packet_info.stream_settings = packet_info.stream_settings;
      }
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
vital::video_uninterpreted_data_sptr
ffmpeg_video_input_rewire
::uninterpreted_frame_data()
{
  auto result = std::make_shared< ffmpeg_video_uninterpreted_data >();
  for( auto const& [source_index, source] : d->video_sources )
  {
    // Skip sources that have ended
    if( !source.input->good() )
    {
      continue;
    }

    auto const source_data_ptr = source.input->uninterpreted_frame_data();
    auto const source_data =
      dynamic_cast< ffmpeg_video_uninterpreted_data* >( source_data_ptr.get() );
    if( !source_data )
    {
      continue;
    }

    // Combine packets from all sources
    for( auto const& packet : source_data->audio_packets )
    {
      auto const stream_index = packet->stream_index;
      if( stream_index < 0 )
      {
        continue;
      }

      auto const key =
        std::make_tuple( source_index, static_cast< size_t >( stream_index ) );
      if( auto const it = d->rewire_map.find( key ); it != d->rewire_map.end() )
      {
        auto const new_packet =
          throw_error_null(
            av_packet_clone( packet.get() ), "Failed to clone packet" );

        // Relabel stream index
        new_packet->stream_index = it->second;
        result->audio_packets.emplace_back( new_packet );
      }
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
vital::metadata_map_sptr
ffmpeg_video_input_rewire
::metadata_map()
{
  return nullptr;
}

// ----------------------------------------------------------------------------
vital::video_settings_uptr
ffmpeg_video_input_rewire
::implementation_settings() const
{
  // Video data taken from first video source
  return d->video_sources.at( 0 ).input->implementation_settings();
}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
