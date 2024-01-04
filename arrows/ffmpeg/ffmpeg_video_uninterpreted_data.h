// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg video uninterpreted data.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_UNINTERPRETED_DATA_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_UNINTERPRETED_DATA_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>
#include <arrows/ffmpeg/ffmpeg_util.h>

#include <vital/types/video_uninterpreted_data.h>

#include <list>
#include <memory>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_uninterpreted_data
  : public vital::video_uninterpreted_data
{
  ffmpeg_video_uninterpreted_data();

  ffmpeg_video_uninterpreted_data(
    ffmpeg_video_uninterpreted_data const& ) = delete;
  ffmpeg_video_uninterpreted_data&
  operator=( ffmpeg_video_uninterpreted_data const& ) = delete;

  std::list< packet_uptr > audio_packets;
};
using ffmpeg_video_uninterpreted_data_sptr =
  std::shared_ptr< ffmpeg_video_uninterpreted_data >;

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
