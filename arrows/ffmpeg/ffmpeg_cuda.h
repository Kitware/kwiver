// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declarations of CUDA/CUVID utilities for use in FFmpeg.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_CUDA_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_CUDA_H_

#ifdef KWIVER_ENABLE_FFMPEG_CUDA

#include <arrows/ffmpeg/ffmpeg_util.h>

#include <cuda.h>
#include <ffnvcodec/dynlink_cuda.h>
#include <ffnvcodec/dynlink_cuviddec.h>
#include <ffnvcodec/dynlink_loader.h>
#include <ffnvcodec/dynlink_nvcuvid.h>
#include <ffnvcodec/nvEncodeAPI.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext_cuda.h>
}

#include <string>
#include <vector>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
CuvidFunctions* cuvid_fn();

// ----------------------------------------------------------------------------
NV_ENCODE_API_FUNCTION_LIST* nvenc_fn();

// ----------------------------------------------------------------------------
std::string cuda_error_string( CUresult err );

// ----------------------------------------------------------------------------
template< class... Args >
inline int throw_error_code_cuda( CUresult error_code, Args... args )
{
  if( error_code != CUDA_SUCCESS )
  {
    throw_error( args..., ": ", cuda_error_string( error_code ) );
  }
  return error_code;
}

// ----------------------------------------------------------------------------
std::vector< AVCodec const* >
cuda_find_decoders( AVCodecParameters const& video_params );

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
#endif
