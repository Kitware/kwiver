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
  PLUGIN_INFO( "ffmpeg",
               "Use FFMPEG to write video files from a sequence of images." );

  ffmpeg_video_output();

  virtual ~ffmpeg_video_output();

  vital::config_block_sptr get_configuration() const override;

  void set_configuration( vital::config_block_sptr config ) override;

  bool check_configuration( vital::config_block_sptr config ) const override;

  void open( std::string video_name,
             vital::video_settings const* settings ) override;

  void close() override;

  bool good() const override;

  void add_image(
    vital::image_container_sptr const& image,
    vital::timestamp const& ts ) override;
  void add_image( vital::video_raw_image const& image );

  void add_metadata( vital::metadata const& md ) override;

  void add_uninterpreted_data(
    vital::video_uninterpreted_data const& misc_data ) override;

  vital::video_settings_uptr implementation_settings() const override;

private:
  class impl;

  std::unique_ptr< impl > const d;
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
