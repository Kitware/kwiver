// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of FFmpeg video clipping utility.

#include <arrows/ffmpeg/ffmpeg_video_input_clip.h>

#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/ffmpeg/ffmpeg_video_settings.h>
#include <arrows/ffmpeg/ffmpeg_video_raw_image.h>
#include <arrows/ffmpeg/ffmpeg_video_raw_metadata.h>
#include <arrows/ffmpeg/ffmpeg_video_uninterpreted_data.h>

#include <stdexcept>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
class ffmpeg_video_input_clip::impl
{
public:
  impl();

  void seek_to_start();
  void filter_metadata(
    vital::metadata_vector& metadata, vital::timestamp const& ts ) const;
  vital::frame_id_t true_frame_begin() const;
  vital::frame_id_t true_frame_end() const;

  std::shared_ptr< ffmpeg_video_input > video;
  vital::frame_id_t frame_begin;
  vital::frame_id_t frame_end;

  vital::metadata_map_sptr all_metadata;
  std::string video_name;
  vital::timestamp initial_timestamp;
  int64_t initial_pts;
  bool start_at_keyframe;
  bool before_first_frame;
};

// ----------------------------------------------------------------------------
ffmpeg_video_input_clip::impl
::impl()
  : video{ new ffmpeg_video_input },
    frame_begin{ 0 },
    frame_end{ 0 },
    all_metadata{ nullptr },
    video_name{},
    initial_timestamp{},
    initial_pts{ AV_NOPTS_VALUE },
    start_at_keyframe{ false },
    before_first_frame{ true }
{}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_clip::impl
::seek_to_start()
{
  if( !video->seek_frame_(
        initial_timestamp, frame_begin,
        start_at_keyframe
        ? ffmpeg_video_input::SEEK_MODE_KEYFRAME_BEFORE
        : ffmpeg_video_input::SEEK_MODE_EXACT ) )
  {
    throw_error( "Could not start video clip" );
  }
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_clip::impl
::filter_metadata(
  vital::metadata_vector& metadata, vital::timestamp const& ts ) const
{
  for( auto& md : metadata )
  {
    if( md )
    {
      md.reset( new vital::metadata( *md ) );
      md->set_timestamp( ts );
    }
  }
}

// ----------------------------------------------------------------------------
vital::frame_id_t
ffmpeg_video_input_clip::impl
::true_frame_begin() const
{
  return
    initial_timestamp.has_valid_frame()
    ? initial_timestamp.get_frame()
    : frame_begin;
}

// ----------------------------------------------------------------------------
vital::frame_id_t
ffmpeg_video_input_clip::impl
::true_frame_end() const
{
  return
    video->num_frames()
    ? std::min< vital::frame_id_t >( frame_end, video->num_frames() )
    : frame_end;
}

// ----------------------------------------------------------------------------
ffmpeg_video_input_clip
::ffmpeg_video_input_clip()
  : d{ new impl }
{
  attach_logger( "ffmpeg_video_input_clip" );
}

// ----------------------------------------------------------------------------
ffmpeg_video_input_clip
::~ffmpeg_video_input_clip()
{}

// ----------------------------------------------------------------------------
vital::config_block_sptr
ffmpeg_video_input_clip
::get_configuration() const
{
  auto config = vital::algo::video_input::get_configuration();
  config->set_value(
    "frame_begin", d->frame_begin,
    "First frame to include in the clip. Indexed from 1." );
  config->set_value(
    "frame_end", d->frame_end,
    "First frame not to include in the clip, i.e. one past the final frame in "
    "the clip. Indexed from 1." );
  config->set_value(
    "start_at_keyframe", d->start_at_keyframe,
    "Start at the first keyframe before frame_begin, if frame_begin is not a "
    "keyframe." );

  vital::algo::video_input::
    get_nested_algo_configuration( "video_input", config, d->video );
  return config;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_clip
::set_configuration( vital::config_block_sptr in_config )
{
  auto config = get_configuration();
  config->merge_config( in_config );

  d->frame_begin =
    config->get_value< vital::frame_id_t >( "frame_begin", d->frame_begin );
  d->frame_end =
    config->get_value< vital::frame_id_t >( "frame_end", d->frame_end );
  d->start_at_keyframe =
    config->get_value< bool >( "start_at_keyframe", d->start_at_keyframe );

  d->video->set_configuration( config->subblock_view( "video_input:ffmpeg" ) );
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_clip
::check_configuration( vital::config_block_sptr config ) const
{
  if( !config->has_value( "frame_begin" ) ||
      !config->has_value( "frame_end" ) ||
      !config->has_value( "video_input:type" ) )
  {
    return false;
  }

  auto const frame_begin =
    config->get_value< vital::frame_id_t >( "frame_begin" );
  auto const frame_end =
    config->get_value< vital::frame_id_t >( "frame_end" );

  return frame_begin <= frame_end && frame_begin > 0;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_clip
::open( std::string video_name )
{
  d->video_name = video_name;
  d->before_first_frame = true;
  d->video->open( video_name );
  d->seek_to_start();
  auto const raw_image =
    dynamic_cast< ffmpeg_video_raw_image const* >(
      d->video->raw_frame_image().get() );
  if( !raw_image || raw_image->frame_pts == AV_NOPTS_VALUE )
  {
    throw std::runtime_error( "Could not acquire PTS of first frame" );
  }
  d->initial_pts = raw_image->frame_pts;

  auto const& capabilities = d->video->get_implementation_capabilities();
  using vi = vital::algo::video_input;
  for( auto const& capability : {
    vi::HAS_EOV,
    vi::HAS_FRAME_NUMBERS,
    vi::HAS_FRAME_DATA,
    vi::HAS_FRAME_TIME,
    vi::HAS_METADATA,
    vi::HAS_ABSOLUTE_FRAME_TIME,
    vi::HAS_TIMEOUT,
    vi::IS_SEEKABLE,
    vi::HAS_RAW_IMAGE,
    vi::HAS_RAW_METADATA,
    vi::HAS_UNINTERPRETED_DATA, } )
  {
    set_capability( capability, capabilities.capability( capability ) );
  }
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input_clip
::close()
{
  d->all_metadata.reset();
  d->video->close();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_clip
::end_of_video() const
{
  if( d->before_first_frame )
  {
    return false;
  }

  return
    d->video->end_of_video() ||
    ( d->video->frame_timestamp().get_frame() >= d->frame_end );
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_clip
::good() const
{
  if( d->before_first_frame || end_of_video() )
  {
    return false;
  }

  return d->video->good();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_clip
::seekable() const
{
  return d->video->seekable();
}

// ----------------------------------------------------------------------------
size_t
ffmpeg_video_input_clip
::num_frames() const
{
  return d->true_frame_end() - d->true_frame_begin();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_clip
::next_frame( vital::timestamp& ts, uint32_t timeout )
{
  if( end_of_video() )
  {
    ts = vital::timestamp{};
    return false;
  }

  if( d->before_first_frame )
  {
    d->before_first_frame = false;
    ts = frame_timestamp();
    return true;
  }

  vital::timestamp tmp_ts;
  auto const success =
    d->video->next_frame( tmp_ts, timeout ) && !end_of_video();
  ts = success ? frame_timestamp() : vital::timestamp{};
  return success;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input_clip
::seek_frame(
  vital::timestamp& ts, vital::timestamp::frame_t frame_number,
  uint32_t timeout )
{
  if( frame_number > 1 )
  {
    frame_number += d->true_frame_begin();
    frame_number = std::min( frame_number, d->true_frame_end() );
    return d->video->seek_frame( ts, frame_number, timeout );
  }
  else
  {
    d->seek_to_start();
    return good();
  }
}

// ----------------------------------------------------------------------------
vital::timestamp
ffmpeg_video_input_clip
::frame_timestamp() const
{
  auto video_ts = d->video->frame_timestamp();
  vital::timestamp ts;
  if( video_ts.has_valid_frame() )
  {
    ts.set_frame( video_ts.get_frame() - d->true_frame_begin() + 1 );
  }
  if( video_ts.has_valid_time() && d->initial_timestamp.has_valid_time() )
  {
    ts.set_time_usec(
      video_ts.get_time_usec() - d->initial_timestamp.get_time_usec() );
  }
  return ts;
}

// ----------------------------------------------------------------------------
vital::image_container_sptr
ffmpeg_video_input_clip
::frame_image()
{
  return d->before_first_frame ? nullptr : d->video->frame_image();
}

// ----------------------------------------------------------------------------
vital::video_raw_image_sptr
ffmpeg_video_input_clip
::raw_frame_image()
{
  return d->before_first_frame ? nullptr : d->video->raw_frame_image();
}

// ----------------------------------------------------------------------------
vital::metadata_vector
ffmpeg_video_input_clip
::frame_metadata()
{
  if( d->before_first_frame )
  {
    return {};
  }

  auto result = d->video->frame_metadata();
  d->filter_metadata( result, frame_timestamp() );
  return result;
}

// ----------------------------------------------------------------------------
vital::video_raw_metadata_sptr
ffmpeg_video_input_clip
::raw_frame_metadata()
{
  return d->before_first_frame ? nullptr : d->video->raw_frame_metadata();
}

// ----------------------------------------------------------------------------
vital::video_uninterpreted_data_sptr
ffmpeg_video_input_clip
::uninterpreted_frame_data()
{
  return d->before_first_frame ? nullptr : d->video->uninterpreted_frame_data();
}

// ----------------------------------------------------------------------------
vital::metadata_map_sptr
ffmpeg_video_input_clip
::metadata_map()
{
  if( d->all_metadata )
  {
    return d->all_metadata;
  }

  ffmpeg_video_input_clip tmp_video;
  tmp_video.set_configuration( get_configuration() );
  tmp_video.open( d->video_name );

  vital::metadata_map::map_metadata_t result;
  vital::timestamp ts;
  while( tmp_video.next_frame( ts ) )
  {
    result.emplace( ts.get_frame(), tmp_video.frame_metadata() );
  }

  d->all_metadata.reset(
    new vital::simple_metadata_map{ std::move( result ) } );
  return d->all_metadata;
}

// ----------------------------------------------------------------------------
vital::video_settings_uptr
ffmpeg_video_input_clip
::implementation_settings() const
{
  auto const settings = d->video->implementation_settings();
  auto const ffmpeg_settings =
    dynamic_cast< ffmpeg_video_settings const* >( settings.get() );
  if( !ffmpeg_settings )
  {
    return nullptr;
  }

  auto result = *ffmpeg_settings;
  result.start_timestamp = d->initial_pts;
  return std::make_unique< ffmpeg_video_settings >( std::move( result ) );
}

} // namespace ffmpeg

} // namespace arrows

} // end namespace
