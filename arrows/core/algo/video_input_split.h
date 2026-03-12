// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef ARROWS_CORE_VIDEO_INPUT_SPLIT_H
#define ARROWS_CORE_VIDEO_INPUT_SPLIT_H

#include <vital/algo/video_input.h>

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.txx>

namespace kwiver {

namespace arrows {

namespace core {

/// Video input that pulls image and metadata inputs from different sources.
// ----------------------------------------------------------------------------
/// This class implements a video input algorithm that holds two other video
/// input algorithms and pulls imagery from one and metadata from the other.
class KWIVER_ALGO_CORE_EXPORT video_input_split
  : public vital::algo::video_input
{
public:
  PLUGGABLE_IMPL(
    video_input_split,
    "Coordinate two video readers."
    " One reader supplies the image/data stream."
    " The other reader supplies the metadata stream.",
    PARAM(
      image_source,
      vital::algo::video_input_sptr,
      "Algorithm pointer to reader" ),
    PARAM(
      metadata_source,
      vital::algo::video_input_sptr,
      "Algorithm pointer to metadata stream" )
  );

  virtual ~video_input_split();

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;
  bool seekable() const override;
  size_t num_frames() const override;

  bool next_frame( vital::time_usec_t timeout = 0 ) override;

  bool seek_frame(
    vital::timestamp::frame_t frame_number,
    vital::time_usec_t timeout = 0 ) override;

  kwiver::vital::timestamp frame_timestamp() const override;
  kwiver::vital::image_container_sptr frame_image() override;
  kwiver::vital::metadata_vector frame_metadata() override;

  kwiver::vital::video_settings_uptr implementation_settings() const override;

protected:
  void initialize() override;

private:
  kwiver::vital::timestamp merge_timestamps(
    kwiver::vital::timestamp const& image_ts,
    kwiver::vital::timestamp const& metadata_ts ) const;

  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace core

} // namespace arrows

}     // end namespace

#endif // ARROWS_CORE_VIDEO_INPUT_SPLIT_H
