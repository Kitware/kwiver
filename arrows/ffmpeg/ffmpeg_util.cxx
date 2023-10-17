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
pretty_codec_name( AVCodecID codec_id )
{
  std::stringstream ss;
  auto const info = avcodec_descriptor_get( codec_id );
  if( info )
  {
    ss << info->name << " (" << info->long_name << ")";
  }
  else
  {
    ss << "#" << codec_id << " (Unknown Codec)";
  }
  return ss.str();
}

// ----------------------------------------------------------------------------
std::string
pretty_codec_name( AVCodec const* codec )
{
  std::stringstream ss;
  if( codec )
  {
    ss << codec->name << " (" << codec->long_name << ")";
  }
  else
  {
    ss << "(Null Codec)";
  }
  return ss.str();
}

// ----------------------------------------------------------------------------
bool
is_hardware_codec( AVCodec const* codec )
{
#if LIBAVCODEC_VERSION_MAJOR > 57
  return ( codec->capabilities & AV_CODEC_CAP_HARDWARE );
#else
  // There's no reliable way to know with older versions of FFmpeg,
  // so we'll just hardcode some names here
  std::string name = codec->name;
  return
    ( name.find( "cuvid"   ) != name.npos ) ||
    ( name.find( "nvenc"   ) != name.npos ) ||
    ( name.find( "v4l2m2m" ) != name.npos ) ||
    ( name.find( "vaapi"   ) != name.npos ) ||
    ( name.find( "vdpau"   ) != name.npos );
#endif
}

// ----------------------------------------------------------------------------
std::string
error_string( int error_code )
{
  std::array< char, AV_ERROR_MAX_STRING_SIZE > buffer = { 0 };
  av_strerror( error_code, buffer.data(), buffer.size() );
  return buffer.data();
}

// ----------------------------------------------------------------------------
bool
format_supports_codec( AVOutputFormat const* format, AVCodecID codec_id )
{
  // FFmpeg isn't sure that H.264 and H.265 are supported by TS files, but
  // they are.
  if( format->name == std::string{ "mpegts" } &&
      ( codec_id == AV_CODEC_ID_H264 || codec_id == AV_CODEC_ID_H265 ) )
  {
    return true;
  }

  return avformat_query_codec( format, codec_id, FF_COMPLIANCE_NORMAL ) > 0;
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
  if( ptr->codec && ptr->codec_type == AVMEDIA_TYPE_VIDEO )
  {
    avcodec_flush_buffers( ptr );
  }
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

// ----------------------------------------------------------------------------
DEFINE_DELETER( hardware_device_context, AVBufferRef )
{
  av_buffer_unref( &ptr );
}

// ----------------------------------------------------------------------------
DEFINE_DELETER( bsf_context, AVBSFContext )
{
  av_bsf_free( &ptr );
}

#undef DEFINE_DELETER

} // namespace ffmpeg

} // namespace vital

} // namespace kwiver
