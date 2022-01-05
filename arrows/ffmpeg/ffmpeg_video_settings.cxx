// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of FFmpeg video settings.

#include <arrows/ffmpeg/ffmpeg_video_settings.h>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::ffmpeg_video_settings()
  : width{ 0 },
    height{ 0 },
    codec_id{ AV_CODEC_ID_H264 },
    pixel_format{ AV_PIX_FMT_RGB24 },
    frame_rate{ 0, 1 },
    time_base{ 0, 1 },
    sample_aspect_ratio{ 1, 1 },
    bit_rate{ 0 },
    gop_size{ 0 },
    profile{ 0 },
    level{ 0 },
    stream_id{ 0 } {}

// ----------------------------------------------------------------------------
ffmpeg_video_settings
::ffmpeg_video_settings( size_t width, size_t height, AVRational frame_rate )
  : width{ width },
    height{ height },
    codec_id{ AV_CODEC_ID_H264 },
    pixel_format{ AV_PIX_FMT_RGB24 },
    frame_rate( frame_rate ),
    time_base{ frame_rate.den, frame_rate.num },
    sample_aspect_ratio{ 1, 1 },
    bit_rate{ 0 },
    gop_size{ 0 },
    profile{ 0 },
    level{ 0 },
    stream_id{ 0 } {}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
