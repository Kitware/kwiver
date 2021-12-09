// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "moving_burnin_detector_opencv.h"

#include <arrows/ocv/image_container.h>

#include <opencv/cxcore.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>

#define HAVE_COLOR_IMAGE


namespace kwiver {

namespace arrows {

namespace ocv {

// ----------------------------------------------------------------------------
// private implementation class
class moving_burnin_detector_opencv::priv
{
public:
  // constructor
  priv( moving_burnin_detector_opencv* parent ) : p( parent )
  {}

  ~priv()
  {}

  /// Check if the input image is valid
  bool set_input_image( cv::Mat const& img);
  /// Scale the given input parameters if the target resolution is different
  /// from the input resolution
  void scale_params_for_image( cv::Mat const & img );

  moving_burnin_detector_opencv* p;

  struct metadata
  {
    cv::Point center;
  };

  /// Draw the detected burnin on the image
  void draw_metadata_mask( cv::Mat& img, const metadata& md );
  /// Write out the values of aspect ratios and jitters that were actually
  /// used in the final detections.
  void log_detection_stats( const metadata& md );

  /// Detect cross hairs in an image
  void detect_cross( const cv::Mat& edges, cv::Point& center );
  /// Draw cross hair template
  void draw_cross( cv::Mat& img, const cv::Point & center, double clr, int width );

  kwiver::vital::image_container_sptr filter( kwiver::vital::image_container_sptr image );

  bool invalid_image;
  cv::Mat byte_mask;
  int w;
  int h;

  // config default values
  bool disabled{ false };

  bool highest_score_only{ false };
  double line_width{ 3 };
  double draw_line_width{ 3 };
  double roi_ratio{ 0.5 };
  double roi_aspect{ 0 };
  int off_center_x{ 0 };
  int off_center_y{ 0 };

  double cross_threshold{ 0.2 };
  int cross_gap_x{ 6 };
  int cross_gap_y{ 6 };
  int cross_length_x{ 14 };
  int cross_length_y{ 14 };
  float cross_ends_ratio{ -1.0 };

  int off_center_jitter{ 1 };
  unsigned target_resolution_x{ 0 };
  unsigned target_resolution_y{ 0 };
  bool verbose{ true };
};

// ----------------------------------------------------------------------------
moving_burnin_detector_opencv
::moving_burnin_detector_opencv()
  : d{ new priv{ this } }
{
  attach_logger( "arrows.ocv.moving_burnin_detector_opencv" );
}

moving_burnin_detector_opencv
::~moving_burnin_detector_opencv()
{}

// ----------------------------------------------------------------------------------
vital::config_block_sptr
moving_burnin_detector_opencv
::get_configuration() const
{
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value( "disabled",
                     d->disabled,
                     "Disable this process, causing the image to pass through unmodified." );
  config->set_value( "highest_score_only",
                     d->highest_score_only,
                     "If there are multiple detections of a given category, should only the one "
                     "with the highest score be reported as a detection?" );
  config->set_value( "line_width",
                     d->line_width,
                     "Width of lines in the metadata" );
  config->set_value( "draw_line_width",
                     d->draw_line_width,
                     "Width of lines in the mask output" );
  config->set_value( "roi_ratio",
                     d->roi_ratio,
                     "Proportion of the width of the frame size (centered) to look for burnin" );
  config->set_value( "roi_aspect",
                     d->roi_aspect,
                     "Aspect ratio of the brackets. (height/width; 0 for same as frame)" );
  config->set_value( "off_center_x",
                     d->off_center_x,
                     "Horizontal offset of the center of the brackets from the center of the frame" );
  config->set_value( "off_center_y",
                     d->off_center_y,
                     "Vertical offset of the center of the brackets from the center of the frame" );
  // crosshair
  config->set_value( "cross_threshold",
                      d->cross_threshold,
                      "Minimum coverage of the cross with edge detection (negative to disable)" );
  config->set_value( "cross_gap_x",
                     d->cross_gap_x,
                    "Horizontal gap between cross segments" );
  config->set_value( "cross_gap_y",
                     d->cross_gap_y,
                     "Vertical gap between cross segments" );
  config->set_value( "cross_length_x",
                     d->cross_length_x,
                     "Length of horizontal cross segments" );
  config->set_value( "cross_length_y",
                     d->cross_length_y,
                    "Length of vertical cross segments" );
  config->set_value( "cross_ends_ratio",
                     d->cross_ends_ratio,
                     "The 'ends' of cross are the perpendicular lines at the outer ends of the cross,"
                     " e.g. if this is ratio of length of '|' to '---' in one the left leg '|---' "
                     " of the cross-hair. This is a ratio between the length of this end to the inner"
                     " segment" );

  // TODO: add brackets and text

  config->set_value( "off_center_jitter",
                     d->off_center_jitter,
                     "Offset from center to search for brackets and the cross" );
  config->set_value( "target_resolution_x",
                     d->target_resolution_x,
                     "Image column resolution that these settings were designed for, if known." );
  config->set_value( "target_resolution_y",
                     d->target_resolution_y,
                     "Image row resolution that these settings were designed for, if known." );
  config->set_value( "verbose",
                     d->verbose,
                     "Enable additional log messages about detection that are useful in parameter tuning." );

  return config;
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv
::set_configuration( vital::config_block_sptr in_config )
{
  // Start with our generated vital::config_block to ensure that assumed values
  // are present. An alternative would be to check for key presence before
  // performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  d->disabled = config->get_value< bool >("disabled");
  d->highest_score_only = config->get_value< bool >("highest_score_only");
  d->line_width = config->get_value< double >("line_width");
  d->draw_line_width = config->get_value< double >("draw_line_width");
  d->roi_ratio = config->get_value< double >("roi_ratio");
  d->roi_aspect = config->get_value< double >("roi_aspect");
  d->off_center_x = config->get_value< int >("off_center_x");
  d->off_center_y = config->get_value< int >("off_center_y");

  // crosshair
  d->cross_threshold = config->get_value< double >("cross_threshold");
  d->cross_gap_x = config->get_value< int >("cross_gap_x");
  d->cross_gap_y = config->get_value< int >("cross_gap_y");
  d->cross_length_x = config->get_value< int >("cross_length_x");
  d->cross_length_y = config->get_value< int >("cross_length_y");
  d->cross_ends_ratio = config->get_value< float >("cross_ends_ratio");

  // TODO: add brackets and text

  d->off_center_jitter = config->get_value< int >("off_center_jitter");
  d->target_resolution_x = config->get_value< unsigned >("target_resolution_x");
  d->target_resolution_y = config->get_value< unsigned >("target_resolution_y");
  d->verbose = config->get_value< bool >("verbose");
}

// ----------------------------------------------------------------------------
bool
moving_burnin_detector_opencv
::check_configuration( vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------------
bool
moving_burnin_detector_opencv::priv
::set_input_image( cv::Mat const& img )
{
  //bool invalid_image = true;
  if( img.channels() != 1 && img.channels() != 3)
  {
    LOG_ERROR( p->logger(), "Input image does not have either 1 or 3 channels" );
    invalid_image = true;
    return invalid_image;
  }

  //if( ( input_image.nplanes() != 1 && input_image.planestep() != 1 ) ||
  //    input_image.istep() != input_image.nplanes() ||
  //    input_image.jstep() != ( input_image.ni() * input_image.nplanes() ) )
  //{
  //  LOG_ERROR( p->logger(), "Input image memory is not aligned for Opencv" );
  //  invalid_image = true;
  //}

  invalid_image = false;
  return invalid_image;
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::scale_params_for_image( cv::Mat const& img )
{
  if( target_resolution_x == 0 ||
      target_resolution_y == 0 ||
      ( target_resolution_x == img.cols &&
        target_resolution_y == img.rows ) )
  {
    //nothing to do here
    return;
  }

  double scale_factor_x = static_cast<double>( img.cols ) / target_resolution_x;
  double scale_factor_y = static_cast<double>( img.rows ) / target_resolution_y;

  // TODO: add template images for text templates

  double avg_scale_factor = ( scale_factor_x + scale_factor_y ) / 2.0;

  double min_draw_line_width = ( draw_line_width >= 3.0 ? 3.0 : draw_line_width );
  double min_line_width = ( line_width >= 1.0 ? 1.0 : line_width );

  line_width = std::max( line_width * avg_scale_factor, min_line_width );
  draw_line_width = std::max( draw_line_width * avg_scale_factor, min_draw_line_width );
  off_center_x = static_cast<int>( off_center_x * scale_factor_x + 0.5 );
  off_center_y = static_cast<int>( off_center_y * scale_factor_y + 0.5 );
  cross_gap_x = static_cast<int>( cross_gap_x * scale_factor_x + 0.5 );
  cross_gap_y = static_cast<int>( cross_gap_y * scale_factor_y + 0.5 );
  cross_length_x = static_cast<int>( cross_length_x * scale_factor_x + 0.5 );
  cross_length_y = static_cast<int>( cross_length_y * scale_factor_y + 0.5 );
  // TODO: add brackets

  target_resolution_x = img.cols;
  target_resolution_y = img.rows;
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_metadata_mask( cv::Mat& img, const metadata& md )
{
  if ( md.center.x != 0 && md.center.y != 0 )
  {
  draw_cross( img, md.center, 255, draw_line_width );
  }
  // TODO: add brackets and text
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::log_detection_stats( const metadata& md )
{
  cv::Point cntr = cv::Point( w / 2, h / 2 ) + cv::Point( off_center_x, off_center_y );

  if ( md.center.x != 0 && md.center.y != 0 )
  {
    cv::Point jitter = cntr - md.center;
    LOG_INFO( p->logger(), "Cross jitter: [" << jitter.x <<", "<< jitter.y << "]" );
  }

  // TODO: add brackets
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::detect_cross( const cv::Mat& edge_image, cv::Point& center )
{
  double score = 0;
  cv::Mat cross = cv::Mat::zeros( h, w, CV_8UC1 );
  cv::Mat buffer;
  cv::Point cntr = cv::Point( w / 2, h / 2) + cv::Point( off_center_x, off_center_y );

  draw_cross( cross, cntr, 255, line_width );
  const int cross_count = cv::countNonZero( cross );
  draw_cross( cross, cntr, 0, line_width );

  for ( int i = -off_center_jitter; i <= off_center_jitter; ++i )
  {
    for ( int j = -off_center_jitter; j <= off_center_jitter; ++j )
    {
      const cv::Point ct = cntr + cv::Point( j, i );

      const cv::Rect bb( ct - cv::Point( cross_length_x + cross_gap_x,
                                         cross_length_y + cross_gap_y ),
                         cv::Size( 2 * ( cross_length_x + cross_gap_x ),
                                   2 * ( cross_length_y + cross_gap_y ) ) );
     draw_cross( cross, ct, 255, line_width );
     cv::Mat edgev = edge_image(bb);
     cv::Mat crossv = cross(bb);
     cv::bitwise_and( edgev, crossv, buffer );
     draw_cross( cross, ct, 0, line_width );

     const int edge_count = cv::countNonZero( buffer );

     const double s = double(edge_count) / double(cross_count);
     std::cout << "score: " << s << std::endl;

     if ( s > score )
     {
       center = ct;
       score = s;
     }
    }
  }

  if ( score < cross_threshold )
  {
    center = cv::Point( 0, 0 );
  }
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_cross( cv::Mat& img, const cv::Point& center, double clr, int width )
{
  const cv::Scalar color = cv::Scalar( clr );

  // Middle cross
  cv::line( img, center + cv::Point( cross_gap_x, 0 ), center +
                          cv::Point( cross_gap_x + cross_length_x, 0), color, width );
  cv::line( img, center + cv::Point( -cross_gap_x, 0 ), center +
                          cv::Point( -cross_gap_x - cross_length_x, 0), color, width );
  cv::line( img, center + cv::Point( 0, cross_gap_y ), center +
                          cv::Point( 0, cross_gap_y + cross_length_y ), color, width );
  cv::line( img, center + cv::Point( 0, -cross_gap_y ), center +
                          cv::Point( 0, -cross_gap_y - cross_length_y ), color, width );

  // Perpendicular ends
  if( cross_ends_ratio > 0.0)
  {
    // Right
    cv::line( img, center + cv::Point( cross_gap_x + cross_length_x,
                                       static_cast<int>(-cross_ends_ratio * cross_length_x )),
                   center + cv::Point( cross_gap_x + cross_length_x,
                                       static_cast<int>( cross_ends_ratio * cross_length_x )), color, width );
    // Left
    cv::line( img, center + cv::Point(-cross_gap_x - cross_length_x+1,
                                       static_cast<int>(-cross_ends_ratio * cross_length_x )),
                   center + cv::Point(-cross_gap_x - cross_length_x+1,
                                       static_cast<int>( cross_ends_ratio * cross_length_x )), color, width );
    // Bottom
    cv::line( img, center + cv::Point( static_cast<int>(-cross_ends_ratio * cross_length_y ),
                                       cross_gap_y + cross_length_y ),
                   center + cv::Point( static_cast<int>( cross_ends_ratio * cross_length_y ),
                                       cross_gap_y + cross_length_y ), color, width );
    // Top
    cv::line( img, center + cv::Point( static_cast<int>(-cross_ends_ratio * cross_length_y ),
                                      -cross_gap_y - cross_length_y ),
                   center + cv::Point( static_cast<int>( cross_ends_ratio * cross_length_y ),
                                      -cross_gap_y - cross_length_y ), color, width );
  }
}

// ----------------------------------------------------------------------------------
// main processing step
kwiver::vital::image_container_sptr
moving_burnin_detector_opencv::priv
::filter( kwiver::vital::image_container_sptr input_image )
{
  // convert image to cv
  cv::Mat cv_image = ocv::image_container::vital_to_ocv(
    input_image->get_image(),
    ocv::image_container::RGB_COLOR );

  //bool invalid_image;
  invalid_image = set_input_image( cv_image );
  if( invalid_image ){
    LOG_ERROR( p->logger(), "Invalid image" );
    return kwiver::vital::image_container_sptr();
  }

  scale_params_for_image( cv_image );

  w = cv_image.cols;
  h = cv_image.rows;
  //roi_aspect = roi_aspect ? roi_aspect : ( double(h) / double(w) );
  const int nw = w * roi_ratio;
  const int nh = roi_aspect ? ( nw * roi_aspect ) : ( h * roi_ratio );

  /*if( byte_mask.size() != cv_image.size() )
  {
    byte_mask = cv::Mat( h, w );
  }
  byte_mask = cv::Mat::zeros( cv_image.rows, cv_image.cols, cv_image.type() )
  */

  cv::Mat input( h, w,
                 ( cv_image.channels() == 1 ) ? CV_8UC1 : CV_8UC3,
                 cv_image.ptr(0, 0) );
  //cv::cvtColor(input, input, BGR2RGB);
  cv::Mat end_mask( cv_image.rows, cv_image.cols,
                    CV_8UC1, cv_image.ptr(0, 0) );

  moving_burnin_detector_opencv::priv::metadata md;
  cv::Rect roi = cv::Rect( (w - nw) / 2, (h - nh) / 2, nw, nh ) + cv::Point( off_center_x, off_center_y );
  cv::Rect roi_buf = ( roi - cv::Point( line_width, line_width) ) + cv::Size( 2 * line_width, 2 * line_width );
  cv::Mat edges = cv::Mat::zeros( h, w, CV_8UC1 );
  cv::Mat edge_view = edges( roi_buf );
  cv::Mat input_view = input( roi_buf );

  std::vector< cv::Mat > bands;
  cv::split( input_view, bands );

  if ( cv_image.channels() == 3 )
  {
    for ( size_t i = 0; i < bands.size(); ++i )
    {
      cv::Mat tmp;
      cv::threshold(bands[i], tmp, 128, 255, cv::THRESH_BINARY);
      bitwise_or( edge_view, tmp, edge_view );
    }
  }
  else
  {
  cv::add( edge_view, bands[0], edge_view );
  }

  // detect burnin objects
  if ( 0.0 <= cross_threshold && cross_threshold <= 1.0)
  {
    std::cout << "Cross threshold: " << cross_threshold << std::endl;
    detect_cross( edges, md.center );
  }

  draw_metadata_mask( cv_image, md );

  vital::image overlay = ocv::image_container::ocv_to_vital(
    cv_image,
    ocv::image_container::BGR_COLOR);

  // logging
  if ( verbose )
  {
    // write out the values of aspect ratios and jitters that were actually
    // used in the final detections.  This information can be used to better
    // set the parameters.
    log_detection_stats( md );
  }

  return std::make_shared< ocv::image_container >( overlay );
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
moving_burnin_detector_opencv
::filter( kwiver::vital::image_container_sptr image )
{
  // Perform Basic Validation on image
  if( !image )
  {
    LOG_ERROR( logger(), "Invalid image" );
    return kwiver::vital::image_container_sptr();
  }

  // Filter and with responses cast to bytes
  auto const mask = d->filter( image );

  return mask;
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
