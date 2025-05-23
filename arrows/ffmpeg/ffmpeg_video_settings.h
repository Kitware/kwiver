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
/// Parameters defining the desired characteristics of a video file.
///
/// This struct will be filled in by ffmpeg_video_input when transcoding, or
/// by the user when created a new video from scratch. Members have been left
/// public so the user may modify them before passing to ffmpeg_video_output.
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

  /// Desired frame rate of the video. Must be set in most cases.
  AVRational frame_rate;

  /// FFmpeg's parameters determining how the video codec is set up. Notably,
  /// height and width must be set before opening a video.
  codec_parameters_uptr parameters;

  /// Settings for each KLV stream to be inserted.
  std::vector< klv::klv_stream_settings > klv_streams;

  /// Settings for each audio stream to be inserted.
  std::vector< ffmpeg_audio_stream_settings > audio_streams;

  /// Time base of the video stream in the input video, if transcoding. Not
  /// guaranteed to determine the time base in the output video.
  AVRational time_base;

  /// Start time of the input video, in AV_TIME_BASE units (microseconds).
  /// This information is necessary for copied and newly-encoded packets to sync
  /// correctly.
  int64_t start_timestamp;

  /// FFmpeg-defined string options passed to the video codec.
  std::map< std::string, std::string > codec_options;
};
using ffmpeg_video_settings_uptr = std::unique_ptr< ffmpeg_video_settings >;

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
