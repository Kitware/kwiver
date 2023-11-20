// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "moving_burnin_detector_opencv.h"

#include <arrows/ocv/image_container.h>

#include <opencv/cxcore.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <assert.h>

//#include <vxl_config.h>
#include <typeinfo>
#define HAVE_COLOR_IMAGE

namespace cv
{
static bool operator<( const cv::Point& a, const cv::Point& b );
}
bool cv::operator<( const cv::Point& a, const cv::Point& b )
{
  return ( ( a.x <  b.x ) ||
         ( ( a.x == b.x ) && ( a.y <  b.y ) ) );
}


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
    std::vector< cv::Rect > brackets;
    cv::Rect rectangle;
  };

  /// Draw the detected burnin on the image
  void draw_metadata_mask( cv::Mat& img, const metadata& md );
  /// Write out the values of aspect ratios and jitters that were actually
  /// used in the final detections.
  void log_detection_stats( const metadata& md );

  // Crosshair
  /// Detect cross hairs in an image
  void detect_cross( const cv::Mat& edge_image, cv::Point& center );
  /// Draw cross hair template
  void draw_cross( cv::Mat& img, const cv::Point & center, cv::Scalar color, int width );

  // Brackets
  typedef std::pair< double, cv::Rect > scored_rect;
  static bool cmp_scored_rects( const scored_rect& a, const scored_rect& b );
  static bool close_rects( const cv::Rect& base, const cv::Rect& cmp, double min_ratio, double max_ratio );
  int count_hits( cv::Mat edge_image, std::set< cv::Point > points);
  void detect_bracket( const cv::Mat& edge_image, const cv::Rect& roi, std::vector< cv::Rect >& brackets );
  static void draw_line( std::set< cv::Point >& pts, const cv::Point& p1, const cv::Point& p2, int line_width );
  void draw_bracket( cv::Mat& img, const cv::Rect& rect, cv::Scalar clr, int width );
  void draw_bracket_tl( std::set< cv::Point >& pts, const cv::Rect& rect, int width );
  void draw_bracket_tr( std::set< cv::Point >& pts, const cv::Rect& rect, int width );
  void draw_bracket_bl( std::set< cv::Point >& pts, const cv::Rect& rect, int width );
  void draw_bracket_br( std::set< cv::Point >& pts, const cv::Rect& rect, int width );

  // Rectangle
  void detect_rectangle( const cv::Mat& edge_image, const std::vector< cv::Rect >& brackets, cv::Rect& rect );
  void draw_rectangle( cv::Mat& img, const cv::Rect& rect, cv::Scalar clr, int width );
  void draw_rectangle( std::set< cv::Point >& pts, const cv::Rect& rect, int width );

  kwiver::vital::image_container_sptr filter( kwiver::vital::image_container_sptr image );

  bool invalid_image;
  cv::Mat byte_mask;
  int w;
  int h;
  cv::Scalar cross_output_color;
  cv::Scalar bracket_output_color;
  cv::Scalar rectangle_output_color;
  std::vector<unsigned> target_widths;

  // config default values
  bool disabled{ false };

  bool highest_score_only{ false };
  double line_width{ 3 };
  double draw_line_width{ 3 };
  double roi_ratio{ 0.5 };
  double min_roi_ratio{ 0.1 };
  double roi_aspect{ 0 };
  int off_center_x{ 0 };
  int off_center_y{ 0 };

  double cross_output_color_R{ 255 }; // red
  double cross_output_color_G{ 0 };
  double cross_output_color_B{ 0 };
  double cross_threshold{ 0.2 };
  int cross_gap_x{ 6 };
  int cross_gap_y{ 6 };
  int cross_length_x{ 14 };
  int cross_length_y{ 14 };
  float cross_ends_ratio{ -1.0 };

  double bracket_threshold{ -1 };
  double bracket_output_color_R{ 0 };
  double bracket_output_color_G{ 255 }; // green
  double bracket_output_color_B{ 0 };
  int bracket_length_x{ 10 };
  int bracket_length_y{ 6 };
  int bracket_aspect_jitter{ 5 };

  double rectangle_threshold{ -1 };
  double rectangle_output_color_R{ 0 };
  double rectangle_output_color_G{ 0 };
  double rectangle_output_color_B{ 255 }; // blue

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
  config->set_value( "min_roi_ratio",
                     d->min_roi_ratio,
                     "Minimum proportion of the width of the frame size (centered) to look for burnin" );
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
  config->set_value("cross_output_color_R",
                     d->cross_output_color_R,
                     "Red value of the color the detected crosshair is drawn" );
  config->set_value("cross_output_color_G",
                     d->cross_output_color_G,
                     "Green value of the color the detected crosshair is drawn" );
  config->set_value("cross_output_color_B",
                     d->cross_output_color_B,
                     "Blue value of the color the detected crosshair is drawn" );
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

  // brackets
  config->set_value( "bracket_threshold",
                     d->bracket_threshold,
                    "Minimum coverage of the bracket with edge detection (negative to disable)" );
  config->set_value( "bracket_output_color_R",
                     d->bracket_output_color_R,
                     "Red value of the color the detected brackets are drawn" );
  config->set_value( "bracket_output_color_G",
                     d->bracket_output_color_G,
                     "Green value of the color the detected brackets are drawn" );
  config->set_value( "bracket_output_color_B",
                     d->bracket_output_color_B,
                     "Blue value of the color the detected brackets are drawn" );
  config->set_value( "bracket_length_x",
                     d->bracket_length_x,
                     "Horizontal length of bracket corners" );
  config->set_value( "bracket_length_y",
                    d->bracket_length_y,
                    "Vertical length of bracket corners" );
  config->set_value( "bracket_aspect_jitter",
                     d->bracket_aspect_jitter,
                     "Offset from 1:1 aspect with the frame to search for bracket corners (in pixels)" );

  //rectangle
  config->set_value( "rectangle_threshold",
                      d->rectangle_threshold,
                     "Minimum coverage of the rectangle with edge detection (negative to disable)" );
  config->set_value( "rectangle_output_color_R",
                      d->rectangle_output_color_R,
                     "Red value of the color the detected rectangle is drawn" );
  config->set_value( "rectangle_output_color_G",
                      d->rectangle_output_color_G,
                     "Green value of the color the detected rectangle is drawn" );
  config->set_value( "rectangle_output_color_B",
                      d->rectangle_output_color_B,
                     "Blue value of the color the detected rectangle is drawn" );

  // TODO: add text

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
  d->min_roi_ratio = config->get_value< double >("min_roi_ratio");
  d->roi_aspect = config->get_value< double >("roi_aspect");
  d->off_center_x = config->get_value< int >("off_center_x");
  d->off_center_y = config->get_value< int >("off_center_y");

  // crosshair
  d->cross_output_color_R = config->get_value< double >("cross_output_color_R");
  d->cross_output_color_G = config->get_value< double >("cross_output_color_G");
  d->cross_output_color_B = config->get_value< double >("cross_output_color_B");
  d->cross_threshold = config->get_value< double >("cross_threshold");
  d->cross_gap_x = config->get_value< int >("cross_gap_x");
  d->cross_gap_y = config->get_value< int >("cross_gap_y");
  d->cross_length_x = config->get_value< int >("cross_length_x");
  d->cross_length_y = config->get_value< int >("cross_length_y");
  d->cross_ends_ratio = config->get_value< float >("cross_ends_ratio");

  // brackets
  d->bracket_threshold = config->get_value< double >("bracket_threshold");
  d->bracket_output_color_R = config->get_value< double >("bracket_output_color_R");
  d->bracket_output_color_G = config->get_value< double >("bracket_output_color_G");
  d->bracket_output_color_B = config->get_value< double >("bracket_output_color_B");
  d->bracket_length_x = config->get_value< int >("bracket_length_x");
  d->bracket_length_y = config->get_value< int >("bracket_length_y");
  d->bracket_aspect_jitter = config->get_value< int >("bracket_aspect_jitter");

  // rectangle
  d->rectangle_threshold = config->get_value< double >("rectangle_threshold");
  d->rectangle_output_color_R = config->get_value< double >("rectangle_output_color_R");
  d->rectangle_output_color_G = config->get_value< double >("rectangle_output_color_G");
  d->rectangle_output_color_B = config->get_value< double >("rectangle_output_color_B");

  // TODO: add text

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
  bracket_length_x = static_cast<int>( bracket_length_x * scale_factor_x + 0.5 );
  bracket_length_y = static_cast<int>( bracket_length_y * scale_factor_y + 0.5 );

  target_resolution_x = img.cols;
  target_resolution_y = img.rows;
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_metadata_mask( cv::Mat& img, const metadata& md )
{
  // crosshair
  if ( md.center.x != 0 && md.center.y != 0 )
  {
    draw_cross( img, md.center, cross_output_color, draw_line_width );
  }

  // rectangle
  if ( md.rectangle.x != 0 && md.rectangle.y != 0 )
  {
    draw_rectangle( img, md.rectangle, rectangle_output_color, draw_line_width );
  }

  // brackets
  std::for_each(
    md.brackets.begin(),
    md.brackets.end(),
    std::bind(
      &moving_burnin_detector_opencv::priv::draw_bracket,
      this,
      img,
      std::placeholders::_1,
      bracket_output_color,
      draw_line_width ) );

  // TODO: add text
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
  for (unsigned i=0; i<md.brackets.size(); ++i)
  {
    double aspect = double(md.brackets[i].height) / md.brackets[i].width;
    cv::Point bcntr = (md.brackets[i].tl() + md.brackets[i].br());
    double aspect_jitter = md.brackets[i].height - roi_aspect * md.brackets[i].width - 0.5;
    bcntr.x /= 2;
    bcntr.y /= 2;
    cv::Point pos_jitter = cntr - bcntr;
    LOG_INFO( p->logger(), "Bracket aspect: " << aspect );
    LOG_INFO( p->logger(), "Bracket aspect jitter: " << aspect_jitter );
    LOG_INFO( p->logger(), "Bracket position jitter: [" << pos_jitter.x <<", "<< pos_jitter.y<< "]" );
  }
}

// ----------------------------------------------------------------------------------
bool
moving_burnin_detector_opencv::priv
::cmp_scored_rects( const scored_rect& a, const scored_rect& b )
{
  return (a.first < b.first);
}

// ----------------------------------------------------------------------------------
bool
moving_burnin_detector_opencv::priv
::close_rects( const cv::Rect& base, const cv::Rect& cmp, double min_ratio, double max_ratio )
{
  const double ba = base.area();
  const double ca = cmp.area();

  return ((ba * min_ratio) < ca) && (ca < (ba * max_ratio));
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

  draw_cross( cross, cntr, cv::Scalar(255, 255, 255, 255), line_width );
  const int cross_count = cv::countNonZero( cross );
  draw_cross( cross, cntr, cv::Scalar(0, 0, 0, 0), line_width );

  for ( int i = -off_center_jitter; i <= off_center_jitter; ++i )
  {
    for ( int j = -off_center_jitter; j <= off_center_jitter; ++j )
    {
      const cv::Point ct = cntr + cv::Point( j, i );

      const cv::Rect bb( ct - cv::Point( cross_length_x + cross_gap_x,
                                         cross_length_y + cross_gap_y ),
                         cv::Size( 2 * ( cross_length_x + cross_gap_x ),
                                   2 * ( cross_length_y + cross_gap_y ) ) );
     draw_cross( cross, ct, cv::Scalar(255, 255, 255, 255), line_width );
     cv::Mat edgev = edge_image(bb);
     cv::Mat crossv = cross(bb);
     cv::bitwise_and( edgev, crossv, buffer );
     draw_cross( cross, ct, cv::Scalar(0, 0, 0, 0), line_width );

     const int edge_count = cv::countNonZero( buffer );

     const double s = double(edge_count) / double(cross_count);

     if ( s > score )
     {
       //std::cout << "score: " << s << std::endl;
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
int
moving_burnin_detector_opencv::priv
::count_hits( cv::Mat edge_image, std::set< cv::Point > points)
{
  int hits = 0;
  for ( std::set< cv::Point >::iterator i = points.begin(); i != points.end(); i++ )
  {
    cv::Point p = *i;
    int val = (int) edge_image.at< unsigned char >( p );
    if ( val != 0 )
    {
      hits++;
    }
  }
  return hits;
}
// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::detect_bracket( const cv::Mat& edge_image, const cv::Rect& roi, std::vector< cv::Rect >& brackets )
{
  const double aspect = roi_aspect ? roi_aspect : ( double(h) / double(w) );

  std::vector< std::pair< double, cv::Rect > > scored_rects;
  std::map< cv::Point, std::pair< int, int > > count_tl;
  std::map< cv::Point, std::pair< int, int > > count_tr;
  std::map< cv::Point, std::pair< int, int > > count_bl;
  std::map< cv::Point, std::pair< int, int > > count_br;

  const int min_bracket_width = min_roi_ratio * w;
  for ( int nw = roi.width; nw >= min_bracket_width; )
  {
    double score = 0;
    cv::Rect rect;

    double best = 0;

    for ( int a = -bracket_aspect_jitter; a <= bracket_aspect_jitter; ++a )
    {
      const int nh = nw * aspect + a + 0.5;
      for ( int i = -off_center_jitter; i <= off_center_jitter; ++i )
      {
        for ( int j = -off_center_jitter; j <= off_center_jitter; ++j )
        {
          cv::Rect rt = cv::Rect( (w - nw + 1) / 2 + j, (h - nh + 1) / 2 + i, nw, nh ) + cv::Point( off_center_x, off_center_y );

          int edge_count = 0;
          int bracket_count = 0;

          const cv::Point tl = rt.tl();
          const cv::Point tr = tl + cv::Point( rt.width, 0 );
          const cv::Point br = rt.br();
          const cv::Point bl = tl + cv::Point( 0, rt.height );

          if ( count_tl.find( tl ) == count_tl.end() )
          {
            std::set< cv::Point > points;

            draw_bracket_tl( points, rt, line_width );

            const int hits = count_hits(edge_image, points);
            //const int hits = std::count_if( points.begin(), points.end(), std::bind( is_non_zero, edge_image, std::placeholders::_1 ) );
            count_tl[ tl ] = std::make_pair( hits, points.size() );

            edge_count += hits;
            bracket_count += points.size();
          }
          else
          {
            const std::pair< int, int >& pr = count_tl[ tl ];

            edge_count += pr.first;
            bracket_count += pr.second;
          }
          if ( count_tr.find( tr ) == count_tr.end() )
          {
            std::set< cv::Point > points;

            draw_bracket_tr( points, rt, line_width );

            const int hits = count_hits( edge_image, points );
            //const int hits = std::count_if( points.begin(), points.end(), std::bind( is_non_zero, edge_image, std::placeholders::_1 ) );
            count_tr[ tr ] = std::make_pair( hits, points.size() );

            edge_count += hits;
            bracket_count += points.size();
          }
          else
          {
            const std::pair< int, int >& pr = count_tr[ tr ];

            edge_count += pr.first;
            bracket_count += pr.second;
          }
          if ( count_bl.find( bl ) == count_bl.end() )
          {
            std::set< cv::Point > points;

            draw_bracket_bl( points, rt, line_width );

            const int hits = count_hits(edge_image, points);
            //const int hits = std::count_if( points.begin(), points.end(), std::bind( is_non_zero, edge_image, std::placeholders::_1 ) );
            count_bl[ bl ] = std::make_pair( hits, points.size() );

            edge_count += hits;
            bracket_count += points.size();
          }
          else
          {
            const std::pair< int, int >& pr = count_bl[ bl ];

            edge_count += pr.first;
            bracket_count += pr.second;
          }
          if ( count_br.find( br ) == count_br.end() )
          {
            std::set< cv::Point > points;

            draw_bracket_br( points, rt, line_width );

            const int hits = count_hits(edge_image, points);
            //const int hits = std::count_if( points.begin(), points.end(), std::bind( is_non_zero, edge_image, std::placeholders::_1 ) );
            count_br[ br ] = std::make_pair( hits, points.size() );

            edge_count += hits;
            bracket_count += points.size();
          }
          else
          {
            const std::pair< int, int >& pr = count_br[ br ];

            edge_count += pr.first;
            bracket_count += pr.second;
          }

          const double s = double(edge_count) / double(bracket_count);
          //std::cout << "score: " << s << std::endl;
          if ( s > best )
          {
            best = s;
          }

          if ( s > score )
          {
            rect = rt;
            score = s;
          }
        }
      }
    }

    if ( best > bracket_threshold )
    {
      nw -= 2;
    }
    else
    {
      nw -= 2 * line_width;
    }

    if ( score > bracket_threshold )
    {
      scored_rects.push_back( std::make_pair( score, rect ) );
    }
  }

  std::sort( scored_rects.begin(), scored_rects.end(), cmp_scored_rects );

  if( highest_score_only && !scored_rects.empty() )
  {
    std::pair< double, cv::Rect > top_response = scored_rects.back();
    scored_rects.resize( 1 );
    scored_rects[0] = top_response;
  }

  std::vector< std::pair< double, cv::Rect > >::const_reverse_iterator srit;
  for ( srit = scored_rects.rbegin(); srit != scored_rects.rend(); ++srit )
  {
    std::vector<cv::Rect>::const_iterator i = std::find_if( brackets.begin(), brackets.end(), std::bind( close_rects, std::placeholders::_1, srit->second, 0.8, 1.25 ) );

    if ( i == brackets.end() )
    {
      brackets.push_back( srit->second );
    }
  }
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::detect_rectangle( const cv::Mat& edge_image, const std::vector< cv::Rect >& brackets, cv::Rect& rect )
{
  double score = 0;
  cv::Mat rectangle = cv::Mat::zeros( h, w, CV_8UC1 );
  cv::Mat buffer;

  for ( size_t i = 0; i < brackets.size(); ++i )
  {
    std::set< cv::Point > points;

    draw_rectangle( points, brackets[i], line_width );

    const int rectangle_count = points.size();
    const int edge_count = count_hits(edge_image, points);
    const double s = double(edge_count) / double(rectangle_count);

    if ( s > score )
    {
      rect = brackets[i];
      score = s;
    }
  }

  if ( score < rectangle_threshold )
  {
    rect = cv::Rect( 0, 0, 0, 0 );
  }
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_cross( cv::Mat& img, const cv::Point& center, cv::Scalar color, int width )
{
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
void
moving_burnin_detector_opencv::priv
::draw_line( std::set< cv::Point >& pts, const cv::Point& p1, const cv::Point& p2, int line_width )
{
  if ( !line_width )
  {
    return;
  }

  // Algorithm for non-straight line not implemented
  assert( p1.x == p2.x || p1.y == p2.y );

  if ( p1.x == p2.x )
  {
    const int dy = ( p1.y < p2.y ) ? 1 : -1;

    for ( int j = p1.y; j != p2.y + dy; j += dy )
    {
      for ( int i = ( p1.x - ( line_width - ( dy > 0 ) ) / 2 ); i <= ( p1.x + ( line_width - ( dy < 0 ) ) / 2 ); ++i )
      {
        pts.insert( cv::Point( i, j ) );
      }
    }
  }
  else if ( p1.y == p2.y )
  {
    const int dx = ( p1.x < p2.x ) ? 1 : -1;

    for ( int i = p1.x; i != p2.x + dx; i += dx )
    {
      for ( int j = ( p1.y - ( line_width - ( dx > 0 ) ) / 2 ); j <= ( p1.y + ( line_width - ( dx < 0 ) ) / 2 ); ++j )
      {
        pts.insert( cv::Point( i, j ) );
      }
    }
  }
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_bracket( cv::Mat& img, const cv::Rect& rect, cv::Scalar clr, int width )
{
  std::set< cv::Point > points;

  draw_bracket_tl( points, rect, width );
  draw_bracket_tr( points, rect, width );
  draw_bracket_bl( points, rect, width );
  draw_bracket_br( points, rect, width );

  const cv::Point dx = cv::Point( bracket_length_x, 0 );
  const cv::Point dy = cv::Point( 0, bracket_length_y );

  // Top left
  cv::line( img, rect.tl(), rect.tl() + dx, clr, width);
  cv::line( img, rect.tl(), rect.tl() + dy, clr, width);

  // Top Right
  cv::Point tr = rect.tl() + cv::Point(rect.width, 0);
  cv::line( img, tr, tr - dx, clr, width);
  cv::line( img, tr, tr + dy, clr, width);

  // Bottom left
  cv::Point bl = rect.tl() + cv::Point(0, rect.height);
  cv::line( img, bl, bl + dx, clr, width);
  cv::line( img, bl, bl - dy, clr, width);

  // Bottom right
  cv::line( img, rect.br(), rect.br() - dx, clr, width);
  cv::line( img, rect.br(), rect.br() - dy, clr, width);

  /*
  std::set< cv::Point >::const_iterator pt = points.begin();
  std::set< cv::Point >::const_iterator end = points.end();

  for ( ; pt != end; ++pt )
  {
    cv::Point p = *pt;
    cv::Vec3b& color = img.at< cv::Vec3b >( p );
    color[0] = clr[0];
    color[1] = clr[1];
    color[2] = clr[2];
  }
  */
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_bracket_tl( std::set< cv::Point >& pts, const cv::Rect& rect, int width )
{
  const cv::Point dx = cv::Point( bracket_length_x, 0 );
  const cv::Point dy = cv::Point( 0, bracket_length_y );

  const cv::Point tl = rect.tl();
  draw_line( pts, tl + dy, tl - cv::Point( 0, ( width - 1 ) / 2 ), width );
  draw_line( pts, tl + cv::Point( width / 2, 0 ), tl + dx, width );
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_bracket_tr( std::set< cv::Point >& pts, const cv::Rect& rect, int width )
{
  const cv::Point dx = cv::Point( bracket_length_x, 0 );
  const cv::Point dy = cv::Point( 0, bracket_length_y );

  const cv::Point tr = rect.tl() + cv::Point( rect.width, 0 );
  draw_line( pts, tr - dx, tr + cv::Point( width / 2, 0 ), width );
  draw_line( pts, tr + cv::Point( 0, width / 2 + 1 ), tr + dy, width );
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_bracket_bl( std::set< cv::Point >& pts, const cv::Rect& rect, int width )
{
  const cv::Point dx = cv::Point( bracket_length_x, 0 );
  const cv::Point dy = cv::Point( 0, bracket_length_y );

  const cv::Point bl = rect.tl() + cv::Point( 0, rect.height );
  draw_line( pts, bl + dx, bl - cv::Point( width / 2, 0 ), width );
  draw_line( pts, bl - cv::Point( 0, width / 2 + 1), bl - dy, width );
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_bracket_br( std::set< cv::Point >& pts, const cv::Rect& rect, int width )
{
  const cv::Point dx = cv::Point( bracket_length_x, 0 );
  const cv::Point dy = cv::Point( 0, bracket_length_y );

  const cv::Point br = rect.br();
  draw_line( pts, br - dy, br + cv::Point( 0, ( width - 1 ) / 2 ), width );
  draw_line( pts, br - cv::Point( width / 2, 0 ), br - dx, width );
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_rectangle( cv::Mat& img, const cv::Rect& rect, cv::Scalar clr, int width )
{
  cv::rectangle( img, rect, clr, width);

  /*std::set< cv::Point > points;

  draw_rectangle( points, rect, width );

  std::set< cv::Point >::const_iterator pt = points.begin();
  std::set< cv::Point >::const_iterator end = points.end();

  for ( ; pt != end; ++pt )
  {
    cv::Vec3b& color = img.at< cv::Vec3b >( *pt );
    color[0] = clr[0];
    color[1] = clr[1];
    color[2] = clr[2];
  }*/
}

// ----------------------------------------------------------------------------------
void
moving_burnin_detector_opencv::priv
::draw_rectangle( std::set< cv::Point >& pts, const cv::Rect& rect, int width )
{
  const cv::Point dx = cv::Point( rect.width, 0 );
  const cv::Point dy = cv::Point( 0, rect.height );

  draw_line( pts, rect.tl() + cv::Point( width / 2, 0 ), rect.tl() + dx, width );
  draw_line( pts, rect.tl() + dx + cv::Point( 0, width / 2 ), rect.br(), width );
  draw_line( pts, rect.br() - cv::Point( width / 2, 0 ), rect.tl() + dy, width );
  draw_line( pts, rect.tl() + dy - cv::Point( 0, width / 2 ), rect.tl(), width );
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
    ocv::image_container::BGR_COLOR );

  cross_output_color = cv::Scalar(cross_output_color_B, cross_output_color_G, cross_output_color_R);
  bracket_output_color = cv::Scalar(bracket_output_color_B, bracket_output_color_G, bracket_output_color_R);
  rectangle_output_color = cv::Scalar(rectangle_output_color_B, rectangle_output_color_G, rectangle_output_color_R);

  //bool invalid_image;
  invalid_image = set_input_image( cv_image );
  if( invalid_image ){
    LOG_ERROR( p->logger(), "Invalid image" );
    return kwiver::vital::image_container_sptr();
  }

  scale_params_for_image( cv_image );

  if( byte_mask.cols != cv_image.cols || byte_mask.rows != cv_image.rows )
  {
    byte_mask = cv::Mat(cv_image.rows, cv_image.cols, CV_8UC1);
  }
  byte_mask.setTo(cv::Scalar::all(0));

  w = cv_image.cols;
  h = cv_image.rows;
  //roi_aspect = roi_aspect ? roi_aspect : ( double(h) / double(w) );
  const int nw = w * roi_ratio;
  const int nh = roi_aspect ? ( nw * roi_aspect ) : ( h * roi_ratio );

  cv::Mat input( h, w,
                 ( cv_image.channels() == 1 ) ? CV_8UC1 : CV_8UC3,
                 cv_image.ptr(0, 0) );
  cv::Mat end_mask( cv_image.rows, cv_image.cols,
                    CV_8UC1, byte_mask.ptr(0, 0) );

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
  if ( 0.0 <= bracket_threshold && bracket_threshold <= 1.0 )
  {
    std::cout << "Bracket threshold: " << bracket_threshold << std::endl;
    detect_bracket( edges, roi, md.brackets );
    if ( 0.0 <= rectangle_threshold && rectangle_threshold <= 1.0 )
    {
      std::cout << "Rectangle threshold: " << rectangle_threshold << std::endl;
      detect_rectangle( edges, md.brackets, md.rectangle);
    }
  }
  // TODO: add text detection

  cv::Mat final_mask =  cv::Mat::zeros( cv_image.rows, cv_image.cols, CV_8UC3 );
  //draw_metadata_mask( final_mask, md );
  //cv::imwrite( "final_mask.jpg", final_mask );
  draw_metadata_mask( cv_image, md );

  if( !md.brackets.empty() )
  {
    this->target_widths.clear();

    for( unsigned i = 0; i < md.brackets.size(); ++i )
    {
      this->target_widths.push_back( md.brackets[i].width );
    }
  }

  cv::cvtColor( cv_image, cv_image, cv::COLOR_BGR2RGB );
  vital::image overlay = ocv::image_container::ocv_to_vital(
    cv_image,
    ocv::image_container::RGB_COLOR);

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
