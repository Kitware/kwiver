// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definitions of CUDA/CUVID utilities for use in FFmpeg.

#ifdef KWIVER_ENABLE_FFMPEG_CUDA

#include "ffmpeg_cuda.h"

#include <vital/logger/logger.h>
#include <vital/util/interval.h>

extern "C" {
#include <libavutil/pixdesc.h>
}

#include <map>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ffmpeg {

namespace {

// ----------------------------------------------------------------------------
kv::logger_handle_t logger()
{
  return kv::get_logger( "cuda" );
}

// ----------------------------------------------------------------------------
std::string dimensions_to_string( size_t w, size_t h )
{
  return std::to_string( w ) + "x" + std::to_string( h );
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
CuvidFunctions*
cuvid_fn()
{
  static CuvidFunctions* result = nullptr;
  static auto tried = false;
  if( !tried && !result )
  {
    tried = true;
    cuvid_load_functions( &result, nullptr );
  }
  if( !result )
  {
    LOG_ERROR( logger(), "Could not load CUVID functions" );
  }
  return result;
}

// ----------------------------------------------------------------------------
NV_ENCODE_API_FUNCTION_LIST*
nvenc_fn()
{
  static NvencFunctions* fns = nullptr;
  static NV_ENCODE_API_FUNCTION_LIST result = {};
  static auto tried = false;
  static auto err = 0;
  if( !tried )
  {
    tried = true;
    nvenc_load_functions( &fns, nullptr );
    if( !fns )
    {
      return nullptr;
    }
    result.version = NV_ENCODE_API_FUNCTION_LIST_VER;
    err = fns->NvEncodeAPICreateInstance( &result );
  }
  if( err )
  {
    LOG_ERROR( logger(), "Could not load NVENC functions" );
    return nullptr;
  }
  return &result;
}

// ----------------------------------------------------------------------------
std::string
cuda_error_string( CUresult err )
{
  std::string result;
  char const* ptr;
  cuGetErrorName( err, &ptr );
  result += ptr ? ptr : "Unknown Cuda Error";
  result += ": ";
  cuGetErrorString( err, &ptr );
  result += ptr ? ptr : "No description provided";
  return result;
}

// ----------------------------------------------------------------------------
std::vector< AVCodec const* >
cuda_find_decoders( AVCodecParameters const& video_params )
{
  std::vector< AVCodec const* > result;
  AVCodec const* codec_ptr = nullptr;
#if LIBAVCODEC_VERSION_MAJOR > 57
  for( void* it = nullptr; ( codec_ptr = av_codec_iterate( &it ) ); )
#else
  while( ( codec_ptr = av_codec_next( codec_ptr ) ) )
#endif
  {
    // Only compatible NVENC encoders
    if( codec_ptr->id == video_params.codec_id &&
        av_codec_is_decoder( codec_ptr ) &&
        is_hardware_codec( codec_ptr ) &&
        !( codec_ptr->capabilities & AV_CODEC_CAP_EXPERIMENTAL ) &&
        ( std::string{ codec_ptr->name }.rfind( "_cuvid" ) !=
          std::string::npos ) )
    {
      result.emplace_back( codec_ptr );
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
std::vector< AVCodec const* >
cuda_find_encoders(
  AVOutputFormat const& output_format,
  AVCodecParameters const& video_params )
{
  std::vector< AVCodec const* > result;
  AVCodec const* codec_ptr = nullptr;
#if LIBAVCODEC_VERSION_MAJOR > 57
  for( void* it = nullptr; ( codec_ptr = av_codec_iterate( &it ) ); )
#else
  while( ( codec_ptr = av_codec_next( codec_ptr ) ) )
#endif
  {
    // Only compatible NVENC encoders
    if( av_codec_is_encoder( codec_ptr ) &&
        is_hardware_codec( codec_ptr ) &&
        !( codec_ptr->capabilities & AV_CODEC_CAP_EXPERIMENTAL ) &&
        format_supports_codec( &output_format, codec_ptr->id ) &&
        ( std::string{ codec_ptr->name }.rfind( "_nvenc" ) !=
          std::string::npos ) )
    {
      result.emplace_back( codec_ptr );
    }
  }
  return result;
}

// ----------------------------------------------------------------------------
hardware_device_context_uptr
cuda_create_context( int device_index )
{
  // Initialize CUDA
  throw_error_code_cuda( cuInit( 0 ), "Could not initialize CUDA" );

  // Create FFmpeg CUDA context
  hardware_device_context_uptr hw_context{
    throw_error_null(
      av_hwdevice_ctx_alloc( AV_HWDEVICE_TYPE_CUDA ),
    "Could not allocate hardware device context" ) };

  auto const cuda_hw_context =
    static_cast< AVCUDADeviceContext* >(
      reinterpret_cast< AVHWDeviceContext* >( hw_context->data )->hwctx );

  // Acquire CUDA device
  CUdevice cu_device;
  throw_error_code_cuda(
    cuDeviceGet( &cu_device, device_index ),
    "Could not acquire CUDA device " + std::to_string( device_index ) );

  constexpr size_t buffer_size = 128;
  char buffer[ buffer_size ] = {};
  cuDeviceGetName( buffer, buffer_size - 1, cu_device );
  LOG_INFO(
    kv::get_logger( "ffmpeg" ),
    "Using CUDA device " << device_index << ": `" << buffer << "`" );

  // Initialize FFmpeg CUDA context
  throw_error_code_cuda(
    cuCtxCreate( &cuda_hw_context->cuda_ctx, 0, cu_device ),
    "Could not create CUDA context" );

#if LIBAVCODEC_VERSION_MAJOR > 57
  throw_error_code_cuda(
    cuStreamCreate( &cuda_hw_context->stream, CU_STREAM_DEFAULT ),
    "Could not create CUDA stream" );
#endif

  throw_error_code(
    av_hwdevice_ctx_init( hw_context.get() ),
    "Could not initialize hardware device context" );

  // Only keep this hardware context if setup worked
  return hw_context;
}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
