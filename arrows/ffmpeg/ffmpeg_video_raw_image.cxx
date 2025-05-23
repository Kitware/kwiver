// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of FFmpeg video raw image.

#include "ffmpeg_video_raw_image.h"

namespace kwiver {

namespace arrows {

namespace ffmpeg {

ffmpeg_video_raw_image
::ffmpeg_video_raw_image()
  : packets{},
    frame_dts{ AV_NOPTS_VALUE },
    frame_pts{ AV_NOPTS_VALUE },
    is_keyframe{ true }
{}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
