// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of a video input which applies a buffered metadata filter.

#ifndef ARROWS_CORE_VIDEO_INPUT_BUFFERED_METADATA_FILTER_H_
#define ARROWS_CORE_VIDEO_INPUT_BUFFERED_METADATA_FILTER_H_

#include <vital/algo/video_input.h>

#include <arrows/core/kwiver_algo_core_export.h>

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
  PLUGIN_INFO( "buffered_metadata_filter",
               "A video input that calls another video input and applies a "
               "buffered filter to the output metadata." )

  video_input_buffered_metadata_filter();
  virtual ~video_input_buffered_metadata_filter();

  vital::config_block_sptr get_configuration() const override;
  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;
  bool seekable() const override;
  size_t num_frames() const override;

  bool next_frame( vital::timestamp& ts,
                   uint32_t timeout = 0 ) override;

  bool seek_frame( vital::timestamp& ts,
                   vital::timestamp::frame_t frame_number,
                   uint32_t timeout = 0 ) override;

  vital::timestamp frame_timestamp() const override;
  vital::image_container_sptr frame_image() override;
  vital::video_raw_image_sptr raw_frame_image() override;
  vital::metadata_vector frame_metadata() override;
  vital::video_uninterpreted_data_sptr uninterpreted_frame_data() override;
  vital::metadata_map_sptr metadata_map() override;

  vital::video_settings_uptr implementation_settings() const override;

private:
  class impl;

  std::unique_ptr< impl > const d;
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
