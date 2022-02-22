// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_OCV_INPAINT_
#define KWIVER_ARROWS_OCV_INPAINT_

#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <vital/algo/merge_images.h>

namespace kwiver {

namespace arrows {

namespace ocv {

/// OCV image inpainting process.
///
/// Replace pixels in the image specified by non-zero elements in the mask with
/// inpainted values estimated from surrounding pixels.
class KWIVER_ALGO_OCV_EXPORT inpaint
  : public vital::algo::merge_images
{
public:
  PLUGIN_INFO( "ocv_inpainting",
               "Inpaint pixels specified by non-zero mask values." )

  inpaint();
  virtual ~inpaint();

  /// Get this algorithm's \link vital::config_block configuration block
  /// \endlink
  vital::config_block_sptr get_configuration() const override;
  /// Set this algorithm's properties via a config block
  void set_configuration( vital::config_block_sptr config ) override;
  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Inpaint image based on locations specied in mask
  kwiver::vital::image_container_sptr merge(
    kwiver::vital::image_container_sptr image,
    kwiver::vital::image_container_sptr mask ) const override;

private:
  class priv;

  std::unique_ptr< priv > const d;
};

} // namespace ocv

} // namespace arrows

} // namespace kwiver

#endif
