// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declaration of FFmpeg video settings.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_SETTINGS_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_SETTINGS_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <vital/types/video_settings.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace kwiver {

namespace arrows {

namespace ffmpeg {

namespace ffmpeg_detail {

struct avcodec_parameters_deleter
{
  void
  operator()( AVCodecParameters* ptr ) const
  {
    avcodec_parameters_free( &ptr );
  }
};

using avcodec_parameters_uptr =
  std::unique_ptr< AVCodecParameters, avcodec_parameters_deleter >;

} // namespace ffmpeg_detail

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_settings
  : public vital::video_settings
{
  ffmpeg_video_settings();

  ffmpeg_video_settings( size_t width, size_t height, AVRational frame_rate );

  AVRational frame_rate;
  ffmpeg_detail::avcodec_parameters_uptr parameters;
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
