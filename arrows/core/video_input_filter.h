// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef ARROWS_CORE_VIDEO_INPUT_FILTER_H
#define ARROWS_CORE_VIDEO_INPUT_FILTER_H

#include <vital/algo/video_input.h>

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.txx>

#include "vital/plugin_management/pluggable_macro_magic.h"
namespace kwiver {
namespace arrows {
namespace core {

/// A video reader that filters the frames and metadata
// ----------------------------------------------------------------------------
/// This class implements a video input that down selects frames
/// ready by another video reader.  It may down sample the framerate,
/// remove frames before or after indicated frames, etc.
class KWIVER_ALGO_CORE_EXPORT video_input_filter
  : public  vital::algo::video_input
{
public:
   

  PLUGGABLE_IMPL(
    video_input_filter,
     "A video input that calls another video input"
     " and filters the output on frame range and other parameters.",
    PARAM_DEFAULT(start_at_frame, vital::frame_id_t, 
          "Frame number (from 1) to start processing video input. " 
          "If set to zero, start at the beginning of the video.", 
          1),
    PARAM_DEFAULT(stop_after_frame,vital::frame_id_t, 
          "End the video after passing this frame number. "
          "Set this value to 0 to disable filter.",
          0),
    PARAM_DEFAULT(output_nth_frame,vital::frame_id_t, 
          "Only outputs every nth frame of the video starting at the " 
          "first frame. The output of num_frames still reports the total " 
          "frames in the video but skip_frame is valid every nth frame " 
          "only and there are metadata_map entries for only every nth " 
          "frame.",
          1),
      PARAM_DEFAULT(frame_rate,double, 
          "Number of frames per second. "
          "If the video does not provide a valid time, use this rate "
          "to compute frame time.  Set 0 to disable.",
          30.0),
      PARAM(video_input,vital::algo::video_input_sptr, 
          "pointer to the nested algorithm")
          )
  
  virtual ~video_input_filter();

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration(vital::config_block_sptr config) const override;

  virtual void open( std::string name );
  virtual void close();

  virtual bool end_of_video() const;
  virtual bool good() const;
  virtual bool seekable() const;
  virtual size_t num_frames() const;

  virtual bool next_frame( kwiver::vital::timestamp& ts,
                           uint32_t timeout = 0 );

  virtual bool seek_frame( kwiver::vital::timestamp& ts,
                           kwiver::vital::timestamp::frame_t frame_number,
                           uint32_t timeout = 0 );

  virtual kwiver::vital::timestamp frame_timestamp() const;
  virtual kwiver::vital::image_container_sptr frame_image();
  virtual kwiver::vital::metadata_vector frame_metadata();
  kwiver::vital::video_raw_metadata_sptr raw_frame_metadata() override;
  virtual kwiver::vital::metadata_map_sptr metadata_map();

  kwiver::vital::video_settings_uptr implementation_settings() const override;

private:
  void initialize() override;
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR(priv,d);
};

} } } // end namespace

#endif // ARROWS_CORE_VIDEO_INPUT_FILTER_H
