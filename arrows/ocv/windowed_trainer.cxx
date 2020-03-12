/*ckwg +29
 * Copyright 2019-2020 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "windowed_trainer.h"
#include "windowed_detector_resize.h"

#include <vital/util/cpu_timer.h>
#include <vital/algo/image_io.h>

#include <arrows/ocv/image_container.h>

#include <kwiversys/SystemTools.hxx>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

namespace kwiver {
namespace arrows {
namespace ocv {

#ifdef WIN32
  const std::string div = "\\";
#else
  const std::string div = "/";
#endif

// =============================================================================
class windowed_trainer::priv
{
public:
  priv()
    : m_train_directory( "deep_training" )
    , m_chip_subdirectory( "cached_chips" )
    , m_chip_format( "png" )
    , m_skip_format( false )
    , m_mode( "disabled" )
    , m_scale( 1.0 )
    , m_chip_width( 1000 )
    , m_chip_height( 1000 )
    , m_chip_step_width( 500 )
    , m_chip_step_height( 500 )
    , m_chip_adaptive_thresh( 2000000 )
    , m_chip_random_factor( -1.0 )
    , m_original_to_chip_size( true )
    , m_black_pad( false )
    , m_always_write_image( false )
    , m_ensure_standard( false )
    , m_overlap_required( 0.05 )
    , m_chips_w_gt_only( false )
    , m_max_neg_ratio( 0.0 )
    , m_random_validation( 0.0 )
    , m_ignore_category( "false_alarm" )
    , m_min_train_box_length( 5 )
    , m_synthetic_labels( true )
  {}

  ~priv()
  {}

  // Items from the config
  std::string m_train_directory;
  std::string m_chip_subdirectory;
  std::string m_chip_format;
  bool m_skip_format;
  std::string m_mode;
  double m_scale;
  int m_chip_width;
  int m_chip_height;
  int m_chip_step_width;
  int m_chip_step_height;
  int m_chip_adaptive_thresh;
  double m_chip_random_factor;
  bool m_original_to_chip_size;
  bool m_black_pad;
  bool m_always_write_image;
  bool m_ensure_standard;
  double m_overlap_required;
  bool m_chips_w_gt_only;
  double m_max_neg_ratio;
  double m_random_validation;
  std::string m_ignore_category;
  int m_min_train_box_length;

  // Helper functions
  void format_images_from_disk(
    std::vector< std::string > image_names,
    std::vector< vital::detected_object_set_sptr > groundtruth,
    std::vector< std::string >& formatted_names,
    std::vector< vital::detected_object_set_sptr >& formatted_truth );

  void format_image_from_memory(
    const cv::Mat& image,
    vital::detected_object_set_sptr groundtruth,
    const std::string format_method,
    std::vector< std::string >& formatted_names,
    std::vector< vital::detected_object_set_sptr >& formatted_truth );

  bool filter_detections_in_roi(
    vital::detected_object_set_sptr all_detections,
    vital::bounding_box_d region,
    vital::detected_object_set_sptr& filt_detections );

  std::string generate_filename( const int len = 10 );

  void write_chip_to_disk( const std::string& filename, const cv::Mat& image );

  bool m_synthetic_labels;
  vital::category_hierarchy_sptr m_labels;
  std::map< std::string, int > m_category_map;
  vital::algo::image_io_sptr m_image_io;
  vital::algo::train_detector_sptr m_trainer;
  vital::logger_handle_t m_logger;
};


// =============================================================================
windowed_trainer
::windowed_trainer()
  : d( new priv() )
{
  attach_logger( "arrows.ocv.windowed_trainer" );

  d->m_logger = logger();
}

windowed_trainer
::~windowed_trainer()
{
}


// -----------------------------------------------------------------------------
vital::config_block_sptr
windowed_trainer
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value( "train_directory", d->m_train_directory,
    "Directory for all files used in training." );
  config->set_value( "chip_format", d->m_chip_format,
    "Image format for output chips." );
  config->set_value( "skip_format", d->m_skip_format,
    "Skip file formatting, assume that the train_directory is pre-populated "
    "with all files required for model training." );
  config->set_value( "mode", d->m_mode,
    "Pre-processing resize option, can be: disabled, maintain_ar, scale, "
    "chip, or chip_and_original." );
  config->set_value( "scale", d->m_scale,
    "Image scaling factor used when mode is scale or chip." );
  config->set_value( "chip_height", d->m_chip_height,
    "When in chip mode, the chip height." );
  config->set_value( "chip_width", d->m_chip_width,
    "When in chip mode, the chip width." );
  config->set_value( "chip_step_height", d->m_chip_step_height,
    "When in chip mode, the chip step size between chips." );
  config->set_value( "chip_step_width", d->m_chip_step_width,
    "When in chip mode, the chip step size between chips." );
  config->set_value( "chip_adaptive_thresh", d->m_chip_adaptive_thresh,
    "If using adaptive selection, total pixel count at which we start to chip." );
  config->set_value( "chip_random_factor", d->m_chip_random_factor,
    "A percentage [0.0, 1.0] of chips to randomly use in training" );
  config->set_value( "original_to_chip_size", d->m_original_to_chip_size,
    "Optionally enforce the input image is not larger than the chip size" );
  config->set_value( "black_pad", d->m_black_pad,
    "Black pad the edges of resized chips to ensure consistent dimensions" );
  config->set_value( "always_write_image", d->m_always_write_image,
    "Always re-write images to training directory even if they already exist "
    "elsewhere on disk." );
  config->set_value( "ensure_standard", d->m_ensure_standard,
    "If images are not one of 3 common formats (jpg, jpeg, png) or 3 channel "
    "write them to the training directory even if they are elsewhere already" );
  config->set_value( "overlap_required", d->m_overlap_required,
    "Percentage of which a target must appear on a chip for it to be included "
    "as a training sample for said chip." );
  config->set_value( "chips_w_gt_only", d->m_chips_w_gt_only,
    "Only chips with valid groundtruth objects on them will be included in "
    "training." );
  config->set_value( "max_neg_ratio", d->m_max_neg_ratio,
    "Do not use more than this many more frames without groundtruth in "
    "training than there are frames with truth." );
  config->set_value( "random_validation", d->m_random_validation,
    "Randomly add this percentage of training frames to validation." );
  config->set_value( "ignore_category", d->m_ignore_category,
    "Ignore this category in training, but still include chips around it." );
  config->set_value( "min_train_box_length", d->m_min_train_box_length,
    "If a box resizes to smaller than this during training, the input frame " 
    "will not be used in training." );

  vital::algo::image_io::get_nested_algo_configuration( "image_reader",
    config, d->m_image_io );
  vital::algo::train_detector::get_nested_algo_configuration( "trainer",
    config, d->m_trainer );

  return config;
}


// -----------------------------------------------------------------------------
void
windowed_trainer
::set_configuration( vital::config_block_sptr config_in )
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();

  config->merge_config( config_in );

  this->d->m_train_directory = config->get_value< std::string >( "train_directory" );
  this->d->m_chip_format = config->get_value< std::string >( "chip_format" );
  this->d->m_skip_format = config->get_value< bool >( "skip_format" );
  this->d->m_mode = config->get_value< std::string >( "mode" );
  this->d->m_scale = config->get_value< double >( "scale" );
  this->d->m_chip_width = config->get_value< int >( "chip_width" );
  this->d->m_chip_height = config->get_value< int >( "chip_height" );
  this->d->m_chip_step_width = config->get_value< int >( "chip_step_width" );
  this->d->m_chip_step_height = config->get_value< int >( "chip_step_height" );
  this->d->m_chip_adaptive_thresh = config->get_value< int >( "chip_adaptive_thresh" );
  this->d->m_chip_random_factor = config->get_value< double >( "chip_random_factor" );
  this->d->m_original_to_chip_size = config->get_value< bool >( "original_to_chip_size" );
  this->d->m_black_pad = config->get_value< bool >( "black_pad" );
  this->d->m_always_write_image = config->get_value< bool >( "always_write_image" );
  this->d->m_ensure_standard = config->get_value< bool >( "ensure_standard" );
  this->d->m_overlap_required = config->get_value< double >( "overlap_required" );
  this->d->m_chips_w_gt_only = config->get_value< bool >( "chips_w_gt_only" );
  this->d->m_max_neg_ratio = config->get_value< double >( "max_neg_ratio" );
  this->d->m_random_validation = config->get_value< double >( "random_validation" );
  this->d->m_ignore_category = config->get_value< std::string >( "ignore_category" );
  this->d->m_min_train_box_length = config->get_value< int >( "min_train_box_length" );

  if( !d->m_skip_format )
  {
    // Delete and reset folder contents
    if( kwiversys::SystemTools::FileExists( d->m_train_directory ) &&
        kwiversys::SystemTools::FileIsDirectory( d->m_train_directory ) )
    {
      kwiversys::SystemTools::RemoveADirectory( d->m_train_directory );

      if( kwiversys::SystemTools::FileExists( d->m_train_directory ) )
      {
        LOG_ERROR( d->m_logger, "Unable to delete pre-existing training dir" );
        return;
      }
    }

    kwiversys::SystemTools::MakeDirectory( d->m_train_directory );

    if( !d->m_chip_subdirectory.empty() )
    {
      std::string folder = d->m_train_directory + div + d->m_chip_subdirectory;
      kwiversys::SystemTools::MakeDirectory( folder );
    }
  }

  vital::algo::image_io_sptr io;
  vital::algo::image_io::set_nested_algo_configuration( "image_reader", config, io );
  d->m_image_io = io;

  vital::algo::train_detector_sptr trainer;
  vital::algo::train_detector::set_nested_algo_configuration( "trainer", config, trainer );
  d->m_trainer = trainer;
}


// -----------------------------------------------------------------------------
bool
windowed_trainer
::check_configuration( vital::config_block_sptr config ) const
{
  return vital::algo::image_io::check_nested_algo_configuration(
     "image_reader", config )
   && vital::algo::train_detector::check_nested_algo_configuration(
     "trainer", config );
}


// -----------------------------------------------------------------------------
void
windowed_trainer
::add_data_from_disk(
  vital::category_hierarchy_sptr object_labels,
  std::vector< std::string > train_image_names,
  std::vector< vital::detected_object_set_sptr > train_groundtruth,
  std::vector< std::string > test_image_names,
  std::vector< vital::detected_object_set_sptr > test_groundtruth)
{
  if( object_labels )
  {
    d->m_labels = object_labels;
    d->m_synthetic_labels = false;
  }

  std::vector< std::string > filtered_train_names;
  std::vector< vital::detected_object_set_sptr > filtered_train_truth;
  std::vector< std::string > filtered_test_names;
  std::vector< vital::detected_object_set_sptr > filtered_test_truth;

  if( !d->m_skip_format )
  {
    d->format_images_from_disk(
      train_image_names, train_groundtruth,
      filtered_train_names, filtered_train_truth );

    d->format_images_from_disk(
      test_image_names, test_groundtruth,
      filtered_test_names, filtered_test_truth );
  }

  if( d->m_synthetic_labels )
  {
    vital::category_hierarchy_sptr all_labels =
      std::make_shared< vital::category_hierarchy >();

    for( auto p = d->m_category_map.begin(); p != d->m_category_map.end(); p++ )
    {
      all_labels->add_class( p->first );
    }

    d->m_trainer->add_data_from_disk(
      all_labels,
      filtered_train_names, filtered_train_truth,
      filtered_test_names, filtered_test_truth );
  }
  else
  {
    d->m_trainer->add_data_from_disk(
      object_labels,
      filtered_train_names, filtered_train_truth,
      filtered_test_names, filtered_test_truth );
  }
}

void
windowed_trainer
::add_data_from_memory(
  vital::category_hierarchy_sptr object_labels,
  std::vector< vital::image_container_sptr > train_images,
  std::vector< vital::detected_object_set_sptr > train_groundtruth,
  std::vector< vital::image_container_sptr > test_images,
  std::vector< vital::detected_object_set_sptr > test_groundtruth)
{
  if( object_labels )
  {
    d->m_labels = object_labels;
    d->m_synthetic_labels = false;
  }

  std::vector< std::string > filtered_train_names;
  std::vector< vital::detected_object_set_sptr > filtered_train_truth;
  std::vector< std::string > filtered_test_names;
  std::vector< vital::detected_object_set_sptr > filtered_test_truth;

  if( !d->m_skip_format )
  {
    for( unsigned i = 0; i < train_images.size(); ++i )
    {
      cv::Mat image = arrows::ocv::image_container::vital_to_ocv(
        train_images[i]->get_image(), arrows::ocv::image_container::RGB_COLOR );

      if( d->m_random_validation > 0.0 &&
          static_cast< double >( rand() ) / RAND_MAX <= d->m_random_validation )
      {
        d->format_image_from_memory(
          image, train_groundtruth[i], d->m_mode,
          filtered_test_names, filtered_test_truth );
      }
      else
      {
        d->format_image_from_memory(
          image, train_groundtruth[i], d->m_mode,
          filtered_train_names, filtered_train_truth );
      }
    }
    for( unsigned i = 0; i < test_images.size(); ++i )
    {
      cv::Mat image = arrows::ocv::image_container::vital_to_ocv(
        test_images[i]->get_image(), arrows::ocv::image_container::RGB_COLOR );

      d->format_image_from_memory(
        image, test_groundtruth[i], d->m_mode,
        filtered_test_names, filtered_test_truth );
    }
  }

  d->m_trainer->add_data_from_disk(
    object_labels,
    filtered_train_names, filtered_train_truth,
    filtered_test_names, filtered_test_truth );
}

void
windowed_trainer
::update_model()
{
  d->m_trainer->update_model();
}

// -----------------------------------------------------------------------------
void
windowed_trainer::priv
::format_images_from_disk(
  std::vector< std::string > image_names,
  std::vector< vital::detected_object_set_sptr > groundtruth,
  std::vector< std::string >& formatted_names,
  std::vector< vital::detected_object_set_sptr >& formatted_truth )
{
  double negative_ds_factor = -1.0;

  if( m_max_neg_ratio > 0.0 && groundtruth.size() > 10 )
  {
    unsigned gt = 0, no_gt = 0;

    for( unsigned i = 0; i < groundtruth.size(); ++i )
    {
      if( groundtruth[i] && !groundtruth[i]->empty() )
      {
        gt++;
      }
      else
      {
        no_gt++;
      }
    }

    if( no_gt > 0 && gt > 0 )
    {
      double current_ratio = static_cast< double >( no_gt ) / gt;

      if( current_ratio > m_max_neg_ratio )
      {
        negative_ds_factor = m_max_neg_ratio / current_ratio;
      }
    }
  }

  for( unsigned fid = 0; fid < image_names.size(); ++fid )
  {
    if( negative_ds_factor > 0.0 &&
        ( !groundtruth[fid] || groundtruth[fid]->empty() ) &&
        static_cast< double >( rand() ) / RAND_MAX > negative_ds_factor )
    {
      continue;
    }

    const std::string image_fn = image_names[fid];

    if( m_mode == "disabled" && !m_always_write_image && !m_ensure_standard )
    {
      formatted_names.push_back( image_fn );
      formatted_truth.push_back( groundtruth[fid] );
      continue;
    }

    // Scale and break up image according to settings
    vital::image_container_sptr vital_image;
    cv::Mat original_image;
    std::string format_mode = m_mode;
    std::string ext = image_fn.substr( image_fn.find_last_of( "." ) + 1 );

    try
    {
      vital_image = m_image_io->load( image_fn );

      original_image = arrows::ocv::image_container::vital_to_ocv(
        vital_image->get_image(), arrows::ocv::image_container::RGB_COLOR );
    }
    catch( const vital::vital_exception& e )
    {
      LOG_ERROR( m_logger, "Caught exception reading image: " << e.what() );
      return;
    }

    // Early exit don't need to read all images every iteration
    if( format_mode == "adaptive" )
    {
      if( ( original_image.rows * original_image.cols ) < m_chip_adaptive_thresh )
      {
        if( m_always_write_image ||
            ( m_original_to_chip_size &&
              ( original_image.cols > m_chip_width ||
                original_image.rows > m_chip_height ) ) ||
            ( m_ensure_standard &&
              ( original_image.channels() != 3 ||
               !( ext == "jpg" || ext == "png" || ext == "jpeg" ) ) ) )
        {
          format_mode = "maintain_ar";
        }
        else
        {
          formatted_names.push_back( image_fn );
          formatted_truth.push_back( groundtruth[fid] );
          continue;
        }
      }
      else
      {
        format_mode = "chip_and_original";
      }
    }
    else if( format_mode == "original_and_resized" )
    {
      formatted_names.push_back( image_fn );
      formatted_truth.push_back( groundtruth[fid] );

      if( ( original_image.rows * original_image.cols ) < m_chip_adaptive_thresh )
      {
        format_mode = "maintain_ar";
      }
    }

    // Format image and write new ones to disk
    format_image_from_memory(
      original_image, groundtruth[fid], format_mode,
      formatted_names, formatted_truth );
  }
}

void
windowed_trainer::priv
::format_image_from_memory(
  const cv::Mat& image,
  vital::detected_object_set_sptr groundtruth,
  const std::string format_method,
  std::vector< std::string >& formatted_names,
  std::vector< vital::detected_object_set_sptr >& formatted_truth )
{
  cv::Mat resized_image;
  vital::detected_object_set_sptr scaled_groundtruth = groundtruth->clone();
  vital::detected_object_set_sptr filtered_truth;

  double resized_scale = 1.0;

  if( format_method != "disabled" )
  {
    resized_scale = format_image( image, resized_image,
      format_method, m_scale, m_chip_width, m_chip_height, m_black_pad );

    scaled_groundtruth->scale( resized_scale );
  }
  else
  {
    resized_image = image;
    scaled_groundtruth = groundtruth;
  }

  if( format_method != "chip" && format_method != "chip_and_original" )
  {
    vital::bounding_box_d roi_box( 0, 0, resized_image.cols, resized_image.rows );

    if( filter_detections_in_roi( scaled_groundtruth, roi_box, filtered_truth ) )
    {
      std::string img_file = generate_filename();
      write_chip_to_disk( img_file, resized_image );

      formatted_names.push_back( img_file );
      formatted_truth.push_back( filtered_truth );
    }
  }
  else
  {
    // Chip up and process scaled image
    for( int i = 0;
         i < resized_image.cols - m_chip_width + m_chip_step_width;
         i += m_chip_step_width )
    {
      int cw = i + m_chip_width;

      if( cw > resized_image.cols )
      {
        cw = resized_image.cols - i;
      }
      else
      {
        cw = m_chip_width;
      }

      for( int j = 0;
           j < resized_image.rows - m_chip_height + m_chip_step_height;
           j += m_chip_step_height )
      {
        // random downsampling
        if( m_chip_random_factor > 0.0 &&
              static_cast< double >( rand() ) / static_cast<double>( RAND_MAX )
                > m_chip_random_factor )
        {
          continue;
        }

        int ch = j + m_chip_height;

        if( ch > resized_image.rows )
        {
          ch = resized_image.rows - j;
        }
        else
        {
          ch = m_chip_height;
        }

        // Only necessary in a few circumstances when chip_step exceeds image size.
        if( ch < 0 || cw < 0 )
        {
          continue;
        }

        cv::Mat cropped_image = resized_image( cv::Rect( i, j, cw, ch ) );
        cv::Mat resized_crop;

        scale_image_maintaining_ar( cropped_image,
          resized_crop, m_chip_width, m_chip_height, m_black_pad );

        vital::bounding_box_d roi_box( i, j, i + m_chip_width, j + m_chip_height );

        if( filter_detections_in_roi( scaled_groundtruth, roi_box, filtered_truth ) )
        {
          std::string img_file = generate_filename();
          write_chip_to_disk( img_file, resized_crop );

          formatted_names.push_back( img_file );
          formatted_truth.push_back( filtered_truth );
        }
      }
    }

    // Process full sized image if enabled
    if( format_method == "chip_and_original" )
    {
      cv::Mat scaled_original;

      double scaled_original_scale = scale_image_maintaining_ar( image,
        scaled_original, m_chip_width, m_chip_height, m_black_pad );

      vital::detected_object_set_sptr scaled_original_dets_ptr = groundtruth->clone();
      scaled_original_dets_ptr->scale( scaled_original_scale );

      vital::bounding_box_d roi_box( 0, 0, scaled_original.cols, scaled_original.rows );

      if( filter_detections_in_roi( scaled_original_dets_ptr, roi_box, filtered_truth ) )
      {
        std::string img_file = generate_filename();
        write_chip_to_disk( img_file, scaled_original );

        formatted_names.push_back( img_file );
        formatted_truth.push_back( filtered_truth );
      }
    }
  }
}


bool
windowed_trainer::priv
::filter_detections_in_roi(
  vital::detected_object_set_sptr all_detections,
  vital::bounding_box_d region,
  vital::detected_object_set_sptr& filtered_detections )
{
  auto ie = all_detections->cend();

  filtered_detections = std::make_shared< vital::detected_object_set >();

  for( auto detection = all_detections->cbegin(); detection != ie; ++detection )
  {
    vital::bounding_box_d det_box = (*detection)->bounding_box();
    vital::bounding_box_d overlap = vital::intersection( region, det_box );

    if( det_box.width() < m_min_train_box_length ||
        det_box.height() < m_min_train_box_length )
    {
      return false;
    }

    if( det_box.area() > 0 &&
        overlap.max_x() > overlap.min_x() &&
        overlap.max_y() > overlap.min_y() &&
        overlap.area() / det_box.area() >= m_overlap_required )
    {
      std::string category;

      if( !(*detection)->type() )
      {
        LOG_ERROR( m_logger, "Input detection is missing type category" );
        return false;
      }

      (*detection)->type()->get_most_likely( category );

      if( !m_ignore_category.empty() && category == m_ignore_category )
      {
        continue;
      }
      else if( m_synthetic_labels )
      {
        if( m_category_map.find( category ) == m_category_map.end() )
        {
          m_category_map[ category ] = m_category_map.size() - 1;
        }
        category = std::to_string( m_category_map[ category ] );
      }
      else if( m_labels->has_class_name( category ) )
      {
        category = std::to_string( m_labels->get_class_id( category ) );
      }
      else
      {
        LOG_WARN( m_logger, "Ignoring unlisted class " << category );
        continue;
      }

      double min_x = overlap.min_x() - region.min_x();
      double min_y = overlap.min_y() - region.min_y();
      double max_x = overlap.max_x() - region.min_x();
      double max_y = overlap.max_y() - region.min_y();

      vital::bounding_box_d bbox( min_x, min_y, max_x, max_y );

      auto odet = (*detection)->clone();
      odet->set_bounding_box( bbox );

      filtered_detections->add( odet );
    }
  }

  return true;
}


std::string
windowed_trainer::priv
::generate_filename( const int len )
{
  static int sample_counter = 0;
  sample_counter++;

  std::ostringstream ss;
  ss << std::setw( len ) << std::setfill( '0' ) << sample_counter;
  std::string s = ss.str();

  return m_train_directory + div +
         m_chip_subdirectory + div +
         s + "." + m_chip_format;
}


void
windowed_trainer::priv
::write_chip_to_disk( const std::string& filename, const cv::Mat& image )
{
  m_image_io->save( filename,
    vital::image_container_sptr(
      new arrows::ocv::image_container( image,
        arrows::ocv::image_container::RGB_COLOR ) ) );
}


} } } // end namespace
