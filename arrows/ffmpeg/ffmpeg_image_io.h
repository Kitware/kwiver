// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of the FFmpeg-based image_io algorithm.

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_IMAGE_IO_H_
#define KWIVER_ARROWS_FFMPEG_FFMPEG_IMAGE_IO_H_

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_export.h>

#include <vital/algo/image_io.h>

#include <memory>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

// ----------------------------------------------------------------------------
/// Image reader / writer using FFmpeg (libav).
class KWIVER_ALGO_FFMPEG_EXPORT ffmpeg_image_io
  : public vital::algo::image_io
{
public:
  ffmpeg_image_io();
  virtual ~ffmpeg_image_io();

  PLUGIN_INFO( "ffmpeg", "Use FFmpeg to read and write image files." )

  vital::config_block_sptr get_configuration() const override;
  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  vital::image_container_sptr load_(
    std::string const& filename ) const override;

  void save_(
    std::string const& filename,
    kwiver::vital::image_container_sptr data ) const override;

  class impl;

  std::shared_ptr< impl > d;
};

} // namespace ffmpeg

} // namespace arrows

} // namespace kwiver

#endif
