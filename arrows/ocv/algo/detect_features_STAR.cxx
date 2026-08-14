// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV Star feature detector wrapper implementation

#include "detect_features_STAR.h"

#if KWIVER_OPENCV_VERSION_MAJOR < 3 || defined( HAVE_OPENCV_XFEATURES2D )

#include <vital/vital_config.h>

#if KWIVER_OPENCV_VERSION_MAJOR < 3
typedef cv::StarDetector cv_STAR_t;
#else
#include <opencv2/xfeatures2d.hpp>
typedef cv::xfeatures2d::StarDetector cv_STAR_t;
#endif

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace  {

cv::Ptr< cv_STAR_t >
create( const detect_features_STAR& parent )
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  return cv::Ptr< cv_STAR_t >(
    new cv_STAR_t(
      parent.get_max_size(), parent.get_response_threshold(),
      parent.get_line_threshold_projected(),
      parent.get_line_threshold_binarized(), parent.get_suppress_nonmax_size() )
  );
#else
  return cv_STAR_t::create(
    parent.get_max_size(), parent.get_response_threshold(),
    parent.get_line_threshold_projected(),
    parent.get_line_threshold_binarized(), parent.get_suppress_nonmax_size() );
#endif
}

} // namespace

void
detect_features_STAR
::initialize()
{
  attach_logger( "arrows.ocv.star" );
  detector = create( *this );
}

detect_features_STAR
::~detect_features_STAR()
{}

void
detect_features_STAR
::update_detector_parameters() const
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  cv::Ptr< cv_STAR_t > a  = detector;
  a->set( "maxSize", max_size() );
  a->set( "responseThreshold", response_threshold() );
  a->set( "lineThresholdProjected", line_threshold_projected() );
  a->set( "lineThresholdBinarized", line_threshold_binarized() );
  a->set( "suppressNonmaxSize", suppress_nonmax_size() );
#else
  detector.constCast<  cv::Feature2D  >() = create( *this );
#endif
}

void
detect_features_STAR
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

bool
detect_features_STAR
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // has OCV support
