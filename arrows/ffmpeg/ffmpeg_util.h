// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg internal utility classes and functions.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_UTIL_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_UTIL_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <memory>
#include <string>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
std::string error_string( int error_code );

namespace ffmpeg_detail {

// ----------------------------------------------------------------------------
struct av_packet_deleter
{
  void
  operator()( AVPacket* ptr ) const
  {
    av_packet_free( &ptr );
  }
};

using av_packet_uptr =
  std::unique_ptr< AVPacket, av_packet_deleter >;

} // namespace ffmpeg_detail

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
