// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV MSER feature detector wrapper implementation

#include "detect_features_MSER.h"

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

cv::Ptr< cv::MSER >
create( const detect_features_MSER& parent )
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  // 2.4.x version constructor
  return cv::Ptr< cv::MSER >(
    new cv::MSER(
      parent.get_delta(), parent.get_min_area(), parent.get_max_area(),
      parent.get_max_variation(),
      parent.get_min_diversity(), parent.get_max_evolution(),
      parent.get_area_threshold(),
      parent.get_min_margin(), parent.get_edge_blur_size() )
  );
#else
  cv::Ptr< cv::MSER > p =
    cv::MSER::create(
      parent.get_delta(), parent.get_min_area(), parent.get_max_area(),
      parent.get_max_variation(),
      parent.get_min_diversity(), parent.get_max_evolution(),
      parent.get_area_threshold(),
      parent.get_min_margin(), parent.get_edge_blur_size() );
  p->setPass2Only( parent.get_pass2only() );
  return p;
#endif
}

} // namespace

void
detect_features_MSER
::initialize()
{
  attach_logger( "arrows.ocv.detect_features_FAST" );
  detector = create( *this );
}

detect_features_MSER
::~detect_features_MSER()
{}

// OCV 3.x does not have adequate setter functions for updating all parameters
// the algorithm was constructed with. So, instead of updating, we'll just
// create a new cv::MSER instance on parameter update.
void
detect_features_MSER
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

void
detect_features_MSER
::update_detector_parameters() const
{
  this->detector.constCast< cv::FeatureDetector >() = create( *this );
}

bool
detect_features_MSER
::check_configuration( vital::config_block_sptr config ) const
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );

  bool valid = true;

  // checking that area values are >= 0
  if( c->get_value< int >( "min_area" ) < 0 ||
      c->get_value< int >( "max_area" ) < 0 ||
      c->get_value< double >( "area_threshold" ) < 0 )
  {
    LOG_ERROR(logger(), "Areas should be at least 0." );
    valid = false;
  }

  return valid;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
