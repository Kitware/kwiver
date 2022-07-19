// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg video settings.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_SETTINGS_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_SETTINGS_H_

#include <arrows/ffmpeg/ffmpeg_util.h>
#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <vital/types/video_settings.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <memory>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_settings
  : public vital::video_settings
{
  ffmpeg_video_settings();

  ffmpeg_video_settings(
    size_t width, size_t height,
    AVRational frame_rate,
    size_t klv_stream_count );

  ~ffmpeg_video_settings();

  AVRational frame_rate;
  codec_parameters_uptr parameters;
  size_t klv_stream_count;
};
using ffmpeg_video_settings_uptr = std::unique_ptr< ffmpeg_video_settings >;

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
