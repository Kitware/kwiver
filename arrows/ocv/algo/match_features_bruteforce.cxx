// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV brute-force feature matcher wrapper implementation

#include "match_features_bruteforce.h"

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

// Can't currently update parameters on BF implementation, so no update
// function. Will need to create a new instance on each parameter update.

/// Create a new brute-force matcher instance and set our matcher param to it
cv::Ptr< cv::BFMatcher >
create( int norm_type, bool cross_check )
{
  // cross version compatible
  return cv::Ptr< cv::BFMatcher >(
    new cv::BFMatcher( norm_type, cross_check )
  );
}

/// Check value against known OCV norm enum values
bool
check_norm_enum_value( int norm_type )
{
  switch( norm_type )
  {
    case cv::NORM_INF:
    case cv::NORM_L1:
    case cv::NORM_L2:
    case cv::NORM_L2SQR:
    case cv::NORM_HAMMING:
    case cv::NORM_HAMMING2:
    // case cv::NORM_TYPE_MASK:  // This is the same value as HAMMING2
    // apparently
    case cv::NORM_RELATIVE:
    case cv::NORM_MINMAX:
      return true;
    default:
      return false;
  }
}

} // namespace

void
match_features_bruteforce
::initialize()
{
  attach_logger( "arrows.ocv.match_features_bruteforce" );
}

match_features_bruteforce
::~match_features_bruteforce()
{}

void
match_features_bruteforce
::set_configuration_internal(
  [[maybe_unused]] vital::config_block_sptr in_config )
{
  // Create new instance with the updated parameters
  matcher = create( this->get_norm_type(), this->get_cross_check() );
}

bool
match_features_bruteforce
::check_configuration( vital::config_block_sptr in_config ) const
{
  vital::config_block_sptr config = get_configuration();
  config->merge_config( in_config );

  bool valid = true;

  // user has the chance to input an incorret value for the norm type enum value
  int norm_type = config->get_value< int >( "norm_type" );
  if( !check_norm_enum_value( norm_type ) )
  {
    std::stringstream ss;
    ss << "Incorrect norm type enum value given: '" << norm_type << "'. "
       << "Valid values are: " << match_features_bruteforce::list_enum_values;
    logger()->log_error( ss.str() );
    valid = false;
  }

  return valid;
}

void
match_features_bruteforce
::ocv_match(
  const cv::Mat& descriptors1, const cv::Mat& descriptors2,
  std::vector< cv::DMatch >& matches ) const
{
  // make sure matcher is up-to-date in case parameters where updated via
  // setters
  this->matcher.constCast< cv::BFMatcher >() = create(
    this->get_norm_type(),
    this->get_cross_check() );
  this->matcher->match( descriptors1, descriptors2, matches );
}

const std::string match_features_bruteforce::list_enum_values =
  "cv::NORM_INF="       KWIVER_STRINGIFY( cv::NORM_INF )       ", "
                                                               "cv::NORM_L1="
  KWIVER_STRINGIFY( cv::NORM_L1 )        ", "
                                         "cv::NORM_L2="
  KWIVER_STRINGIFY( cv::NORM_L2 )        ", "
                                         "cv::NORM_L2SQR="
  KWIVER_STRINGIFY( cv::NORM_L2SQR )     ", "
                                         "cv::NORM_HAMMING="
  KWIVER_STRINGIFY( cv::NORM_HAMMING )   ", "
                                         "cv::NORM_HAMMING2="
  KWIVER_STRINGIFY( cv::NORM_HAMMING2 )  ", "
                                         "cv::NORM_TYPE_MASK="
  KWIVER_STRINGIFY( cv::NORM_TYPE_MASK ) ", "
                                         "cv::NORM_RELATIVE="
  KWIVER_STRINGIFY( cv::NORM_RELATIVE )  ", "
                                         "cv::NORM_MINMAX="
  KWIVER_STRINGIFY( cv::NORM_MINMAX );

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
