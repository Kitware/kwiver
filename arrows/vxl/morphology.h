// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_MORPHOLOGY_
#define KWIVER_ARROWS_VXL_MORPHOLOGY_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// Convert between VXL image formats.
///
/// This can be used, for example, to turn a floating point image into
/// a byte image and vice versa.
class KWIVER_ALGO_VXL_EXPORT morphology
  : public vital::algo::image_filter
{
public:
  PLUGIN_INFO( "vxl_morphology",
               "Apply channel-wise morphological operations and "
               "optionally merge across channels." )

  morphology();
  ~morphology();

  /// Get this algorithm's \link vital::config_block configuration block
  /// \endlink.
  virtual vital::config_block_sptr get_configuration() const;
  /// Set this algorithm's properties via a config block.
  virtual void set_configuration( vital::config_block_sptr config );
  /// Check that the algorithm's currently configuration is valid.
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Convert to the right type and optionally transform.
  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data );

private:
  class priv;

  std::unique_ptr< priv > const d;
};

} // namespace vxl

} // namespace arrows

} // namespace kwiver

#endif
