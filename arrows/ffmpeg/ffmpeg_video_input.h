// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg-based video input.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_INPUT_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_INPUT_H_

#include <arrows/ffmpeg/ffmpeg_video_settings.h>
#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <vital/algo/video_input.h>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
/// Video input using FFmpeg (libav).
class KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_input
  : public vital::algo::video_input
{
public:
  enum seek_mode {
    SEEK_MODE_EXACT,
    SEEK_MODE_KEYFRAME_BEFORE,
  };

  ffmpeg_video_input();
  virtual ~ffmpeg_video_input();

  PLUGIN_INFO(
    "ffmpeg", "Use FFmpeg to read video files as a sequence of images." )

  vital::config_block_sptr get_configuration() const override;
  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string video_name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;

  bool seekable() const override;
  size_t num_frames() const override;
  double frame_rate() override;

  bool next_frame( vital::timestamp& ts, uint32_t timeout = 0 ) override;
  bool seek_frame(
    vital::timestamp& ts, vital::timestamp::frame_t frame_number,
    uint32_t timeout = 0 ) override;

  bool seek_frame_(
    vital::timestamp& ts, vital::timestamp::frame_t frame_number,
    seek_mode mode, uint32_t timeout = 0 );

  vital::timestamp frame_timestamp() const override;
  vital::image_container_sptr frame_image() override;
  vital::video_raw_image_sptr raw_frame_image() override;
  vital::metadata_vector frame_metadata() override;
  vital::video_raw_metadata_sptr raw_frame_metadata() override;
  vital::video_uninterpreted_data_sptr uninterpreted_frame_data() override;
  vital::metadata_map_sptr metadata_map() override;

  vital::video_settings_uptr implementation_settings() const override;

private:
  /// Private implementation class
  class priv;

  const std::unique_ptr< priv > d;
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
