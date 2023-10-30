// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef ARROWS_CORE_VIDEO_INPUT_METADATA_FILTER_H
#define ARROWS_CORE_VIDEO_INPUT_METADATA_FILTER_H

#include <vital/algo/video_input.h>

#include <arrows/core/kwiver_algo_core_export.h>

namespace kwiver {

namespace arrows {

namespace core {

/// A video reader that filters the metadata.
///
/// This class implements a video input that applies another filter to a video
/// frame's metadata.
class KWIVER_ALGO_CORE_EXPORT video_input_metadata_filter
  : public vital::algo::video_input
{
public:
  PLUGIN_INFO( "metadata_filter",
               "A video input that calls another video input"
               " and applies a filter to the output metadata." )

  video_input_metadata_filter();
  virtual ~video_input_metadata_filter();

  vital::config_block_sptr get_configuration() const override;
  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;
  bool seekable() const override;
  size_t num_frames() const override;

  bool next_frame( kwiver::vital::timestamp& ts,
                   uint32_t timeout = 0 ) override;

  bool seek_frame( kwiver::vital::timestamp& ts,
                   kwiver::vital::timestamp::frame_t frame_number,
                   uint32_t timeout = 0 ) override;

  kwiver::vital::timestamp frame_timestamp() const override;
  kwiver::vital::image_container_sptr frame_image() override;
  kwiver::vital::video_raw_image_sptr raw_frame_image() override;
  kwiver::vital::metadata_vector frame_metadata() override;
  kwiver::vital::video_uninterpreted_data_sptr uninterpreted_frame_data() override;
  kwiver::vital::metadata_map_sptr metadata_map() override;

  kwiver::vital::video_settings_uptr implementation_settings() const override;

private:
  class priv;

  std::unique_ptr< priv > const m_d;
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
