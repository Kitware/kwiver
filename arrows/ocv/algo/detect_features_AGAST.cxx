// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV AGAST feature detector wrapper

#include "detect_features_AGAST.h"

// Only available in OpenCV 3.x
#if KWIVER_OPENCV_VERSION_MAJOR >= 3

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

/// Check that the given integer is one of the valid enum values
bool
check_agast_type( int const& type )
{
  switch( type )
  {
    case cv::AgastFeatureDetector::AGAST_5_8:
    case cv::AgastFeatureDetector::AGAST_7_12d:
    case cv::AgastFeatureDetector::AGAST_7_12s:
    case cv::AgastFeatureDetector::OAST_9_16:
      return true;
    default:
      return false;
  }
}

} // end namespace anonymous

/// Return multi-line, tabbed list string of available enum types and their
/// values
const std::string detect_features_AGAST::list_agast_types =
  "\tAGAST_5_8 = " KWIVER_STRINGIFY(
    cv::AgastFeatureDetector::AGAST_5_8 )  "\n"
                                           "\tAGAST_7_12d = "
  KWIVER_STRINGIFY( cv::AgastFeatureDetector::AGAST_7_12d ) "\n"
                                                            "\tAGAST_7_12s = "
  KWIVER_STRINGIFY( cv::AgastFeatureDetector::AGAST_7_12s ) "\n"
                                                            "\tOAST_9_16 = "
  KWIVER_STRINGIFY( cv::AgastFeatureDetector::OAST_9_16 );

void
detect_features_AGAST
::initialize()
{
  attach_logger( "arrows.ocv.AGAST" );
  detector = cv::AgastFeatureDetector::create(
    this->get_threshold(), this->get_nonmax_suppression(),
    cv::AgastFeatureDetector::DetectorType( this->get_type() ) );
}

detect_features_AGAST
::~detect_features_AGAST()
{}

void
detect_features_AGAST
::update_detector_parameters() const
{
  auto algo = detector.dynamicCast< cv::AgastFeatureDetector >();
  algo->setThreshold( this->get_threshold() );
  algo->setNonmaxSuppression( this->get_nonmax_suppression() );
  algo->setType( cv::AgastFeatureDetector::DetectorType( this->get_type() ) );
}

void
detect_features_AGAST
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_detector_parameters();
}

bool
detect_features_AGAST
::check_configuration( vital::config_block_sptr config ) const
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );

  bool valid = true;

  int t = c->get_value< int >( "type" );
  if( !check_agast_type( t ) )
  {
    LOG_ERROR(
      logger(), "Given AGAST type not valid. Must be one of:\n" +
      std::string( list_agast_types ) );
    valid = false;
  }

  return valid;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // KWIVER_OPENCV_VERSION_MAJOR >= 3
