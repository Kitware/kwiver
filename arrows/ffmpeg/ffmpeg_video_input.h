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

  virtual ~ffmpeg_video_input();
  

public:
   PLUGGABLE_IMPL(
   ffmpeg_video_input,
      "Use FFMPEG to read video files as a sequence of images.",
   PARAM_DEFAULT(filter_desc,std::string, 
      "A string describing the libavfilter pipeline to apply when reading " 
      "the video.  Only filters that operate on each frame independently " 
      "will currently work.  The default \"yadif=deint=1\" filter applies " 
      "deinterlacing only to frames which are interlaced.  " 
      "See details at https://ffmpeg.org/ffmpeg-filters.html", 
      "yadif=deint=1"), 
  PARAM_DEFAULT(klv_enabled,bool, 
      "When set to false, will not attempt to process any KLV metadata found in " 
    "the video file. This may be useful if only processing imagery.", 
    true), 
  PARAM_DEFAULT(audio_enabled,bool, 
      "When set to false, will not attempt to pass along any audio streams found "
      "in the video file. This may be useful if no transcoding is to be done, or "
      "if audio is to be dropped.",
    true), 
  PARAM_DEFAULT(use_misp_timestamps,bool, 
    "When set to true, will attempt to use correlate KLV packet data to " 
    "frames using the MISP timestamps embedding in the frame packets. This is " 
    "technically the correct way to decode KLV, but the frame timestamps are " 
    "wrongly encoded so often in real-world data that it is turned off by " 
    "default. When turned off, the frame timestamps are emulated by looking " 
    "at the KLV packets near each frame.", 
    false), 
  PARAM_DEFAULT( smooth_klv_packets, bool, 
    "When set to true, will output 'smoothed' KLV packets: one packet for each " 
    "standard for each frame with the current value of every existing tag. " 
    "Otherwise, will report packets as they appear in the source video.", 
    false), 
  PARAM_DEFAULT(unknown_stream_behavior, std::string, 
    "Set to 'klv' to treat unknown streams as KLV. " 
    "Set to 'ignore' to ignore unknown streams (default).", 
    "ignore"), 
  PARAM_DEFAULT(retain_klv_duration,uint64_t, 
    "Number of microseconds of past KLV to retain in case of backwards " 
    "timestamp jumps. Defaults to 5000000.", 
    5000000), 
  PARAM_DEFAULT(cuda_enabled, bool, 
    "When set to true, uses CUDA/CUVID to accelerate video decoding.", 
    m_cuda_available
    ),
  PARAM_DEFAULT(cuda_device_index,int, 
   "Integer index of the CUDA-enabled device to use for decoding. " 
   "Defaults to 0.", 
   0))

  /// Check that the algorithm's currently configuration is valid
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

protected:
  void initialize() override;
  void set_configuration_internal(vital::config_block_sptr cb) override;

private:
 #ifdef KWIVER_ENABLE_FFMPEG_CUDA
 static constexpr bool m_cuda_available = true;
 #else
 static constexpr bool m_cuda_available = false;
 #endif
  /// Private implementation class
  class priv;
  
  KWIVER_UNIQUE_PTR(priv,d);
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
