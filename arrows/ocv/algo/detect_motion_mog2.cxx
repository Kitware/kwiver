// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of ocv::detect_motion_MOG2

#include "detect_motion_mog2.h"

#include <vital/exceptions.h>
#include <vital/types/matrix.h>
#include <vital/vital_config.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/core/eigen.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

namespace kwiver {

namespace arrows {

namespace ocv {

using namespace kwiver::vital;

// ----------------------------------------------------------------------------

/// Private implementation class
class detect_motion_mog2::priv
{
  detect_motion_mog2& parent;

public:
  /// Parameters
  int m_frame_count;
  int
  m_history() const { return parent.get_history(); }
  double
  m_var_threshold() const { return parent.get_var_threshold(); }
  double
  m_learning_rate() const { return parent.get_learning_rate(); }
  int
  m_blur_kernel_size() const { return parent.get_blur_kernel_size(); }
  int
  m_min_frames() const { return parent.get_min_frames(); }

  int m_nmixtures;
  double m_max_foreground_fract() { return parent.get_max_foreground_fract(); }

  cv::Ptr< cv::BackgroundSubtractorMOG2 > bg_model;
  image_container_sptr motion_heat_map;
  kwiver::vital::logger_handle_t m_logger;

  /// Constructor
  priv( detect_motion_mog2& parent )
    : parent( parent ),
      m_frame_count( 0 ),
      m_nmixtures( 3 )
  {}

  /// Create new impl instance based on current parameters
  void
  reset()
  {
    m_frame_count = 0;
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
    bg_model = cv::createBackgroundSubtractorMOG2(
      m_history(), m_var_threshold(),
      false );
    bg_model->setNMixtures( m_nmixtures );
#else
    bg_model = new cv::BackgroundSubtractorMOG2(
      m_history(), m_var_threshold(),
      false );
    bg_model->set( "nmixtures", m_nmixtures );
#endif
  }
};

void
detect_motion_mog2
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.ocv.detect_motion_mog2" );
  d_->m_logger = logger();
  d_->reset();
}

/// Destructor
detect_motion_mog2
::~detect_motion_mog2() noexcept
{}

/// Set this algo's properties via a config block
void
detect_motion_mog2
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  if( d_->m_max_foreground_fract() < 0 || d_->m_max_foreground_fract() > 1 )
  {
    VITAL_THROW(
      algorithm_configuration_exception, interface_name(), impl_name(),
      "max_foreground_fract must be in "
      "the range 0-1." );
  }

  if( d_->m_min_frames() < 0 )
  {
    VITAL_THROW(
      algorithm_configuration_exception, interface_name(), impl_name(),
      "min_frames must be greater than zero." );
  }

  LOG_DEBUG(
    logger(),
    "var_threshold: " << std::to_string( d_->m_var_threshold() ) );
  LOG_DEBUG( logger(), "history: " << std::to_string( d_->m_history() ) );
  LOG_DEBUG(
    logger(),
    "learning_rate: " << std::to_string( d_->m_learning_rate() ) );
  LOG_DEBUG(
    logger(),
    "blur_kernel_size: " << std::to_string( d_->m_blur_kernel_size() ) );
  LOG_DEBUG(
    logger(),
    "max_foreground_fract: " <<
      std::to_string( d_->m_max_foreground_fract() ) );
}

bool
detect_motion_mog2
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

/// Detect motion from a sequence of images
image_container_sptr
detect_motion_mog2
::process_image(
  [[maybe_unused]] const timestamp& ts,
  const image_container_sptr image,
  bool reset_model )
{
  if( !image )
  {
    VITAL_THROW(
      vital::invalid_data,
      "Inputs to ocv::detect_motion_mog2 are null" );
  }

  if( reset_model )
  {
    LOG_TRACE( logger(), "Received command to reset background model" );
    d_->reset();
  }

  cv::Mat cv_src;
  ocv::image_container::vital_to_ocv(
    image->get_image(),
    image_container::BGR_COLOR ).copyTo( cv_src );

  if( d_->m_blur_kernel_size() > 0 )
  {
    cv::blur(
      cv_src, cv_src,
      cv::Size( d_->m_blur_kernel_size(), d_->m_blur_kernel_size() ) );
  }

  cv::Mat fgmask;
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
  d_->bg_model->apply( cv_src, fgmask, d_->m_learning_rate() );
#else
  d_->bg_model->operator()( cv_src, fgmask, d_->m_learning_rate );
#endif
  LOG_TRACE( logger(), "Finished MOG2 motion detector for this iteration" );

  ++d_->m_frame_count;

  if( d_->m_frame_count < d_->m_min_frames() )
  {
    LOG_TRACE(
      logger(), "Haven't collected enough frames yet, so setting "
                "foreground mask to all zeros." );
    fgmask = cv::Scalar( 0 );
  }
  else
  {
    if( d_->m_max_foreground_fract() < 1 )
    {
      int total_pixels = fgmask.rows * fgmask.cols;
      int max_fg_pixels = total_pixels * d_->m_max_foreground_fract();
      int nonzero_pixels = cv::countNonZero( fgmask );
      LOG_TRACE(
        logger(), ( double ) nonzero_pixels / ( double ) total_pixels * 100 <<
          "% foreground pixels." );
      if( nonzero_pixels > max_fg_pixels )
      {
        LOG_DEBUG(
          logger(), "Foreground pixels exceed maximum set to " <<
            d_->m_max_foreground_fract() * 100 << "%, something must have "
                                                "failed. Resetting background model." );

        // Reset background model, but wait until next iteration to start
        // updating it because the current frame might be bad.
        d_->reset();
        fgmask = cv::Scalar( 0 );
      }
    }
  }

  d_->motion_heat_map = std::make_shared< ocv::image_container >(
    fgmask,
    image_container::BGR_COLOR );

  return d_->motion_heat_map;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
