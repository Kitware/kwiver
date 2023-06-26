// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg video raw image.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_RAW_IMAGE_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_RAW_IMAGE_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>
#include <arrows/ffmpeg/ffmpeg_util.h>

#include <vital/types/video_raw_image.h>

#include <list>
#include <memory>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_raw_image
  : public vital::video_raw_image
{
  ffmpeg_video_raw_image();

  ffmpeg_video_raw_image( ffmpeg_video_raw_image const& ) = delete;
  ffmpeg_video_raw_image&
  operator=( ffmpeg_video_raw_image const& ) = delete;

  std::list< packet_uptr > packets;
  int64_t frame_dts;
  int64_t frame_pts;
  bool is_keyframe;
};
using ffmpeg_video_raw_image_sptr = std::shared_ptr< ffmpeg_video_raw_image >;

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
