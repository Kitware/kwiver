// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header file for video input using VXL methods.

#ifndef KWIVER_ARROWS_VXL_VIDL_FFMPEG_VIDEO_INPUT_H
#define KWIVER_ARROWS_VXL_VIDL_FFMPEG_VIDEO_INPUT_H

#include <vital/algo/algorithm.txx>
#include <vital/algo/video_input.h>

#include <arrows/vxl/kwiver_algo_vxl_export.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// Video input using VXL vidl ffmpeg services.
// ----------------------------------------------------------------------------
/// This class implements a video input algorithm using the VXL vidl
/// ffmpeg video services.
///
class KWIVER_ALGO_VXL_EXPORT vidl_ffmpeg_video_input
  : public vital::algo::video_input
{
public:
  PLUGGABLE_IMPL(
    vidl_ffmpeg_video_input,
    "Use VXL (vidl with FFMPEG) to read video files as a sequence of images.",
    PARAM_DEFAULT(
      start_at_frame, vital::frame_id_t,
      "Frame number (from 1) to start processing video input. If set to zero, "
      "start at the beginning of the video.",
      0 ),
    PARAM_DEFAULT(
      stop_after_frame, vital::frame_id_t,
      "Number of frames to supply. If set to zero then supply all frames after "
      "start frame.",
      0 ),
    PARAM_DEFAULT(
      output_nth_frame, vital::timestamp::frame_t,
      "Only outputs every nth frame of the video starting at the first frame. "
      "The output of num_frames still reports the total frames in the video "
      "but skip_frame is valid every nth frame only and there are metadata "
      "frames for the skipped frames.",
      1 ),
    PARAM_DEFAULT(
      time_scan_frame_limit, int,
      "Number of frames to be scanned searching input video for embedded time "
      "codes. If the value is zero, the whole video is scanned.",
      100 ),
    PARAM_DEFAULT(
      use_metadata, bool,
      "Whether to use metadata from the video stream. If set to false, any "
      "metadata included in the video stream is ignored.",
      true ),
    PARAM_DEFAULT(
      time_source, std::string,
      "List of sources for absolute frame time information. This entry "
      "specifies a comma or space separated list of sources that are tried in "
      "order until a valid time source is found. If an entry is not valid, the "
      "next source is tried. Valid sources are:\n"
      "  - none - do not supply absolute time\n"
      "  - misp - use frame embedded time stamps.\n"
      "  - klv0601 - use klv 0601 format metadata for frame time\n"
      "  - klv0104 - use klv 0104 format metadata for frame time\n"
      "  - current - use current time as the frame time\n"
      "  - start_at_0 - start metadata time at 0",
      "start_at_0" ) )

  virtual ~vidl_ffmpeg_video_input();

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string video_name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;
  bool seekable() const override;
  size_t num_frames() const override;

  bool next_frame( kwiver::vital::time_usec_t timeout = 0 ) override;

  bool seek_frame(
    kwiver::vital::timestamp::frame_t frame_number,
    kwiver::vital::time_usec_t timeout = 0 ) override;

  bool seek_time(
    kwiver::vital::timestamp::time_t time_usec,
    kwiver::vital::time_usec_t timeout = 0 ) override;

  kwiver::vital::timestamp frame_timestamp() const override;

  double frame_rate() override;

  kwiver::vital::image_container_sptr frame_image() override;
  kwiver::vital::metadata_vector frame_metadata() override;
  kwiver::vital::metadata_map_sptr metadata_map() override;

protected:
  void initialize() override;
  void set_configuration_internal(
    vital::config_block_sptr config ) override;

private:
  /// Push the declared parameters into the private implementation
  void apply_config_to_priv();

  /// private implementation class
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace vxl

} // namespace arrows

}     // end namespace

#endif // KWIVER_ARROWS_VXL_VIDL_FFMPEG_VIDEO_INPUT_H
