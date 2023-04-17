// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Video-based implementations of the metadata_stream interfaces.

#include <arrows/core/metadata_stream_from_video.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
metadata_istream_from_video
::metadata_istream_from_video( vital::algo::video_input& video )
  : m_video{ &video }
{
  // If the video input has not been stepped yet, we should move it to the
  // first frame
  if( !m_video->good() && !m_video->end_of_video() )
  {
    vital::timestamp ts;
    m_video->next_frame( ts );
  }
}

// ----------------------------------------------------------------------------
metadata_istream_from_video
::~metadata_istream_from_video()
{}

// ----------------------------------------------------------------------------
vital::algo::video_input&
metadata_istream_from_video
::video() const
{
  return *m_video;
}

// ----------------------------------------------------------------------------
vital::frame_id_t
metadata_istream_from_video
::frame_number() const
{
  if( at_end() )
  {
    throw std::invalid_argument(
            "metadata_istream_from_video::frame_number() "
            "called at end of stream" );
  }

  auto const timestamp = m_video->frame_timestamp();
  return timestamp.has_valid_frame() ? timestamp.get_frame() : 0;
}

// ----------------------------------------------------------------------------
vital::metadata_vector
metadata_istream_from_video
::metadata()
{
  if( at_end() )
  {
    throw std::invalid_argument(
            "metadata_istream_from_video::metadata() "
            "called at end of stream" );
  }
  return m_video->frame_metadata();
}

// ----------------------------------------------------------------------------
bool
metadata_istream_from_video
::next_frame()
{
  vital::timestamp ts;
  return m_video->next_frame( ts );
}

// ----------------------------------------------------------------------------
bool
metadata_istream_from_video
::at_end() const
{
  return !m_video->good();
}

} // namespace core

} // namespace arrows

} // namespace kwiver
