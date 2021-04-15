// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_OCV_INPAINT_
#define KWIVER_ARROWS_OCV_INPAINT_

#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace ocv {

/// OCV image inpainting process
///
/// Create an inpainted RGB image from an RGBA one. This is used to fix the RGB
/// image which has corrupted content in the regions specified by the non-zero
/// elements of the Alpha channel. The value for the corrupted pixels are
/// estimated from the neighboring pixels.
class KWIVER_ALGO_OCV_EXPORT inpaint
  : public vital::algo::image_filter
{
public:
  PLUGIN_INFO( "ocv_inpainting",
               "Inpaint an RGBA image using non-zero alpha values as a mask." )

  inpaint();
  virtual ~inpaint();

  /// Get this algorithm's \link vital::config_block configuration block
  /// \endlink
  vital::config_block_sptr get_configuration() const override;
  /// Set this algorithm's properties via a config block
  void set_configuration( vital::config_block_sptr config ) override;
  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Perform high pass filtering
  kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data ) override;

private:
  class priv;

  std::unique_ptr< priv > const d;
};

} // namespace ocv

} // namespace arrows

} // namespace kwiver

#endif
