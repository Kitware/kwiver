// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of FFmpeg video raw metadata.

#include "ffmpeg_video_raw_metadata.h"

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
ffmpeg_video_raw_metadata
::ffmpeg_video_raw_metadata() : packets{}
{}

// ----------------------------------------------------------------------------
ffmpeg_video_raw_metadata::packet_info
::packet_info() : packet{}, stream_settings{}
{}

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver
