// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of FFmpeg image conversion utilities.

#include <arrows/ffmpeg/ffmpeg_convert_image.h>

#include <vital/logger/logger.h>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

namespace {

// ----------------------------------------------------------------------------
// Some libav algorithms use vectorized operations, which requires some extra
// dead memory at the end of buffers, as well as memory alignment
constexpr size_t padding = AV_INPUT_BUFFER_PADDING_SIZE;

// ----------------------------------------------------------------------------
// Interpretation of vital images from 1-4 channels
AVPixelFormat depth_pix_fmts[] = {
  AV_PIX_FMT_GRAY8,  // Grayscale
  AV_PIX_FMT_GRAY8A, // Grayscale with alpha
  AV_PIX_FMT_RGB24,  // RGB
  AV_PIX_FMT_RGBA,   // RGB with alpha
  AV_PIX_FMT_NONE, };

// ----------------------------------------------------------------------------
// JPEG versions of YUV formats are deprecated and cause warnings when used
AVPixelFormat
dejpeg_pix_fmt( AVPixelFormat format )
{
  switch( format )
  {
    case AV_PIX_FMT_YUVJ411P: return AV_PIX_FMT_YUV411P;
    case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
    case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
    case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
    default:
      return format;
  }
}

// ----------------------------------------------------------------------------
// All YUV formats except JPEG versions default to MPEG limited color range
AVColorRange color_range_from_pix_fmt( AVPixelFormat format )
{
  switch( format )
  {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUYV422:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUV410P:
    case AV_PIX_FMT_YUV411P:
    case AV_PIX_FMT_UYVY422:
    case AV_PIX_FMT_UYYVYY411:
    case AV_PIX_FMT_NV12:
    case AV_PIX_FMT_NV21:
    case AV_PIX_FMT_YUV440P:
    case AV_PIX_FMT_YUVA420P:
    case AV_PIX_FMT_YUV420P16LE:
    case AV_PIX_FMT_YUV420P16BE:
    case AV_PIX_FMT_YUV422P16LE:
    case AV_PIX_FMT_YUV422P16BE:
    case AV_PIX_FMT_YUV444P16LE:
    case AV_PIX_FMT_YUV444P16BE:
    case AV_PIX_FMT_YUV420P9BE:
    case AV_PIX_FMT_YUV420P9LE:
    case AV_PIX_FMT_YUV420P10BE:
    case AV_PIX_FMT_YUV420P10LE:
    case AV_PIX_FMT_YUV422P10BE:
    case AV_PIX_FMT_YUV422P10LE:
    case AV_PIX_FMT_YUV444P9BE:
    case AV_PIX_FMT_YUV444P9LE:
    case AV_PIX_FMT_YUV444P10BE:
    case AV_PIX_FMT_YUV444P10LE:
    case AV_PIX_FMT_YUV422P9BE:
    case AV_PIX_FMT_YUV422P9LE:
    case AV_PIX_FMT_YUVA422P:
    case AV_PIX_FMT_YUVA444P:
    case AV_PIX_FMT_YUVA420P9BE:
    case AV_PIX_FMT_YUVA420P9LE:
    case AV_PIX_FMT_YUVA422P9BE:
    case AV_PIX_FMT_YUVA422P9LE:
    case AV_PIX_FMT_YUVA444P9BE:
    case AV_PIX_FMT_YUVA444P9LE:
    case AV_PIX_FMT_YUVA420P10BE:
    case AV_PIX_FMT_YUVA420P10LE:
    case AV_PIX_FMT_YUVA422P10BE:
    case AV_PIX_FMT_YUVA422P10LE:
    case AV_PIX_FMT_YUVA444P10BE:
    case AV_PIX_FMT_YUVA444P10LE:
    case AV_PIX_FMT_YUVA420P16BE:
    case AV_PIX_FMT_YUVA420P16LE:
    case AV_PIX_FMT_YUVA422P16BE:
    case AV_PIX_FMT_YUVA422P16LE:
    case AV_PIX_FMT_YUVA444P16BE:
    case AV_PIX_FMT_YUVA444P16LE:
      return AVCOL_RANGE_MPEG;
    default:
      return AVCOL_RANGE_JPEG;
  }
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
AVPixelFormat
pix_fmt_from_depth( size_t depth )
{
  if( !depth || depth > 4 )
  {
    throw_error( "Unsupported depth: ", depth );
  }
  return depth_pix_fmts[ depth - 1 ];
}

// ----------------------------------------------------------------------------
size_t
depth_from_pix_fmt( AVPixelFormat pix_fmt )
{
  auto const depth_pix_fmt =
    avcodec_find_best_pix_fmt_of_list( depth_pix_fmts, pix_fmt, true, nullptr );

  for( size_t i = 0; i < 4; ++i )
  {
    if( depth_pix_fmts[ i ] == depth_pix_fmt )
    {
      return i + 1;
    }
  }

  return 3;  // This should never happen, but if it does, default to RGB
}

// ----------------------------------------------------------------------------
vital::image_container_sptr
frame_to_vital_image( AVFrame* frame, sws_context_uptr* cached_sws )
{
  throw_error_null( frame, "frame_to_vital_image() given null frame" );

  // Determine pixel formats
  auto const src_pix_fmt =
    dejpeg_pix_fmt( static_cast< AVPixelFormat >( frame->format ) );
  auto const depth = depth_from_pix_fmt( src_pix_fmt );
  auto const dst_pix_fmt = pix_fmt_from_depth( depth );

  // Allocate memory of correct size
  auto const width = static_cast< size_t >( frame->width );
  auto const height = static_cast< size_t >( frame->height );
  auto const linesize = width * depth;
  auto const image_size = linesize * height + padding;
  auto const image_memory =
    std::make_shared< vital::image_memory >( image_size );

  // Create pixel format converter
  sws_context_uptr tmp_sws;
  if( !cached_sws )
  {
    cached_sws = &tmp_sws;
  }

  cached_sws->reset(
    throw_error_null(
      sws_getCachedContext(
        cached_sws->release(),
        width, height, src_pix_fmt,
        width, height, dst_pix_fmt,
        SWS_BICUBIC, nullptr, nullptr, nullptr ),
      "Could not create image conversion context" ) );

  if( frame->color_range == AVCOL_RANGE_UNSPECIFIED )
  {
    // Not using the de-JPEG'd src_pix_fmt
    frame->color_range = color_range_from_pix_fmt(
      static_cast< AVPixelFormat >( frame->format ) );
  }

  if( sws_setColorspaceDetails(
        cached_sws->get(), sws_getCoefficients( frame->colorspace ),
        frame->color_range == AVCOL_RANGE_JPEG,
        sws_getCoefficients( SWS_CS_DEFAULT ), 1, 0, 1 << 16, 1 << 16 ) < 0 )
  {
    LOG_WARN(
      vital::get_logger( "ffmpeg" ),
      "Could not convert to standardized colorspace; "
      "image will be decoded as-is" );
  }

  // Convert pixel format
  auto const out_data = static_cast< uint8_t* >( image_memory->data() );
  auto const out_linesize = static_cast< int >( linesize );
  if( sws_scale(
        cached_sws->get(),
        frame->data, frame->linesize,
        0, height,
        &out_data, &out_linesize ) != static_cast< int >( height ) )
  {
    throw_error( "Could not convert image to vital pixel format" );
  }

  return std::make_shared< vital::simple_image_container >(
    vital::image(
      image_memory, image_memory->data(),
      width, height, depth,
      depth, linesize, 1 ) );
}

// ----------------------------------------------------------------------------
frame_uptr
vital_image_to_frame(
  vital::image_container_scptr const& image,
  AVCodecContext const* codec_context, sws_context_uptr* cached_sws )
{
  if( !image )
  {
    throw_error( "vital_image_to_frame() given null image" );
  }

  if( image->get_image().pixel_traits() !=
      vital::image_pixel_traits_of< uint8_t >() )
  {
    // TODO: Is there an existing conversion function somewhere?
    throw_error( "Image has unsupported pixel traits (non-uint8)" );
  }

  // Create frame object for incoming image
  frame_uptr frame{
    throw_error_null( av_frame_alloc(), "Could not allocate frame" ) };

  // Determine image dimensions
  frame->width = image->width();
  frame->height = image->height();

  auto const src_pix_fmt = pix_fmt_from_depth( image->depth() );
  frame->format = src_pix_fmt;

  auto const dst_pix_fmt =
    codec_context ? codec_context->pix_fmt : AV_PIX_FMT_NONE;

  throw_error_code(
    av_frame_get_buffer( frame.get(), padding ),
    "Could not allocate frame data" );

  // Give the frame the raw pixel data
  {
    auto ptr =
      static_cast< uint8_t const* >( image->get_image().first_pixel() );
    auto const i_step = image->get_image().h_step();
    auto const j_step = image->get_image().w_step();
    auto const k_step = image->get_image().d_step();
    if( j_step == static_cast< ptrdiff_t >( image->depth() ) &&
        k_step == static_cast< ptrdiff_t >( 1 ) )
    {
      // Fast method for packed / interleaved data - line by line
      for( size_t i = 0; i < image->height(); ++i )
      {
        std::memcpy(
          frame->data[ 0 ] + i * frame->linesize[ 0 ], ptr + i * i_step,
          image->width() * image->depth() );
      }
    }
    // TODO: Add faster method to copy planar data
    else
    {
      // Slow method - pixel by pixel
      auto const i_step_ptr = i_step - j_step * image->width();
      auto const j_step_ptr = j_step - k_step * image->depth();
      auto const k_step_ptr = k_step;
      auto const i_step_index =
        frame->linesize[ 0 ] - image->width() * image->depth();
      size_t index = 0;
      for( size_t i = 0; i < image->height(); ++i )
      {
        for( size_t j = 0; j < image->width(); ++j )
        {
          for( size_t k = 0; k < image->depth(); ++k )
          {
            frame->data[ 0 ][ index++ ] = *ptr;
            ptr += k_step_ptr;
          }
          ptr += j_step_ptr;
        }
        ptr += i_step_ptr;
        index += i_step_index;
      }
    }
  }

  if( dst_pix_fmt == AV_PIX_FMT_NONE || dst_pix_fmt == frame->format )
  {
    // No need to convert the frame
    return frame;
  }

  // Allocate a new frame with the desire pixel format
  frame_uptr converted_frame{
    throw_error_null( av_frame_alloc(), "Could not allocate frame" ) };

  converted_frame->width = frame->width;
  converted_frame->height = frame->height;
  converted_frame->format = dejpeg_pix_fmt( dst_pix_fmt );
  if( codec_context->color_range == AVCOL_RANGE_UNSPECIFIED )
  {
    // Not using the de-JPEG'd converted_frame->format
    converted_frame->color_range = color_range_from_pix_fmt( dst_pix_fmt );
  }
  else
  {
    converted_frame->color_range = codec_context->color_range;
  }
  converted_frame->colorspace = codec_context->colorspace;
  converted_frame->color_trc = codec_context->color_trc;
  converted_frame->color_primaries = codec_context->color_primaries;

  throw_error_code(
    av_frame_get_buffer( converted_frame.get(), padding ),
    "Could not allocate frame data" );

  // Create pixel format converter
  sws_context_uptr tmp_sws;
  if( !cached_sws )
  {
    cached_sws = &tmp_sws;
  }

  cached_sws->reset(
    throw_error_null(
      sws_getCachedContext(
        cached_sws->release(),
        frame->width, frame->height, src_pix_fmt,
        frame->width, frame->height,
        static_cast< AVPixelFormat >( converted_frame->format ),
        SWS_BICUBIC, nullptr, nullptr, nullptr ),
      "Could not create image conversion context" ) );

  if( sws_setColorspaceDetails(
        cached_sws->get(), sws_getCoefficients( SWS_CS_DEFAULT ), 1,
        sws_getCoefficients( converted_frame->colorspace ),
        converted_frame->color_range == AVCOL_RANGE_JPEG,
        0, 1 << 16, 1 << 16 ) < 0 )
  {
    LOG_WARN(
      vital::get_logger( "ffmpeg" ),
      "Could not convert to desired colorspace; "
      "image will be encoded as-is" );
  }

  // Convert pixel format
  if( sws_scale(
        cached_sws->get(),
        frame->data, frame->linesize,
        0, frame->height,
        converted_frame->data, converted_frame->linesize ) != frame->height )
  {
    throw_error( "Could not convert image to target pixel format" );
  }

  return converted_frame;
}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
