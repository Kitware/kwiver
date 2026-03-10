// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef ARROWS_CORE_VIDEO_INPUT_SPLICE_H
#define ARROWS_CORE_VIDEO_INPUT_SPLICE_H

#include <vital/algo/video_input.h>

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.txx>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
/// Video input that splices frames together from multiple video input sources.
///
/// This class implements a video input algorithm that splices multiple video
/// input sources together into a single source.
class KWIVER_ALGO_CORE_EXPORT video_input_splice
  : public vital::algo::video_input
{
public:

public:
  PLUGGABLE_IMPL(
    video_input_splice,
    "Splices multiple video sources together.",
    PARAM_DEFAULT(
      output_nth_frame, size_t,
      "Only outputs every nth frame of the video starting at the first frame. "
      "The output of num_frames still reports the total frames in the video "
      "but skip_frame is valid every nth frame only and there are metadata_map "
      "entries for only every nth frame.",
      1 ),
    PARAM(
      video_source, std::vector< vital::algo::video_input_sptr >,
      "Video sources" )
  )

  virtual ~video_input_splice();

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  virtual void open( std::string name );
  virtual void close();

  virtual bool end_of_video() const;
  virtual bool good() const;
  virtual size_t num_frames() const;

  virtual bool next_frame( vital::time_usec_t timeout = 0 );

  virtual bool seek_frame(
    vital::timestamp::frame_t frame_number,
    vital::time_usec_t timeout = 0 );
  bool seek_time(
    vital::timestamp::time_t time_usec,
    vital::time_usec_t timeout = 0 ) override;

  virtual kwiver::vital::timestamp frame_timestamp() const;
  virtual kwiver::vital::image_container_sptr frame_image();
  virtual kwiver::vital::metadata_vector frame_metadata();

  kwiver::vital::video_settings_uptr implementation_settings() const override;

protected:
  void initialize() override;
  void set_configuration_internal(
    vital::config_block_sptr in_config ) override;

private:
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace core

} // namespace arrows

}     // end namespace

#endif // ARROWS_CORE_VIDEO_INPUT_SPLICE_H
