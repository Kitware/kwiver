// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief GDAL image_io interface

#ifndef KWIVER_ARROWS_GDAL_IMAGE_IO_H_
#define KWIVER_ARROWS_GDAL_IMAGE_IO_H_

#include <arrows/gdal/kwiver_algo_gdal_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/image_io.h>

namespace kwiver {

namespace arrows {

namespace gdal {

/// A class for using GDAL to read and write images
class KWIVER_ALGO_GDAL_EXPORT image_io
  : public vital::algo::image_io
{
public:
  PLUGGABLE_IMPL(
    image_io,
    " A class for using GDAL to read and write images. ",
    PARAM_DEFAULT(
      nodata_enabled, bool,
      "When set to true, GDAL will attempt to mark any pixels that have a "
      "value of nodata_value as transparent when writing an image.",
      false ),
    PARAM_DEFAULT(
      nodata_value, double,
      "Special value that marks pixels as having no data, causing them to be "
      "displayed as transparent. This is only supported when writing certain "
      "formats, and only effective when nodata_enabled is set to true.",
      0.0 ),
  )

// No configuration for this class yet
  /// \cond DoxygenSuppress
  virtual bool
  check_configuration( vital::config_block_sptr /*config*/ ) const
  {
    return true;
  }

  /// \endcond

private:
  /// Implementation specific load functionality.
  ///
  /// \param filename the path to the file to load
  /// \returns an image container refering to the loaded image
  virtual vital::image_container_sptr load_(
    const std::string& filename ) const;

  /// Implementation specific save functionality.
  ///
  /// \param filename the path to the file to save
  /// \param data the image container refering to the image to write
  virtual void save_(
    const std::string& filename,
    vital::image_container_sptr data ) const;
};

} // end namespace gdal

} // end namespace arrows

} // end namespace kwiver

#endif
