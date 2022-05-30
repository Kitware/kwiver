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
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <memory>
#include <stdexcept>
#include <string>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
std::string error_string( int error_code );

// ----------------------------------------------------------------------------
#define DECLARE_PTRS( LOWER, UPPER )                                     \
struct _ ## LOWER ## _deleter { void operator()( UPPER* ptr ) const; };  \
using LOWER ## _uptr = std::unique_ptr< UPPER, _ ## LOWER ## _deleter >; \

DECLARE_PTRS( format_context, AVFormatContext )
DECLARE_PTRS( codec_context, AVCodecContext )
DECLARE_PTRS( codec_parameters, AVCodecParameters )
DECLARE_PTRS( packet, AVPacket )
DECLARE_PTRS( frame, AVFrame )
DECLARE_PTRS( filter_graph, AVFilterGraph )
DECLARE_PTRS( filter_in_out, AVFilterInOut )
DECLARE_PTRS( sws_context, SwsContext )

#undef DECLARE_PTRS

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
