// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg audio stream settings.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_AUDIO_STREAM_SETTINGS_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_AUDIO_STREAM_SETTINGS_H_

#include <arrows/ffmpeg/ffmpeg_util.h>
#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
/// Parameters describing the general characteristics of an audio stream.
///
/// This struct will be filled in by ffmpeg_video_input, to be used by
/// ffmpeg_video_output when creating an audio stream. Members have been left
/// public so users may modify them before passing to ffmpeg_video_output.
struct KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_audio_stream_settings
{
  ffmpeg_audio_stream_settings();
  ffmpeg_audio_stream_settings( ffmpeg_audio_stream_settings const& other );
  ffmpeg_audio_stream_settings( ffmpeg_audio_stream_settings&& other );

  ~ffmpeg_audio_stream_settings();

  ffmpeg_audio_stream_settings&
  operator=( ffmpeg_audio_stream_settings const& other );
  ffmpeg_audio_stream_settings&
  operator=( ffmpeg_audio_stream_settings&& other );

  /// Index of this stream in the input video. Does not determine the index in
  /// the output video.
  int index;

  /// FFmpeg's parameters determining how the audio codec is set up.
  codec_parameters_uptr parameters;

  /// Time base of this stream in the input video. Not guaranteed to determine
  /// the time base in the output video.
  AVRational time_base;
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
