// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of a video input which applies a buffered metadata filter.

#ifndef ARROWS_CORE_VIDEO_INPUT_BUFFERED_METADATA_FILTER_H_
#define ARROWS_CORE_VIDEO_INPUT_BUFFERED_METADATA_FILTER_H_

#include <vital/algo/video_input.h>

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.txx>

#include <vital/algo/buffered_metadata_filter.h>
#include <vital/exceptions.h>

namespace kwiver {

namespace arrows {

namespace core {

/// A video reader that filters the metadata, reading ahead some frames.
///
/// This class implements a video input that applies a buffered filter to a
/// video stream's metadata. The filter must (eventually) produce one frame of
/// output metadata for each frame given to it.
class KWIVER_ALGO_CORE_EXPORT video_input_buffered_metadata_filter
  : public vital::algo::video_input
{
public:
  PLUGGABLE_IMPL(
    video_input_buffered_metadata_filter,
    "A video input that calls another video input and applies a "
    "buffered filter to the output metadata.",
    PARAM_DEFAULT(
      load_image, bool,
      "When set to false, the frame image will not be loaded nor buffered and "
      "frame_image() will return nullptr. This can save significant memory and "
      "compute when the frame data is not needed.",
      true ),
    PARAM(
      video_input, kwiver::vital::algo::video_input_sptr,
      "video_input" ),
    PARAM(
      metadata_filter, kwiver::vital::algo::buffered_metadata_filter_sptr,
      "metadata_filter" )
  )

  virtual ~video_input_buffered_metadata_filter();

  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;
  bool seekable() const override;
  size_t num_frames() const override;

  bool next_frame(
    vital::timestamp& ts,
    vital::time_usec_t timeout = 0 ) override;

  bool seek_frame(
    vital::timestamp& ts,
    vital::timestamp::frame_t frame_number,
    vital::time_usec_t timeout = 0 ) override;

  vital::timestamp frame_timestamp() const override;
  vital::image_container_sptr frame_image() override;
  vital::video_raw_image_sptr raw_frame_image() override;
  vital::metadata_vector frame_metadata() override;
  vital::video_raw_metadata_sptr raw_frame_metadata() override;
  vital::video_uninterpreted_data_sptr uninterpreted_frame_data() override;

  vital::video_settings_uptr implementation_settings() const override;

private:
  void initialize() override;
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
