// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VXL image_io interface

#ifndef KWIVER_ARROWS_VXL_IMAGE_IO_H_
#define KWIVER_ARROWS_VXL_IMAGE_IO_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_io.h>
#include <vital/types/vector.h>

namespace kwiver {

namespace arrows {

namespace vxl {

using array2 = std::array< unsigned, 2 >;

/// A class for using VXL to read and write images
class KWIVER_ALGO_VXL_EXPORT image_io
  : public vital::algo::image_io
{
public:
  PLUGGABLE_IMPL(
    image_io,
    "Use VXL (vil) to load and save image files.",
    PARAM_DEFAULT(
      force_byte, bool,
      "When loading, convert the loaded data into a byte "
      "(unsigned char) image regardless of the source data type. "
      "Stretch the dynamic range according to the stretch options "
      "before converting. When saving, convert to a byte image "
      "before writing out the image",
      false ),
    PARAM_DEFAULT(
      auto_stretch, bool,
      "Dynamically stretch the range of the input data such that "
      "the minimum and maximum pixel values in the data map to "
      "the minimum and maximum support values for that pixel "
      "type, or 0.0 and 1.0 for floating point types.  If using "
      "the force_byte option value map between 0 and 255. "
      "Warning, this can result in brightness and constrast "
      "varying between images.",
      false ),
    PARAM_DEFAULT(
      manual_stretch, bool,
      "Manually stretch the range of the input data by "
      "specifying the minimum and maximum values of the data "
      "to map to the full byte range",
      false ),
    PARAM_DEFAULT(
      intensity_range, array2,
      "The range of intensity values (min, max) to stretch into "
      "the byte range.  This is most useful when e.g. 12-bit "
      "data is encoded in 16-bit pixels. Only used when manual_stretch is "
      "set to true.",
      array2( { 0, 255 } ) ),
    PARAM_DEFAULT(
      split_channels, bool,
      "When writing out images, if it contains more than one image "
      "plane, write each plane out as a seperate image file. Also, "
      "when enabled at read time, support images written out in via "
      "this method.",
      false )
  )

  /// Destructor
  virtual ~image_io();

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;
  /// Get plane filenames for a given written file
  std::vector< std::string > plane_filenames(
    std::string const& filename ) const;

private:
  /// Implementation specific load functionality.
  //  NOTE: When loading boolean images (ppm, pbm, etc.), true-value regions are
  //  represented in the returned image as regions of 1's.
  //
  //  \param filename the path to the file to load
  //  \returns an image container refering to the loaded image
  virtual vital::image_container_sptr load_(
    const std::string& filename ) const;

  /// Implementation specific save functionality.
  ///
  /// \param filename the path to the file to save
  /// \param data the image container refering to the image to write
  virtual void save_(
    const std::string& filename,
    vital::image_container_sptr data ) const;

  /// Implementation specific metadata functionality.
  ///
  /// \param filename the path to the file to read
  /// \returns pointer to the loaded metadata
  virtual kwiver::vital::metadata_sptr load_metadata_(
    std::string const& filename ) const;

  void initialize() override;
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver

#endif
