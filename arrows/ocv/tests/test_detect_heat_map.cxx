// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/ocv/algo/detect_heat_map.h>
#include <arrows/ocv/image_container.h>

#include <vital/exceptions/algorithm.h>

#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <vital/algo/algorithm.txx>

#include <gtest/gtest.h>

#include <opencv2/core/core.hpp>

using namespace kwiver::vital;
using namespace kwiver::arrows::ocv;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( detect_heat_map, create )
{
  plugin_manager::instance().load_all_plugins();
  EXPECT_NE(
    nullptr, create_algorithm< algo::image_object_detector >(
      "detect_heat_map" ) );
}

// ----------------------------------------------------------------------------
TEST ( detect_heat_map, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    detect_heat_map,
    "OCV implementation to create detections from heatmaps",

    PARAM_DEFAULT(
      threshold, double,
      "Threshold value applied to each pixel of the heat map to "
      "turn it into a binary mask. Any pixels with value "
      "strictly greater than this threshold will be turned on "
      "in the mask. Detection objects will be associated with "
      "connected-component regions of above-threshold pixels. "
      "The default threshold of -1 indicates that further "
      "processing will be done on the full-range heat map "
      "image. This mode of processing requires that ",
      -1.0 ),

    PARAM_DEFAULT(
      opening_radius, double,
      "Radius of the disk used to morphologically open the "
      "thresholded mask before connected components are found. "
      "The disk holds the offsets strictly closer than this "
      "radius to its center. 0 disables the opening.",
      0.0 ),

    PARAM_DEFAULT(
      closing_radius, double,
      "Radius of the disk used to morphologically close the "
      "thresholded mask, after any opening, before connected "
      "components are found. 0 disables the closing.",
      0.0 ),

    PARAM_DEFAULT(
      force_bbox_width, int,
      "Create bounding boxes of this fixed width.",
      -1 ),

    PARAM_DEFAULT(
      force_bbox_height, int,
      "Create bounding boxes of this fixed height.",
      -1 ),

    PARAM_DEFAULT(
      score_mode, std::string,
      "Mode in which a score is attributed to each detected "
      "object. A numerical value indicates that all detected "
      "objects will be assigned this fixed score. For "
      "connected-component detections, 'max' and 'mean' score "
      "each object by the maximum or mean heat-map value over "
      "its region, divided by the pixel type's maximum for "
      "integer heat maps.",
      "1" ),

    PARAM_DEFAULT(
      bbox_buffer, int,
      "If a bounding box of fixed height and width is specified,"
      "the default bbox_buffer of 0 indicates that the bounding"
      "boxes will tightly crop features in the heat map, and "
      "multiple, non-overlapping bounding boxes will be created "
      "to cover large, extended heat-map features. With a value "
      "greater than 0, generated bounding boxes will tend to "
      "have that number of pixels of buffer from the heat-map "
      "features. Also, setting bbox_buffer causes the generated "
      "bounding boxes to tend to overlap by this number of "
      "pixels when multiple boxes are required to cover and "
      "extended heat-map feature.", 0 ),

    PARAM_DEFAULT(
      min_area, int,
      "Minimum area of above-threshold pixels in a connected "
      "cluster allowed. Area is approximately equal to the "
      "number of pixels in the cluster.",
      1 ),

    PARAM_DEFAULT(
      max_area, int,
      "Maximum area of above-threshold pixels in a connected "
      "cluster allowed. Area is approximately equal to the "
      "number of pixels in the cluster.",
      10000000 ),

    PARAM_DEFAULT(
      min_fill_fraction, double,
      "Fraction of the bounding box filled with above threshold "
      "pixels.",
      0.25 ),

    PARAM_DEFAULT(
      class_name, std::string,
      "Detection class name.",
      "unspecified" ),

    PARAM_DEFAULT(
      max_boxes, int,
      "Maximum number of "
      "bounding boxes to generate. If exceeded, the top "
      "'max_boxes' ones will be returned", 1000000 ),

    PARAM_DEFAULT(
      pyr_red_levels, int,
      "Levels of image "
      "pyramid reduction (decimation) on the heat map before "
      "box selection. This improves speed at the expense of "
      "coarseness of bounding box placement. ", 0 )
  );
}

// ----------------------------------------------------------------------------
namespace {

image_container_sptr
make_heat_map( cv::Mat const& heat_map )
{
  return std::make_shared< kwiver::arrows::ocv::image_container >(
    heat_map, kwiver::arrows::ocv::image_container::OTHER_COLOR );
}

detected_object_set_sptr
detect(
  cv::Mat const& heat_map,
  std::map< std::string, std::string > const& settings )
{
  detect_heat_map detector;
  auto config = detector.get_configuration();
  config->set_value( "threshold", "50" );
  config->set_value( "min_fill_fraction", "0" );
  for( auto const& setting : settings )
  {
    config->set_value( setting.first, setting.second );
  }
  detector.set_configuration( config );
  return detector.detect( make_heat_map( heat_map ) );
}

} // namespace

// ----------------------------------------------------------------------------
TEST ( detect_heat_map, region_scores )
{
  cv::Mat heat_map = cv::Mat::zeros( 100, 100, CV_8U );
  heat_map( cv::Rect( 20, 20, 20, 20 ) ) = 100;
  heat_map.at< uchar >( 30, 30 ) = 200;

  auto const fixed = detect( heat_map, { { "score_mode", "0.5" } } );
  ASSERT_EQ( 1, fixed->size() );
  EXPECT_DOUBLE_EQ( 0.5, fixed->at( 0 )->confidence() );

  auto const max = detect( heat_map, { { "score_mode", "max" } } );
  ASSERT_EQ( 1, max->size() );
  EXPECT_DOUBLE_EQ( 200.0 / 255.0, max->at( 0 )->confidence() );

  auto const mean = detect( heat_map, { { "score_mode", "mean" } } );
  ASSERT_EQ( 1, mean->size() );
  EXPECT_NEAR(
    ( 399 * 100.0 + 200.0 ) / 400.0 / 255.0, mean->at( 0 )->confidence(),
    1e-9 );

  std::string class_name;
  double class_score;
  mean->at( 0 )->type()->get_most_likely( class_name, class_score );
  EXPECT_EQ( mean->at( 0 )->confidence(), class_score );
}

// ----------------------------------------------------------------------------
TEST ( detect_heat_map, float_region_score )
{
  cv::Mat heat_map = cv::Mat::zeros( 100, 100, CV_32F );
  heat_map( cv::Rect( 20, 20, 20, 20 ) ) = 0.75f;

  auto const max = detect(
    heat_map, { { "threshold", "0.5" }, { "score_mode", "max" } } );
  ASSERT_EQ( 1, max->size() );
  EXPECT_FLOAT_EQ( 0.75f, max->at( 0 )->confidence() );
}

// ----------------------------------------------------------------------------
TEST ( detect_heat_map, morphology )
{
  cv::Mat heat_map = cv::Mat::zeros( 100, 100, CV_8U );
  heat_map( cv::Rect( 20, 20, 20, 20 ) ) = 100;
  heat_map( cv::Rect( 42, 20, 20, 20 ) ) = 100;
  heat_map( cv::Rect( 70, 70, 2, 2 ) ) = 100;

  EXPECT_EQ( 3, detect( heat_map, {} )->size() );

  auto const closed = detect( heat_map, { { "closing_radius", "3" } } );
  EXPECT_EQ( 2, closed->size() );

  auto const opened = detect(
    heat_map, { { "opening_radius", "2" }, { "closing_radius", "3" } } );
  ASSERT_EQ( 1, opened->size() );
  auto const box = opened->at( 0 )->bounding_box();
  EXPECT_EQ( 20, box.min_x() );
  EXPECT_EQ( 62, box.max_x() );
}

// ----------------------------------------------------------------------------
TEST ( detect_heat_map, unknown_score_mode )
{
  detect_heat_map detector;
  auto config = detector.get_configuration();
  config->set_value( "threshold", "50" );
  config->set_value( "score_mode", "median" );
  EXPECT_THROW(
    detector.set_configuration( config ),
    algorithm_configuration_exception );
}
