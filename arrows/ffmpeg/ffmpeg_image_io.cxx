// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the FFmpeg-based image_io algorithm.

#include <arrows/ffmpeg/ffmpeg_image_io.h>

#include <arrows/ffmpeg/ffmpeg_convert_image.h>
#include <arrows/ffmpeg/ffmpeg_util.h>

extern "C" {
#include <libavutil/opt.h>
}

#include <algorithm>
#include <filesystem>
#include <map>

#include <cctype>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

namespace {

// ----------------------------------------------------------------------------
AVCodecID
extension_to_codec_id( std::string const& filename )
{
  static std::map< std::string, AVCodecID > const map = {
    { ".bmp",  AV_CODEC_ID_BMP },
    { ".j2k",  AV_CODEC_ID_JPEG2000 },
    { ".jp2",  AV_CODEC_ID_JPEG2000 },
    { ".jpeg", AV_CODEC_ID_MJPEG },
    { ".jpg",  AV_CODEC_ID_MJPEG },
    { ".png",  AV_CODEC_ID_PNG },
    { ".tga",  AV_CODEC_ID_TARGA },
    { ".tif",  AV_CODEC_ID_TIFF },
    { ".tiff", AV_CODEC_ID_TIFF },
    { ".webp", AV_CODEC_ID_WEBP }, };

  auto extension = std::filesystem::path( filename ).extension().string();
  std::transform(
    extension.begin(), extension.end(), extension.begin(),
    []( unsigned char c ){ return std::tolower( c ); } );

  if( auto const it = map.find( extension ); it != map.end() )
  {
    return it->second;
  }
  throw std::runtime_error(
    "Could not determine image format from filename: " + filename );
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
class ffmpeg_image_io::impl
{
public:
  impl();

  std::string codec_name;
  int quality;

  sws_context_uptr load_image_converter;
  sws_context_uptr save_image_converter;
};

// ----------------------------------------------------------------------------
ffmpeg_image_io::impl
::impl()
  : codec_name{},
    quality{ -1 },
    load_image_converter{},
    save_image_converter{}
{}

// ----------------------------------------------------------------------------
ffmpeg_image_io
::ffmpeg_image_io()
  : d{ new impl }
{}

// ----------------------------------------------------------------------------
ffmpeg_image_io
::~ffmpeg_image_io()
{}

// ----------------------------------------------------------------------------
vital::config_block_sptr
ffmpeg_image_io
::get_configuration() const
{
  auto config = vital::algorithm::get_configuration();

  config->set_value(
    "codec_name", d->codec_name,
    "Name of FFmpeg codec to force usage of. "
    "Only effective when saving images." );
  config->set_value(
    "quality", d->quality,
    "Integer 2-31 controlling compression quality. Higher is lossier." );

  return config;
}

// ----------------------------------------------------------------------------
void
ffmpeg_image_io
::set_configuration( vital::config_block_sptr config_in )
{
  auto config = vital::algorithm::get_configuration();
  config->merge_config( config_in );

  d->codec_name = config->get_value< std::string >( "codec_name", "" );
  d->quality = config->get_value< int >( "quality", -1 );
}

// ----------------------------------------------------------------------------
bool
ffmpeg_image_io
::check_configuration( vital::config_block_sptr ) const
{
  return true;
}

// ----------------------------------------------------------------------------
vital::image_container_sptr
ffmpeg_image_io
::load_( std::string const& filename ) const
{
  // Open the file
  format_context_uptr format_context;
  {
    AVFormatContext* ptr = nullptr;
    throw_error_code(
      avformat_open_input(
        &ptr, filename.c_str(), av_find_input_format( "image2" ), NULL ),
      "Could not open input" );
    format_context.reset( ptr );
  }

  // Get the stream information by reading a bit of the file
  throw_error_code(
    avformat_find_stream_info( format_context.get(), NULL ),
    "Could not read stream information" );

  // Find "video" (image) stream
  AVStream* video_stream = nullptr;
  for( size_t i = 0; i < format_context->nb_streams; ++i )
  {
    auto const stream = format_context->streams[ i ];
    auto const params = stream->codecpar;
    if( params->codec_type == AVMEDIA_TYPE_VIDEO &&
        params->width > 0 && params->height > 0 )
    {
      video_stream = stream;
      break;
    }
  }
  throw_error_null( video_stream, "Could not find a valid image in the file" );

  // Create an image codec
  auto const codec = throw_error_null(
    avcodec_find_decoder( video_stream->codecpar->codec_id ),
    "Could not find suitable codec"
  );
  codec_context_uptr codec_context{
    throw_error_null(
      avcodec_alloc_context3( codec ),
      "Could not allocate codec context" ) };

  // Configure the codec
  throw_error_code(
    avcodec_parameters_to_context(
      codec_context.get(), video_stream->codecpar ),
    "Could not configure codec ", pretty_codec_name( codec ) );
  codec_context->thread_count = 0;
  codec_context->thread_type = FF_THREAD_SLICE;

  // Initialize the codec
  throw_error_code(
    avcodec_open2( codec_context.get(), codec, nullptr ),
    "Could not open codec ", pretty_codec_name( codec ) );

  // Get raw image data
  packet_uptr packet{
    throw_error_null( av_packet_alloc(), "Could not allocate packet" ) };
  throw_error_code(
    av_read_frame( format_context.get(), packet.get() ),
    "Could not parse image" );

  // Give data to the decoder
  throw_error_code(
    avcodec_send_packet( codec_context.get(), packet.get() ),
    "Could not send image to decoder" );
  throw_error_code(
    avcodec_send_packet( codec_context.get(), nullptr ),
    "Could not flush image decoder" );

  // Get the decoded frame
  frame_uptr frame{
    throw_error_null( av_frame_alloc(), "Could not allocate frame" ) };
  throw_error_code(
    avcodec_receive_frame( codec_context.get(), frame.get() ),
    "Could not decode image" );

  return frame_to_vital_image( frame.get(), &d->load_image_converter );
}

// ----------------------------------------------------------------------------
void
ffmpeg_image_io
::save_(
  std::string const& filename,
  kwiver::vital::image_container_sptr data ) const
{
  if( !data || !data->width() || !data->height() || !data->depth() )
  {
    throw_error( "Empty image given to ffmpeg_image_io.save()" );
  }

  // Allocate output format context
  format_context_uptr format_context;
  {
    AVFormatContext* tmp = nullptr;
    throw_error_code(
      avformat_alloc_output_context2(
        &tmp, nullptr, "image2", filename.c_str() ),
      "Could not allocate format context" );
    format_context.reset( tmp );
  }

  // Force FFmpeg to treat the output as a single image (not a sequence)
  av_opt_set_int( format_context->priv_data, "update", 1, 0 );

  // Choose image codec
  auto const codec =
    throw_error_null(
      d->codec_name.empty()
      ? avcodec_find_encoder( extension_to_codec_id( filename ) )
      : avcodec_find_encoder_by_name( d->codec_name.c_str() ),
      "Could not find suitable encoder"
    );

  // Create codec context
  codec_context_uptr codec_context{
    throw_error_null(
      avcodec_alloc_context3( codec ),
      "Could not allocate codec context" ) };

  // Configure codec
  codec_context->width = data->width();
  codec_context->height = data->height();
  codec_context->time_base = AV_TIME_BASE_Q;
  if( d->quality >= 0 )
  {
    codec_context->flags |= AV_CODEC_FLAG_QSCALE;
    codec_context->global_quality = d->quality * FF_QP2LAMBDA;
    codec_context->qmin = codec_context->qmax = d->quality;
  }
  codec_context->color_range = AVCOL_RANGE_JPEG;

  // Determine which pixel format to use
  auto const src_pix_fmt = pix_fmt_from_depth( data->depth() );
  codec_context->pix_fmt =
    avcodec_find_best_pix_fmt_of_list(
      codec->pix_fmts, src_pix_fmt, src_pix_fmt == AV_PIX_FMT_RGBA,
      nullptr );

  // Create the "video" (image) stream
  AVStream* video_stream = throw_error_null(
    avformat_new_stream( format_context.get(), nullptr ),
    "Could not allocate image stream" );
  throw_error_code(
    avcodec_parameters_from_context(
      video_stream->codecpar, codec_context.get() ),
    "Could not configure image stream" );

  // Open the output
  throw_error_code(
    avcodec_open2( codec_context.get(), codec, nullptr ),
    "Could not initialize codec" );
  throw_error_code(
    avio_open( &format_context->pb, filename.c_str(), AVIO_FLAG_WRITE ),
    "Could not open image file ", filename );

  // Start writing the file
  throw_error_code(
    avformat_write_header( format_context.get(), nullptr ),
    "Could not write image header" );

  // Convert input image to FFmpeg frame
  auto const frame =
    vital_image_to_frame(
      data,
      codec_context.get(),
      &d->save_image_converter );
  frame->pts = 0;

  // Encode frame
  throw_error_code(
    avcodec_send_frame( codec_context.get(), frame.get() ),
    "Encoder rejected image" );
  throw_error_code(
    avcodec_send_frame( codec_context.get(), nullptr ),
    "Could not flush encoder" );

  // Get encoded frame
  packet_uptr packet{
    throw_error_null( av_packet_alloc(), "Could not allocate packet" ) };
  throw_error_code(
    avcodec_receive_packet( codec_context.get(), packet.get() ),
    "Could not encode image" );

  // Write out frame and close out file
  throw_error_code(
    av_write_frame( format_context.get(), packet.get() ),
    "Could not write image to file" );
  throw_error_code(
    av_write_trailer( format_context.get() ),
    "Could not write image trailer" );
}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
