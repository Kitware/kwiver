// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_BURNOUT_MOVING_BURNIN_DETECTOR_OPENCV_
#define KWIVER_ARROWS_BURNOUT_MOVING_BURNIN_DETECTOR_OPENCV_

#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <vital/algo/image_filter.h>

#include <opencv/cxcore.h>

#include <deque>
#include <vector>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT moving_burnin_detector_opencv
  : public vital::algo::image_filter
{
  public:
    PLUGIN_INFO( "burnout_moving_burnin_detector_opencv",
               "Detect burnin items from an image." )

    moving_burnin_detector_opencv();
    virtual ~moving_burnin_detector_opencv();

    /// Get this algorithm's \link vital::config_block configuration block
    /// \endlink
    virtual vital::config_block_sptr get_configuration() const;
    /// Set this algorithm's properties via a config block
    virtual void set_configuration( vital::config_block_sptr config );
    /// Check that the algorithm's currently configuration is valid
    virtual bool check_configuration( vital::config_block_sptr config ) const;

    /// detect burnin items
    virtual kwiver::vital::image_container_sptr filter(
      kwiver::vital::image_container_sptr image_data );

  private:
    class priv;

    std::unique_ptr< priv > const d;
};

} // namespace ocv

} // namespace arrows

} // namespace kwiver

#endif
