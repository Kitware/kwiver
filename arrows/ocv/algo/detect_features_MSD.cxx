// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV MSD feature detector wrapper

#include "detect_features_MSD.h"

#include <vital/vital_config.h>

// Only available in OpenCV 3.x xfeatures2d
#ifdef HAVE_OPENCV_XFEATURES2D

#include <opencv2/xfeatures2d.hpp>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace  {

/// Create algorithm instance
cv::Ptr< cv::xfeatures2d::MSDDetector >
create( const detect_features_MSD& parent )
{
  return cv::xfeatures2d::MSDDetector::create(
    parent.get_patch_radius(), parent.get_search_area_radius(),
    parent.get_nms_radius(), parent.get_nms_scale_radius(),
    parent.get_th_saliency(), parent.get_knn(), parent.get_scale_factor(),
    parent.get_n_scales(), parent.get_compute_orientation()
  );
}

} // namespace

void
detect_features_MSD
::initialize()
{
  detector = create( *this );
}

detect_features_MSD
::~detect_features_MSD()
{}

void
detect_features_MSD
::update_detector_parameters() const
{
  detector.constCast<  cv::Feature2D  >() = create( *this );
}

void
detect_features_MSD
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

bool
detect_features_MSD
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D
