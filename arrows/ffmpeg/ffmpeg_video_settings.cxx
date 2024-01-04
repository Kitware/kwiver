// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of FFmpeg video settings.

#include <arrows/ffmpeg/ffmpeg_video_settings.h>

#include <stdexcept>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::ffmpeg_video_settings()
  : frame_rate{ 0, 1 },
    parameters{ avcodec_parameters_alloc() },
    klv_streams{},
    audio_streams{},
    time_base{ 0, 1 },
    start_timestamp{ AV_NOPTS_VALUE },
    codec_options{}
{
  if( !parameters )
  {
    throw std::runtime_error{ "Could not allocate AVCodecParameters" };
  }
  parameters->codec_type = AVMEDIA_TYPE_VIDEO;
}

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::ffmpeg_video_settings( ffmpeg_video_settings const& other )
  : frame_rate{ other.frame_rate },
    parameters{ avcodec_parameters_alloc() },
    klv_streams{ other.klv_streams },
    audio_streams{ other.audio_streams },
    time_base{ other.time_base },
    start_timestamp{ other.start_timestamp },
    codec_options{ other.codec_options }
{
  throw_error_code(
    avcodec_parameters_copy( parameters.get(), other.parameters.get() ),
    "Could not copy codec parameters" );
}

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::ffmpeg_video_settings( ffmpeg_video_settings&& other )
  : frame_rate{ std::move( other.frame_rate ) },
    parameters{ std::move( other.parameters ) },
    klv_streams{ std::move( other.klv_streams ) },
    audio_streams{ std::move( other.audio_streams ) },
    time_base{ std::move( other.time_base ) },
    start_timestamp{ std::move( other.start_timestamp ) },
    codec_options{ std::move( other.codec_options ) }
{}

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::ffmpeg_video_settings(
  size_t width, size_t height,
  AVRational frame_rate,
  std::vector< klv::klv_stream_settings > const& klv_streams )
  : frame_rate( frame_rate ),
    parameters{ avcodec_parameters_alloc() },
    klv_streams{ klv_streams },
    time_base{ 0, 1 },
    start_timestamp{ AV_NOPTS_VALUE },
    codec_options{}
{
  if( !parameters )
  {
    throw std::runtime_error{ "Could not allocate AVCodecParameters" };
  }
  parameters->codec_type = AVMEDIA_TYPE_VIDEO;
  parameters->width = width;
  parameters->height = height;
}

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::~ffmpeg_video_settings()
{}

// ----------------------------------------------------------------------------
ffmpeg_video_settings&
ffmpeg_video_settings
::operator=( ffmpeg_video_settings const& other )
{
  frame_rate = other.frame_rate;
  parameters.reset( avcodec_parameters_alloc() );
  throw_error_code(
    avcodec_parameters_copy( parameters.get(), other.parameters.get() ),
    "Could not copy codec parameters" );
  klv_streams = other.klv_streams;
  time_base = other.time_base;
  start_timestamp = other.start_timestamp;
  codec_options = other.codec_options;
  return *this;
}

// ----------------------------------------------------------------------------
ffmpeg_video_settings&
ffmpeg_video_settings
::operator=( ffmpeg_video_settings&& other )
{
  frame_rate = std::move( other.frame_rate );
  parameters = std::move( other.parameters );
  klv_streams = std::move( other.klv_streams );
  time_base = std::move( other.time_base );
  start_timestamp = std::move( other.start_timestamp );
  codec_options = std::move( other.codec_options );
  return *this;
}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
