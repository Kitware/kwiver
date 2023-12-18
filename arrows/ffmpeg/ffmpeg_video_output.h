// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of FFmpeg video writer.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_OUTPUT_H
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_OUTPUT_H

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <vital/algo/video_output.h>

#include <vital/types/image_container.h>
#include <vital/types/timestamp.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// --------------------------------------------------------------------------
class KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_video_output
  : public kwiver::vital::algo::video_output
{
public:
   PLUGGABLE_IMPL(
   ffmpeg_video_output,
      "Use FFMPEG to write video files from a sequence of images.",
   PARAM_DEFAULT(width,size_t, 
      "Output width in pixels.",
      0),
   PARAM_DEFAULT(height,size_t, 
      "Output heigth pixels.",
      0),
   PARAM_DEFAULT(frame_rate_num,int, 
      "Integral numerator of the output frame rate.",
      0),
   PARAM_DEFAULT(frame_rate_den,int,
    "Integral denominator of the output frame rate. Defaults to 1.",
    1),
    PARAM_DEFAULT(bitrate,size_t,
    "Desired bitrate in bits per second.",
    0),
    PARAM_DEFAULT(codec_name,std::string,
     "String identifying the codec to use.",
     ""),
    PARAM_DEFAULT(cuda_enabled,bool,
    "When set to true, uses CUDA/NVENC to accelerate video encoding.",
    m_cuda_enabled_default),
    PARAM_DEFAULT(cuda_device_index,int,
    "Integer index of the CUDA-enabled device to use for encoding. "
    "Defaults to 0.",
     0)
  );

  virtual ~ffmpeg_video_output();


  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string video_name,
             vital::video_settings const* settings ) override;

  void close() override;

  bool good() const override;

  void add_image(
    vital::image_container_sptr const& image,
    vital::timestamp const& ts ) override;
  void add_image( vital::video_raw_image const& image ) override;

  void add_metadata( vital::metadata const& md ) override;

  vital::video_settings_uptr implementation_settings() const override;

protected:
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void initialize() override;

private:
#ifdef KWIVER_ENABLE_FFMPEG_CUDA
    constexpr static bool m_cuda_enabled_default { true };
#else
    constexpr static bool m_cuda_enabled_default { false };
#endif

  class impl;
  KWIVER_UNIQUE_PTR(impl,d);
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
