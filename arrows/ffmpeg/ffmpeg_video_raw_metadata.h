// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg video raw metadata.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_RAW_METADATA_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_RAW_METADATA_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>
#include <arrows/ffmpeg/ffmpeg_util.h>

#include <vital/types/video_raw_metadata.h>

#include <list>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_raw_metadata
  : public vital::video_raw_metadata
{
  ffmpeg_video_raw_metadata();

  ffmpeg_video_raw_metadata( ffmpeg_video_raw_metadata const& ) = delete;
  ffmpeg_video_raw_metadata&
  operator=( ffmpeg_video_raw_metadata const& ) = delete;

  std::list< ffmpeg_detail::av_packet_uptr > packets;
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
