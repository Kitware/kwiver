// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV simple blob feature detector wrapper implementation

#include "detect_features_simple_blob.h"

#include <vital/vital_config.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

cv::SimpleBlobDetector::Params
create_config( const detect_features_simple_blob& parent )
{
  cv::SimpleBlobDetector::Params p;
  p.thresholdStep = parent.get_threshold_step();
  p.minThreshold = parent.get_threshold_min();
  p.maxThreshold = parent.get_threshold_max();
  p.minRepeatability = parent.get_min_repeatability();
  p.minDistBetweenBlobs = parent.get_min_dist_between_blocks();

  p.filterByColor = parent.get_filter_by_color();
  p.blobColor = parent.get_blob_color();

  p.filterByArea = parent.get_filter_by_area();
  p.minArea = parent.get_min_area();
  p.maxArea = parent.get_max_area();

  p.filterByCircularity = parent.get_filter_by_circularity();
  p.minCircularity = parent.get_min_circularity();
  p.maxCircularity = parent.get_max_circularity();

  p.filterByInertia = parent.get_filter_by_inertia();
  p.minInertiaRatio = parent.get_min_inertia_ratio();
  p.maxInertiaRatio = parent.get_max_inertia_ratio();

  p.filterByConvexity = parent.get_filter_by_convexity();
  p.minConvexity = parent.get_min_convexity();
  p.maxConvexity = parent.get_max_convexity();
  return p;
}

/// Create new algorithm based on current parameter values
cv::Ptr< cv::SimpleBlobDetector >
create( const cv::SimpleBlobDetector::Params& p )
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  return cv::Ptr< cv::SimpleBlobDetector >(
    new cv::SimpleBlobDetector( p )
  );
#else
  return cv::SimpleBlobDetector::create( p );
#endif
}

} // namespace


// --------------------------------------------------------
const cv::SimpleBlobDetector::Params detect_features_simple_blob::default_params {};

void
detect_features_simple_blob
::initialize()
{
  attach_logger( "arrows.ocv.simple_blob_detector" );


  auto params = create_config( *this );
  detector = create( params );
}

detect_features_simple_blob
::~detect_features_simple_blob()
{}

void
detect_features_simple_blob
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

void
detect_features_simple_blob
::update_detector_parameters() const
{
  auto params = create_config( *this );
  detector.constCast< cv::Feature2D >() = create( params );
}

bool
detect_features_simple_blob
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
