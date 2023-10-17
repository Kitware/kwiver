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

  int index;
  codec_parameters_uptr parameters;
  AVRational time_base;
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
