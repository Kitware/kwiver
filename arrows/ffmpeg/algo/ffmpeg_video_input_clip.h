// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg video clipping utility.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_INPUT_CLIP_H
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_INPUT_CLIP_H

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/video_input.h>

#include <memory>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
/// Video input which temporally clips an FFmpeg-sourced video.
///
/// This implementation must have access to FFmpeg-level detailed information in
/// order to properly clip raw streams.
class KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_input_clip
  : public vital::algo::video_input
{
public:
  PLUGGABLE_IMPL(
    ffmpeg_video_input_clip,
    "Clip an FFmpeg-sourced video.",

    PARAM_DEFAULT(
      frame_begin, vital::frame_id_t,
      "First frame to include in the clip. Indexed from 1.",
      0 ),

    PARAM_DEFAULT(
      frame_end, vital::frame_id_t,
      "First frame not to include in the clip, i.e. one past the final frame in "
      "the clip. Indexed from 1.",
      0 ),

    PARAM_DEFAULT(
      start_at_keyframe, bool,
      "Start at the first keyframe before frame_begin, if frame_begin is not a "
      "keyframe.",
      false ),

    PARAM(
      video_input, vital::algo::video_input_sptr,
      "Video input reader" )
  )

  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string video_name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;

  size_t num_frames() const override;

  bool next_frame( vital::time_usec_t timeout = 0 ) override;
  bool seek_frame(
    vital::timestamp::frame_t frame_number,
    vital::time_usec_t timeout = 0 ) override;
  bool seek_time(
    vital::timestamp::time_t time_usec,
    vital::time_usec_t timeout = 0 ) override;

  vital::timestamp frame_timestamp() const override;
  vital::image_container_sptr frame_image() override;
  vital::video_raw_image_sptr raw_frame_image() override;
  vital::metadata_vector frame_metadata() override;
  vital::video_raw_metadata_sptr raw_frame_metadata() override;
  vital::video_uninterpreted_data_sptr uninterpreted_frame_data() override;

  vital::video_settings_sptr implementation_settings() const override;
  ~ffmpeg_video_input_clip() override;

protected:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;

private:
  class impl;
  KWIVER_UNIQUE_PTR( impl, d );
};

} // namespace ffmpeg

} // namespace arrows

} // end namespace

#endif
