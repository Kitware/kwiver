// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of FFmpeg internal utility classes and functions.

#include "ffmpeg_util.h"

#include <vital/logger/logger.h>

#include <array>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
std::string
error_string( int error_code )
{
  std::array< char, AV_ERROR_MAX_STRING_SIZE > buffer = { 0 };
  av_strerror( error_code, buffer.data(), buffer.size() );
  return buffer.data();
}

// ----------------------------------------------------------------------------
#define DEFINE_DELETER( LOWER, UPPER ) \
  void _ ## LOWER ## _deleter::operator()( UPPER* ptr ) const

// ----------------------------------------------------------------------------
DEFINE_DELETER( format_context, AVFormatContext )
{
  // Close input if present
  if( ptr->iformat )
  {
    avformat_close_input( &ptr );
  }

  // Close output if present
  if( ptr && ptr->pb )
  {
    auto const err = avio_closep( &ptr->pb );
    if( err < 0 )
    {
      LOG_ERROR(
        kv::get_logger( "ffmpeg" ),
        "Could not close I/O file: " << error_string( err ) );
    }
  }

  avformat_free_context( ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( codec_context, AVCodecContext )
{
  avcodec_flush_buffers( ptr );
  avcodec_free_context( &ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( codec_parameters, AVCodecParameters )
{
  avcodec_parameters_free( &ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( packet, AVPacket )
{
  av_packet_free( &ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( frame, AVFrame )
{
  av_frame_free( &ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( filter_graph, AVFilterGraph )
{
  avfilter_graph_free( &ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( filter_in_out, AVFilterInOut )
{
  avfilter_inout_free( &ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( sws_context, SwsContext )
{
  sws_freeContext( ptr );
}

#undef DEFINE_DELETER

} // namespace ffmpeg

} // namespace vital

} // namespace kwiver
