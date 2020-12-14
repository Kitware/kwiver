// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_BURNOUT_IMAGE_ENHANCER
#define KWIVER_ARROWS_BURNOUT_IMAGE_ENHANCER

#include <arrows/burnout/kwiver_algo_burnout_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {
namespace arrows {
namespace burnout {

/**
 * @brief Burnout Image Filtering
 *
 * This method contains basic methods for image filtering on top of input
 * images via automatic white balancing and smoothing.
 */
class KWIVER_ALGO_BURNOUT_EXPORT burnout_image_enhancer
  : public vital::algo::image_filter
{
public:
  burnout_image_enhancer();
  virtual ~burnout_image_enhancer();

  PLUGIN_INFO( "burnout_enhancer",
               "Image filtering using burnout" )

  vital::config_block_sptr get_configuration() const override;

  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data ) override;

private:

  class priv;
  const std::unique_ptr<priv> d;
};

} } }

#endif /* KWIVER_ARROWS_BURNOUT_IMAGE_ENHANCER */
