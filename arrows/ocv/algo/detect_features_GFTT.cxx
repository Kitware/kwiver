// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV GFTT feature detector wrapper implementation

#include "detect_features_GFTT.h"

#include <vital/vital_config.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

/// Create a new GFTT detector instance with the current parameter values
cv::Ptr< cv::GFTTDetector >
create( const detect_features_GFTT& parent )
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  return cv::Ptr< cv::GFTTDetector >(
    new cv::GFTTDetector(
      parent.get_max_corners(), parent.get_quality_level(),
      parent.get_min_distance(),
      parent.get_block_size(), parent.get_use_harris_detector(),
      parent.get_k() )
  );
#else
  return cv::GFTTDetector::create(
    parent.get_max_corners(), parent.get_quality_level(),
    parent.get_min_distance(),
    parent.get_block_size(), parent.get_use_harris_detector(), parent.get_k() );
#endif
}

} // namespace

void
detect_features_GFTT
::initialize()
{
  attach_logger( "arrows.ocv.GFTT" );
  detector = create( *this );
}

detect_features_GFTT
::~detect_features_GFTT()
{}

void
detect_features_GFTT
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

bool
detect_features_GFTT
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  // Nothing to explicitly check
  return true;
}

void
detect_features_GFTT
::update_detector_parameters() const
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  // since 2.4.x does not have params set for everything that's given to the
  // constructor, lets just remake the algo instance.
  this->detector.constCast< cv::GFTTDetector >() = create( *this );
#else
  auto a = detector.dynamicCast< cv::GFTTDetector >();
  a->setMaxFeatures( this->get_max_corners() );
  a->setQualityLevel( this->get_quality_level() );
  a->setMinDistance( this->get_min_distance() );
  a->setBlockSize( this->get_block_size() );
  a->setHarrisDetector( this->get_use_harris_detector() );
  a->setK( this->get_k() );
#endif
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
