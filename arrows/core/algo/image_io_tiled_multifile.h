// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of the image_io_tiled_multifile algorithm.

#ifndef KWIVER_ARROWS_CORE_IMAGE_IO_TILED_MULTIFILE_H_
#define KWIVER_ARROWS_CORE_IMAGE_IO_TILED_MULTIFILE_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/image_io.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class KWIVER_ALGO_CORE_EXPORT image_io_tiled_multifile
  : public vital::algo::image_io
{
  PLUGGABLE_IMPL(
    image_io_tiled_multifile,
    "Read and write tiled images as a directory of separate files.",
    PARAM_DEFAULT(
      omit_single_file_suffix, bool,
      "If true, saving an image with only a single tile will omit the "
      "location suffix, i.e. file.png instead of file.0000.0000.png",
      false
    ),
    PARAM(
      image_io, vital::algo::image_io_sptr,
      "Image I/O algorithm used on individual files." )
  )

  ~image_io_tiled_multifile();

  bool check_configuration( vital::config_block_sptr config ) const override;

  vital::image_container_sptr load_(
    std::string const& filename ) const override;

  void save_(
    std::string const& filename,
    vital::image_container_sptr data ) const override;

  bool skip_path_validation_() const override;

private:
  void initialize() override;
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
