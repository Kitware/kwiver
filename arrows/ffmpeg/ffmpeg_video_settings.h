// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg video settings.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_SETTINGS_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_SETTINGS_H_

#include <arrows/ffmpeg/ffmpeg_util.h>
#include <arrows/ffmpeg/ffmpeg_audio_stream_settings.h>
#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <arrows/klv/klv_stream_settings.h>

#include <vital/types/video_settings.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <map>
#include <memory>
#include <vector>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_settings
  : public vital::video_settings
{
  ffmpeg_video_settings();
  ffmpeg_video_settings( ffmpeg_video_settings const& other );
  ffmpeg_video_settings( ffmpeg_video_settings&& other );
  ffmpeg_video_settings(
    size_t width, size_t height,
    AVRational frame_rate,
    std::vector< klv::klv_stream_settings > const& klv_streams = {} );

  ~ffmpeg_video_settings();

  ffmpeg_video_settings&
  operator=( ffmpeg_video_settings const& other );
  ffmpeg_video_settings&
  operator=( ffmpeg_video_settings&& other );

  AVRational frame_rate;
  codec_parameters_uptr parameters;
  std::vector< klv::klv_stream_settings > klv_streams;
  std::vector< ffmpeg_audio_stream_settings > audio_streams;
  AVRational time_base;
  int64_t start_timestamp; // In AV_TIME_BASE units
  std::map< std::string, std::string > codec_options;
};
using ffmpeg_video_settings_uptr = std::unique_ptr< ffmpeg_video_settings >;

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
