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

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
