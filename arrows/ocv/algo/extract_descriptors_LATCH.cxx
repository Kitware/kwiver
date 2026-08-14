// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV LATCH descriptor extractor wrapper implementation

#include "extract_descriptors_LATCH.h"

#ifdef HAVE_OPENCV_XFEATURES2D

#include <opencv2/xfeatures2d.hpp>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

cv::Ptr< cv::xfeatures2d::LATCH >
create( const extract_descriptors_LATCH& parent )
{
  return cv::xfeatures2d::LATCH::create(
    parent.get_bytes(), parent.get_rotation_invariance(),
    parent.get_half_ssd_size() );
}

} // namespace

void
extract_descriptors_LATCH
::initialize()
{
  attach_logger( "arrows.ocv.LATCH" );
  extractor = create( *this );
}

extract_descriptors_LATCH
::~extract_descriptors_LATCH()
{}

void
extract_descriptors_LATCH
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );
}

bool
extract_descriptors_LATCH
::check_configuration( vital::config_block_sptr config ) const
{
  config_block_sptr c = get_configuration();
  c->merge_config( config );

  bool valid = true;

  // Bytes can only be one of the following values
  int bytes_ = c->get_value< int >( "bytes" );
  if( !( bytes_ == 1 ||
         bytes_ == 2 ||
         bytes_ == 4 ||
         bytes_ == 8 ||
         bytes_ == 16 ||
         bytes_ == 32 ||
         bytes_ == 64 ) )
  {
    LOG_ERROR(
      logger(), "bytes value must be one of [1, 2, 4, 8, 16, 32, 64]. "
                "Given: " << bytes_ );
    valid = false;
  }

  return valid;
}

void
extract_descriptors_LATCH
::update_extractor_parameters() const
{
  extractor.constCast< cv::DescriptorExtractor >() = create( *this );
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D
