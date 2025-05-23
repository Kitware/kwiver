// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of FFmpeg audio stream settings.

#include <arrows/ffmpeg/ffmpeg_audio_stream_settings.h>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
ffmpeg_audio_stream_settings
::ffmpeg_audio_stream_settings()
  : index{ -1 },
    parameters{ avcodec_parameters_alloc() },
    time_base{ 0, 1 }
{
  if( !parameters )
  {
    throw std::runtime_error{ "Could not allocate AVCodecParameters" };
  }
  parameters->codec_type = AVMEDIA_TYPE_AUDIO;
}

// ----------------------------------------------------------------------------
ffmpeg_audio_stream_settings
::ffmpeg_audio_stream_settings( ffmpeg_audio_stream_settings const& other )
  : parameters{}
{
  *this = other;
}

// ----------------------------------------------------------------------------
ffmpeg_audio_stream_settings
::ffmpeg_audio_stream_settings( ffmpeg_audio_stream_settings&& other )
  : parameters{}
{
  *this = std::move( other );
}

// ----------------------------------------------------------------------------
ffmpeg_audio_stream_settings
::~ffmpeg_audio_stream_settings()
{}

// ----------------------------------------------------------------------------
ffmpeg_audio_stream_settings&
ffmpeg_audio_stream_settings
::operator=( ffmpeg_audio_stream_settings const& other )
{
  index = other.index;

  parameters.reset(
    throw_error_null(
      avcodec_parameters_alloc(),
      "Could not allocate AVCodecParameters" ) );

  throw_error_code(
    avcodec_parameters_copy( parameters.get(), other.parameters.get() ),
    "Could not copy codec parameters" );

  time_base = other.time_base;

  return *this;
}

// ----------------------------------------------------------------------------
ffmpeg_audio_stream_settings&
ffmpeg_audio_stream_settings
::operator=( ffmpeg_audio_stream_settings&& other )
{
  index = std::move( other.index );
  parameters = std::move( other.parameters );
  time_base = std::move( other.time_base );

  return *this;
}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
