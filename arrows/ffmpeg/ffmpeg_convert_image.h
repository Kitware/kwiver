// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg image conversion utilities.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_CONVERT_IMAGE_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_CONVERT_IMAGE_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <arrows/ffmpeg/ffmpeg_util.h>

#include <vital/types/image_container.h>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
vital::image_container_sptr frame_to_vital_image(
  AVFrame* frame, sws_context_uptr* cached_sws = nullptr );

// ----------------------------------------------------------------------------
frame_uptr vital_image_to_frame(
  vital::image_container_scptr const& image,
  AVCodecContext const* codec_context = nullptr,
  sws_context_uptr* cached_sws = nullptr );

// ----------------------------------------------------------------------------
AVPixelFormat pix_fmt_from_depth( size_t depth );

// ----------------------------------------------------------------------------
size_t depth_from_pix_fmt( AVPixelFormat pix_fmt );

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
