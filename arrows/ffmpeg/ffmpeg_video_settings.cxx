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
    klv_stream_count{ 0 }
{
  if( !parameters )
  {
    throw std::runtime_error{ "Could not allocate AVCodecParameters" };
  }
  parameters->codec_type = AVMEDIA_TYPE_VIDEO;
}

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::ffmpeg_video_settings(
  size_t width, size_t height,
  AVRational frame_rate,
  size_t klv_stream_count )
  : frame_rate( frame_rate ),
    parameters{ avcodec_parameters_alloc() },
    klv_stream_count{ klv_stream_count }
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

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
