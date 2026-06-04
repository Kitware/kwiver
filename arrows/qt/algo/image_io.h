// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Qt image_io interface

#ifndef KWIVER_ARROWS_QT_IMAGE_IO_H_
#define KWIVER_ARROWS_QT_IMAGE_IO_H_

#include <arrows/qt/kwiver_algo_qt_export.h>

#include <vital/algo/image_io.h>

namespace kwiver {

namespace arrows {

namespace qt {

/// A class for using Qt to read and write images.
///
/// This class provides an algorithm which can be used to read and write image
/// files using Qt. This algorithm is quite limited in terms of what formats
/// are supported, and offers no configuration. It is intended more as a proof
/// of concept.
class KWIVER_ALGO_QT_EXPORT image_io
  : public vital::algo::image_io
{
public:
  PLUGGABLE_IMPL( image_io, "Use Qt to load and save image files." )

  /// Destructor
  virtual ~image_io();

  /// \copydoc vital::algo::image_io::check_configuration
  virtual bool check_configuration(
    vital::config_block_sptr config ) const override;

private:
  /// \copydoc vital::algo::image_io::load_
  virtual vital::image_container_sptr load_(
    std::string const& filename ) const override;

  /// \copydoc vital::algo::image_io::save_
  virtual void save_(
    std::string const& filename,
    vital::image_container_sptr data ) const override;

  void initialize() override;
};

} // end namespace qt

} // end namespace arrows

} // end namespace kwiver

#endif
