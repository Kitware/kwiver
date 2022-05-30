// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of FFmpeg internal utility classes and functions.

#include "ffmpeg_util.h"

#include <array>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
std::string
error_string( int error_code )
{
  std::array< char, AV_ERROR_MAX_STRING_SIZE > buffer = { 0 };
  av_strerror( error_code, buffer.data(), buffer.size() );
  return buffer.data();
}

} // namespace ffmpeg

} // namespace vital

} // namespace kwiver
