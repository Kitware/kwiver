// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg internal utility classes and functions.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_UTIL_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_UTIL_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <vital/logger/logger.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
}

#include <memory>
#include <stdexcept>
#include <string>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
std::string pretty_codec_name( AVCodecID codec_id );

// ----------------------------------------------------------------------------
std::string pretty_codec_name( AVCodec const* codec );

// ----------------------------------------------------------------------------
bool is_hardware_codec( AVCodec const* codec );

// ----------------------------------------------------------------------------
std::string error_string( int error_code );

// --------------------------------------------------------------------------
template< class... Args >
inline void throw_error( Args... args )
{
  std::stringstream ss;
  bool dummy[] = { ( ss << args ).good()... };
  ( void )dummy;
  throw std::runtime_error( ss.str() );
}

// --------------------------------------------------------------------------
template< class... Args >
inline int throw_error_code( int error_code, Args... args )
{
  if( error_code < 0 )
  {
    throw_error( args..., ": ", error_string( error_code ) );
  }
  return error_code;
}

// --------------------------------------------------------------------------
template< class T, class... Args >
inline T* throw_error_null( T* ptr, Args... args )
{
  if( !ptr )
  {
    throw_error( args... );
  }
  return ptr;
}

// ----------------------------------------------------------------------------
// Wrapper around avformat_query_codec().
bool format_supports_codec( AVOutputFormat const* format, AVCodecID codec_id );

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
DECLARE_PTRS( hardware_device_context, AVBufferRef )
DECLARE_PTRS( bsf_context, AVBSFContext )

#undef DECLARE_PTRS

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
