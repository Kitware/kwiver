// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV FREAK descriptor extractor wrapper implementation

#include "extract_descriptors_FREAK.h"

#if KWIVER_OPENCV_VERSION_MAJOR < 3 || defined( HAVE_OPENCV_XFEATURES2D )

#include <vital/vital_config.h>

// typedef FREAK into a common symbol
#if KWIVER_OPENCV_VERSION_MAJOR < 3
typedef cv::FREAK cv_FREAK_t;
#else
#include <opencv2/xfeatures2d.hpp>
typedef cv::xfeatures2d::FREAK cv_FREAK_t;
#endif

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

/// Create new cv::Ptr algo instance
cv::Ptr< cv_FREAK_t >
create( const extract_descriptors_FREAK& parent )
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  return cv::Ptr< cv_FREAK_t >(
    new cv_FREAK_t(
      parent.get_orientation_normalized(), parent.get_scale_normalized(),
      parent.get_pattern_scale(),
      parent.get_n_octaves() )
  );
#else
  return cv_FREAK_t::create(
    parent.get_orientation_normalized(), parent.get_scale_normalized(),
    parent.get_pattern_scale(), parent.get_n_octaves() );
#endif
}

} // namespace

void
extract_descriptors_FREAK
::initialize()
{
  attach_logger( "arrows.ocv.FREAK" );
  extractor = create( *this );
}

/// Destructor
extract_descriptors_FREAK
::~extract_descriptors_FREAK()
{}

void
extract_descriptors_FREAK
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  this->update_extractor_parameters();
}

void
extract_descriptors_FREAK
::update_extractor_parameters() const
{
#if KWIVER_OPENCV_VERSION_MAJOR < 3
  cv::Ptr< cv_FREAK_t > freak = extractor.dynamicCast< cv_FREAK_t >();
  freak->set( "orientationNormalized", parent.get_orientation_normalized() );
  freak->set( "scaleNormalized", parent.get_scale_normalized() );
  freak->set( "patternScale", parent.get_pattern_scale() );
  freak->set( "nbOctave", parent.get_n_octaves() );
#else
  extractor.constCast< cv::DescriptorExtractor >() = create( *this );
#endif
}

bool
extract_descriptors_FREAK
::check_configuration(
  [[maybe_unused]] vital::config_block_sptr in_config ) const
{
  return true;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // KWIVER_OPENCV_VERSION_MAJOR < 3 || defined(HAVE_OPENCV_XFEATURES2D)
