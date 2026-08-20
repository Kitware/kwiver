// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV DAISY descriptor extractor wrapper implementation

#include "extract_descriptors_DAISY.h"

#ifdef HAVE_OPENCV_XFEATURES2D

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

std::string
generate_list_norm_options()
{
  std::stringstream ss;
  ss << "\tNRM_NONE    = " << cv::xfeatures2d::DAISY::NRM_NONE << "\n"
     << "\tNRM_PARTIAL = " << cv::xfeatures2d::DAISY::NRM_PARTIAL << "\n"
     << "\tNRM_FULL    = " << cv::xfeatures2d::DAISY::NRM_FULL << "\n"
     << "\tNRM_SIFT    = " << cv::xfeatures2d::DAISY::NRM_SIFT;
  return ss.str();
}

bool
check_norm_type( int norm )
{
  switch( norm )
  {
    case cv::xfeatures2d::DAISY::NRM_NONE:
    case cv::xfeatures2d::DAISY::NRM_PARTIAL:
    case cv::xfeatures2d::DAISY::NRM_FULL:
    case cv::xfeatures2d::DAISY::NRM_SIFT:
      return true;
    default:
      return false;
  }
}

cv::Ptr< cv::xfeatures2d::DAISY >
create( const extract_descriptors_DAISY& parent )
{
  // TODO: Allow custom homography matrix?
  return cv::xfeatures2d::DAISY::create(
    parent.get_radius(), parent.get_q_radius(), parent.get_q_theta(),
    parent.get_q_hist(),
    cv::xfeatures2d::DAISY::NormalizationType( parent.get_norm() ),
    cv::noArray(), parent.get_interpolation(),
    parent.get_use_orientation() );
}

} // end namespace anonymous

const std::string extract_descriptors_DAISY::list_norm_options =
  generate_list_norm_options();

void
extract_descriptors_DAISY
::initialize()
{
  attach_logger( "arrows.ocv.DAISY" );
  extractor = create( *this );
}

extract_descriptors_DAISY
::~extract_descriptors_DAISY()
{}

void
extract_descriptors_DAISY
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_extractor_parameters();
}

void
extract_descriptors_DAISY
::update_extractor_parameters() const
{
  extractor.constCast< cv::DescriptorExtractor >() = create( *this );
}

bool
extract_descriptors_DAISY
::check_configuration( vital::config_block_sptr config ) const
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );

  bool valid = true;

  int n = c->get_value< int >( "norm" );
  if( !check_norm_type( n ) )
  {
    LOG_ERROR(
      this->logger(), "Invalid norm option '" << n << "'. Valid choices "
                                                      "are: " <<
        extract_descriptors_DAISY::list_norm_options );
    valid = false;
  }

  return valid;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D
