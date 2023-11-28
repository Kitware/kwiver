// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Video-based implementations of the metadata_stream interfaces.

#ifndef KWIVER_ARROWS_CORE_METADATA_STREAM_FROM_VIDEO_H_
#define KWIVER_ARROWS_CORE_METADATA_STREAM_FROM_VIDEO_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/video_input.h>
#include <vital/types/metadata_stream.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
/// Stream that reads from a \c video_input.
class KWIVER_ALGO_CORE_EXPORT metadata_istream_from_video
  : public vital::metadata_istream
{
public:
  /// \param video
  ///   Video object to read frames of metadata from. Must already be open.
  explicit metadata_istream_from_video( vital::algo::video_input& video );
  virtual ~metadata_istream_from_video();

  vital::algo::video_input& video() const;

  vital::frame_id_t frame_number() const override;
  vital::metadata_vector metadata() override;
  bool next_frame() override;
  bool at_end() const override;

private:
  vital::algo::video_input* m_video;
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
