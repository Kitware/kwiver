// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of base video settings type.

#ifndef VITAL_VIDEO_SETTINGS_H_
#define VITAL_VIDEO_SETTINGS_H_

#include <vital/vital_export.h>

#include <memory>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Base class for holding information about how to encode a video.
struct VITAL_EXPORT video_settings
{
  virtual ~video_settings();
};

using video_settings_sptr = std::shared_ptr< video_settings >;
using video_settings_uptr = std::unique_ptr< video_settings >;

} // namespace vital

} // namespace kwiver

#endif
