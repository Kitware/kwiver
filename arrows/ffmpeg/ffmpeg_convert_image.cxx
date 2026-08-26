// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of FFmpeg image conversion utilities.

#include <arrows/ffmpeg/ffmpeg_convert_image.h>

#include <vital/logger/logger.h>

extern "C" {
#include <libavutil/pixdesc.h>
}

#include <algorithm>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

namespace {

// ----------------------------------------------------------------------------
// Some libav algorithms use vectorized operations, which requires some extra
// dead memory at the end of buffers, as well as memory alignment.
constexpr size_t padding = AV_INPUT_BUFFER_PADDING_SIZE;

// ----------------------------------------------------------------------------
// JPEG versions of YUV formats are deprecated and cause warnings when used.
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
// All YUV formats except JPEG versions default to MPEG limited color range.
AVColorRange
color_range_from_pix_fmt( AVPixelFormat format )
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

// ----------------------------------------------------------------------------
// All FFmpeg planar formats use GBR(A) ordering, not RGB(A).
size_t
gbr_index( size_t index, size_t depth )
{
  if( depth == 3 || depth == 4 )
  {
    switch( index )
    {
      case 0: return 2;
      case 1: return 0;
      case 2: return 1;
      case 3: return 3;
      default: break;
    }
  }

  return index;
}

// ----------------------------------------------------------------------------
// Boolean images require special 8 -> 1 bit conversion.
void
bool_image_to_bool_frame( vital::image const& image, AVFrame* frame )
{
  auto ptr = static_cast< bool const* >( image.first_pixel() );

  auto const i_step_ptr = image.h_step() - image.w_step() * image.width();
  auto const i_step_index =
    frame->linesize[ 0 ] - ( image.width() + 7 ) / 8;

  size_t index = 0;
  for( size_t i = 0; i < image.height(); ++i )
  {
    uint8_t byte = 0;
    uint8_t byte_index = 7;
    for( size_t j = 0; j < image.width(); ++j )
    {
      if( *ptr )
      {
        byte |= 1 << byte_index;
      }

      if( byte_index )
      {
        --byte_index;
      }
      else
      {
        // Write filled byte
        frame->data[ 0 ][ index++ ] = byte;
        byte = 0;
        byte_index = 7;
      }

      ++ptr;
    }

    // Write any remaining partially-filled byte
    if( byte_index != 7 )
    {
      frame->data[ 0 ][ index++ ] = byte;
    }

    ptr += i_step_ptr;
    index += i_step_index;
  }
}

// ----------------------------------------------------------------------------
// Copy pixel by pixel (slow).
void
pixelwise_image_to_packed_frame( vital::image const& image, AVFrame* frame )
{
  auto ptr = static_cast< uint8_t const* >( image.first_pixel() );
  auto const byte_width = image.pixel_traits().num_bytes;

  auto const i_step_ptr =
    ( image.h_step() - image.w_step() * image.width() ) * byte_width;
  auto const j_step_ptr =
    ( image.w_step() - image.d_step() * image.depth() ) * byte_width;
  auto const k_step_ptr = image.d_step() * byte_width;
  auto const i_step_index =
    frame->linesize[ 0 ] - ( image.width() * image.depth() ) * byte_width;

  size_t index = 0;
  for( size_t i = 0; i < image.height(); ++i )
  {
    for( size_t j = 0; j < image.width(); ++j )
    {
      for( size_t k = 0; k < image.depth(); ++k )
      {
        for( size_t m = 0; m < byte_width; ++m )
        {
          frame->data[ 0 ][ index + m ] = ptr[ m ];
        }
        index += byte_width;
        ptr += k_step_ptr;
      }
      ptr += j_step_ptr;
    }
    ptr += i_step_ptr;
    index += i_step_index;
  }
}

// ----------------------------------------------------------------------------
// Copy pixel by pixel (slow).
void
pixelwise_image_to_planar_frame( vital::image const& image, AVFrame* frame )
{
  auto ptr = static_cast< uint8_t const* >( image.first_pixel() );
  auto const byte_width = image.pixel_traits().num_bytes;

  // Copy pixel by pixel (slow)
  auto const i_step_ptr =
    ( image.d_step() - image.h_step() * image.height() ) * byte_width;
  auto const j_step_ptr =
    ( image.h_step() - image.w_step() * image.width() ) * byte_width;
  auto const k_step_ptr = image.w_step() * byte_width;

  for( size_t i = 0; i < image.depth(); ++i )
  {
    size_t index = 0;
    auto gbr_i = gbr_index( i, image.depth() );
    for( size_t j = 0; j < image.height(); ++j )
    {
      for( size_t k = 0; k < image.width(); ++k )
      {
        for( size_t m = 0; m < byte_width; ++m )
        {
          frame->data[ gbr_i ][ index + m ] = ptr[ m ];
        }
        index += byte_width;
        ptr += k_step_ptr;
      }
      ptr += j_step_ptr;
    }
    ptr += i_step_ptr;
  }
}

// ----------------------------------------------------------------------------
// Faster copy when we don't need to switch between packed and planar.
void
packed_image_to_packed_frame( vital::image const& image, AVFrame* frame )
{
  auto ptr = static_cast< uint8_t const* >( image.first_pixel() );
  auto const byte_width = image.pixel_traits().num_bytes;

  if( static_cast< size_t >( image.h_step() ) ==
      image.width() * image.w_step() &&
      static_cast< size_t >( frame->linesize[ 0 ] ) ==
      image.width() * image.depth() * byte_width )
  {
    // Copy entire image
    std::memcpy(
      frame->data[ 0 ], ptr,
      image.height() * image.width() * image.depth() * byte_width );
  }
  else
  {
    // Copy line by line
    for( size_t i = 0; i < image.height(); ++i )
    {
      std::memcpy(
        frame->data[ 0 ] + i * frame->linesize[ 0 ],
        ptr + i * image.h_step() * byte_width,
        image.width() * image.depth() * byte_width );
    }
  }
}

// ----------------------------------------------------------------------------
void
packed_image_to_planar_frame( vital::image const& image, AVFrame* frame )
{
  pixelwise_image_to_planar_frame( image, frame );
}

// ----------------------------------------------------------------------------
// Faster copy when we don't need to switch between packed and planar.
void
planar_image_to_planar_frame( vital::image const& image, AVFrame* frame )
{
  auto ptr = static_cast< uint8_t const* >( image.first_pixel() );
  auto const byte_width = image.pixel_traits().num_bytes;

  if( static_cast< size_t >( image.h_step() ) ==
      image.width() * image.w_step() &&
      static_cast< size_t >( frame->linesize[ 0 ] ) ==
      image.width() * image.depth() * byte_width )
  {
    // Copy each plane
    for( size_t i = 0; i < image.depth(); ++i )
    {
      auto gbr_i = gbr_index( i, image.depth() );
      std::memcpy(
        frame->data[ gbr_i ], ptr + i * image.d_step() * byte_width,
        image.height() * image.width() * byte_width );
    }
  }
  else
  {
    // Copy line by line
    for( size_t i = 0; i < image.depth(); ++i )
    {
      auto gbr_i = gbr_index( i, image.depth() );
      for( size_t j = 0; j < image.height(); ++j )
      {
        std::memcpy(
          frame->data[ gbr_i ] + j * frame->linesize[ gbr_i ],
          ptr + ( i * image.d_step() + j * image.h_step() ) * byte_width,
          image.width() * byte_width );
      }
    }
  }
}

// ----------------------------------------------------------------------------
void
planar_image_to_packed_frame( vital::image const& image, AVFrame* frame  )
{
  pixelwise_image_to_packed_frame( image, frame );
}

// ----------------------------------------------------------------------------
size_t
depth_from_pix_fmt( AVPixelFormat pix_fmt )
{
  auto const descriptor = av_pix_fmt_desc_get( pix_fmt );
  if( !descriptor )
  {
    throw_error( "depth_from_pix_fmt() given invalid pix_fmt" );
  }
  return descriptor->nb_components;
}

// ----------------------------------------------------------------------------
size_t
byte_width_from_pix_fmt( AVPixelFormat pix_fmt )
{
  auto const descriptor = av_pix_fmt_desc_get( pix_fmt );
  if( !descriptor )
  {
    throw_error( "byte_width_from_pix_fmt() given invalid pix_fmt" );
  }

  size_t result = 1;
  for( uint8_t i = 0; i < descriptor->nb_components; ++i )
  {
    if( descriptor->comp[ i ].depth <= 0 )
    {
      continue;
    }

    auto const bit_depth = static_cast< size_t >( descriptor->comp[ i ].depth );
    result = std::max( result, ( bit_depth + 7 ) / 8 );
  }

  return result;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
AVPixelFormat
vital_to_frame_pix_fmt(
  size_t depth, vital::image_pixel_traits const& traits, bool prefer_planar )
{
  auto const fail =
    [](){
      throw_error(
        "Could not convert vital image to FFmpeg: "
        "the pixel format of the vital image is not supported" );
    };

  switch( traits.type )
  {
    case vital::image_pixel_traits::UNSIGNED:
      switch( depth )
      {
        case 1:
          switch( traits.num_bytes )
          {
            case 1: return AV_PIX_FMT_GRAY8;
            case 2: return AV_PIX_FMT_GRAY16;
            default: fail();
          }
          break;
        case 2:
          switch( traits.num_bytes )
          {
            case 1: return AV_PIX_FMT_YA8;
            case 2: return AV_PIX_FMT_YA16;
            default: fail();
          }
          break;
        case 3:
          switch( traits.num_bytes )
          {
            case 1:
              return prefer_planar ? AV_PIX_FMT_GBRP : AV_PIX_FMT_RGB24;
            case 2:
              return prefer_planar ? AV_PIX_FMT_GBRP16 : AV_PIX_FMT_RGB48;
            default: fail();
          }
          break;
        case 4:
          switch( traits.num_bytes )
          {
            case 1:
              return prefer_planar ? AV_PIX_FMT_GBRAP : AV_PIX_FMT_RGBA;
            case 2:
              return prefer_planar ? AV_PIX_FMT_GBRAP16 : AV_PIX_FMT_RGBA64;
            default: fail();
          }
          break;
        default:
          fail();
          break;
      }
      break;

    case vital::image_pixel_traits::BOOL:
      if( depth == 1 )
      {
        return AV_PIX_FMT_MONOBLACK;
      }
      [[fallthrough]];
    case vital::image_pixel_traits::FLOAT:
    case vital::image_pixel_traits::SIGNED:
    case vital::image_pixel_traits::UNKNOWN:
    default:
      fail();
      break;
  }

  fail();

  return AV_PIX_FMT_NONE;
}

// ----------------------------------------------------------------------------
AVPixelFormat
frame_to_vital_pix_fmt( AVPixelFormat src_fmt )
{
  static AVPixelFormat formats[] = {
    AV_PIX_FMT_GRAY8,
    AV_PIX_FMT_YA8,
    AV_PIX_FMT_RGB24,
    AV_PIX_FMT_RGBA,
    AV_PIX_FMT_GRAY16,
    AV_PIX_FMT_YA16,
    AV_PIX_FMT_RGB48,
    AV_PIX_FMT_RGBA64,
    AV_PIX_FMT_NONE, };

  return avcodec_find_best_pix_fmt_of_list( formats, src_fmt, true, nullptr );
}

// ----------------------------------------------------------------------------
vital::image_container_sptr
frame_to_vital_image(
  AVFrame* frame, sws_context_uptr* cached_sws, bool approximate )
{
  throw_error_null( frame, "frame_to_vital_image() given null frame" );

  // Determine pixel formats
  auto const src_pix_fmt =
    dejpeg_pix_fmt( static_cast< AVPixelFormat >( frame->format ) );
  auto const dst_pix_fmt = frame_to_vital_pix_fmt( src_pix_fmt );
  auto const depth = depth_from_pix_fmt( dst_pix_fmt );
  auto const byte_width = byte_width_from_pix_fmt( dst_pix_fmt );
  auto const is_bool =
    src_pix_fmt == AV_PIX_FMT_MONOWHITE ||
    src_pix_fmt == AV_PIX_FMT_MONOBLACK;
  auto const pixel_traits =
    is_bool
    ? vital::image_pixel_traits_of< bool >()
    : vital::image_pixel_traits(
      vital::image_pixel_traits::UNSIGNED, byte_width );

  // Allocate memory of correct size
  auto const width = static_cast< size_t >( frame->width );
  auto const height = static_cast< size_t >( frame->height );
  auto const linesize = width * depth * byte_width;
  auto const image_size = linesize * height + padding;
  auto const image_memory =
    std::make_shared< vital::image_memory >( image_size );

  // Create pixel format converter
  sws_context_uptr tmp_sws;
  if( !cached_sws )
  {
    cached_sws = &tmp_sws;
  }

  int flags = SWS_POINT;
  if( !approximate )
  {
    flags |=
      SWS_ACCURATE_RND |
      SWS_BITEXACT |
      SWS_FULL_CHR_H_INT |
      SWS_FULL_CHR_H_INP;
  }

  cached_sws->reset(
    throw_error_null(
      sws_getCachedContext(
        cached_sws->release(),
        width, height, src_pix_fmt,
        width, height, dst_pix_fmt,
        flags, nullptr, nullptr, nullptr ),
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

  if( linesize % byte_width != 0 )
  {
    throw_error( "Cannot construct vital image due to alignment issues" );
  }

  auto result = vital::image(
    image_memory, image_memory->data(),
    width, height, depth,
    depth, linesize / byte_width, 1,
    pixel_traits );

  if( is_bool )
  {
    static_assert( sizeof( bool ) == sizeof( uint8_t ) );

    auto const ptr = static_cast< uint8_t* >( image_memory->data() );
    for( size_t i = 0; i < image_memory->size(); ++i )
    {
      ptr[ i ] = static_cast< bool >( ptr[ i ] );
    }
  }

  return std::make_shared< vital::simple_image_container >( result );
}

// ----------------------------------------------------------------------------
frame_uptr
vital_image_to_frame(
  vital::image_container_scptr const& image,
  AVCodecContext const* codec_context, sws_context_uptr* cached_sws,
  bool approximate )
{
  if( !image )
  {
    throw_error( "vital_image_to_frame() given null image" );
  }

  // Create frame object for incoming image
  frame_uptr frame{
    throw_error_null( av_frame_alloc(), "Could not allocate frame" ) };

  // Determine image dimensions
  frame->width = image->width();
  frame->height = image->height();

  auto const src_pix_fmt =
    vital_to_frame_pix_fmt(
      image->depth(),
      image->get_image().pixel_traits(),
      is_image_planar( image->get_image() ) );
  frame->format = src_pix_fmt;

  auto dst_pix_fmt = AV_PIX_FMT_NONE;

  if( codec_context )
  {
    dst_pix_fmt = codec_context->pix_fmt;
    if( codec_context->color_range == AVCOL_RANGE_UNSPECIFIED )
    {
      // Not using the de-JPEG'd converted_frame->format
      frame->color_range = color_range_from_pix_fmt( dst_pix_fmt );
    }
    else
    {
      frame->color_range = codec_context->color_range;
    }
    frame->colorspace = codec_context->colorspace;
    frame->color_trc = codec_context->color_trc;
    frame->color_primaries = codec_context->color_primaries;
  }

  throw_error_code(
    av_frame_get_buffer( frame.get(), padding ),
    "Could not allocate frame data" );

  // Give the frame the raw pixel data
  if( src_pix_fmt == AV_PIX_FMT_MONOBLACK )
  {
    bool_image_to_bool_frame( image->get_image(), frame.get() );
  }
  else if( is_image_planar( image->get_image() ) )
  {
    if( is_pix_fmt_planar( src_pix_fmt ) )
    {
      planar_image_to_planar_frame( image->get_image(), frame.get() );
    }
    else
    {
      planar_image_to_packed_frame( image->get_image(), frame.get() );
    }
  }
  else if( is_image_packed( image->get_image() ) )
  {
    if( is_pix_fmt_planar( src_pix_fmt ) )
    {
      packed_image_to_planar_frame( image->get_image(), frame.get() );
    }
    else
    {
      packed_image_to_packed_frame( image->get_image(), frame.get() );
    }
  }
  else
  {
    if( is_pix_fmt_planar( src_pix_fmt ) )
    {
      pixelwise_image_to_planar_frame( image->get_image(), frame.get() );
    }
    else
    {
      pixelwise_image_to_packed_frame( image->get_image(), frame.get() );
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
  av_frame_copy_props( converted_frame.get(), frame.get() );

  throw_error_code(
    av_frame_get_buffer( converted_frame.get(), padding ),
    "Could not allocate frame data" );

  // Create pixel format converter
  sws_context_uptr tmp_sws;
  if( !cached_sws )
  {
    cached_sws = &tmp_sws;
  }

  int flags = SWS_POINT;
  if( !approximate )
  {
    flags |=
      SWS_ACCURATE_RND |
      SWS_BITEXACT |
      SWS_FULL_CHR_H_INT |
      SWS_FULL_CHR_H_INP;
  }

  cached_sws->reset(
    throw_error_null(
      sws_getCachedContext(
        cached_sws->release(),
        frame->width, frame->height, src_pix_fmt,
        frame->width, frame->height,
        static_cast< AVPixelFormat >( converted_frame->format ),
        flags, nullptr, nullptr, nullptr ),
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

// ----------------------------------------------------------------------------
bool
is_image_planar( vital::image const& image )
{
  return
    image.depth() > 1 &&
    std::abs( image.d_step() ) >= image.h_step() &&
    image.h_step() >=
    static_cast< ptrdiff_t >( image.width() * image.w_step() ) &&
    image.w_step() == 1;
}

// ----------------------------------------------------------------------------
bool
is_image_packed( vital::image const& image )
{
  return
    image.h_step() >=
    static_cast< ptrdiff_t >( image.width() * image.w_step() ) &&
    image.w_step() ==
    static_cast< ptrdiff_t >( image.depth() * image.d_step() ) &&
    image.d_step() == 1;
}

// ----------------------------------------------------------------------------
bool
is_pix_fmt_planar( AVPixelFormat pix_fmt )
{
  auto const descriptor =
    throw_error_null(
      av_pix_fmt_desc_get( pix_fmt ),
      "is_pix_fmt_planar() given invalid pix_fmt" );

  return descriptor->flags & AV_PIX_FMT_FLAG_PLANAR;
}

// ----------------------------------------------------------------------------
bool
is_pix_fmt_packed( AVPixelFormat pix_fmt )
{
  return !is_pix_fmt_planar( pix_fmt );
}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
