/*ckwg +29
 * Copyright 2016-2017 by Kitware, Inc.
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

#ifndef KWIVER_ENABLE_GPU
#define CPU_ONLY
#endif
#include <caffe/common.hpp>

#include "faster_rcnn_detector.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <kwiversys/SystemTools.hxx>

#include <vital/algo/dynamic_configuration.h>
#include <vital/io/eigen_io.h>
#include <vital/logger/logger.h>
#include <vital/types/descriptor.h>
#include <vital/types/vector.h>
#include <vital/util/cpu_timer.h>
#include <vital/util/wall_timer.h>
#include <vital/util/data_stream_reader.h>

#include <arrows/ocv/image_container.h>

#include <iostream>
#include <algorithm>
#include <cmath>
#include <math.h>
#include <utility>
#include <vector>

using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::TEST;

namespace kwiver {
namespace arrows {
namespace caffe {


// ==================================================================
class faster_rcnn_detector::priv
{
public:
  std::string m_prototxt_file;
  std::string m_classes_file;
  std::string m_caffe_model;
  std::vector< std::string > m_labels;
  bool m_enable_image_resizing;
  double m_target_size;
  cv::Scalar m_pixel_means;
  double m_max_size;
  std::shared_ptr< Net< float > > m_net;
  bool m_use_gpu;
  int m_gpu_id;
  bool m_use_box_deltas;
  bool m_chip_image;
  unsigned int m_chip_width;
  unsigned int m_chip_height;
  unsigned int m_stride;
  std::string m_descriptor_layer;

  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::algo::dynamic_configuration_sptr m_dynamic_scaling;

  std::pair< cv::Mat, double > prepair_image( cv::Mat const& in_image ) const;
  std::vector< Blob< float >* > set_up_inputs( std::pair< cv::Mat, double > const& pair ) const;

// ------------------------------------------------------------------
  priv()
    : m_enable_image_resizing( true ),
      m_target_size( 600 ),
      m_pixel_means( 102.9801, 115.9465, 122.7717 ),
      m_max_size( 1000 ),
      m_net( NULL ),
      m_use_gpu( false ),
      m_gpu_id( 0 ),
      m_use_box_deltas( true ),
      //TODO: redo these when parameterized
      m_chip_image( false ),
      m_chip_width( 450 ),
      m_chip_height( 400 ),
      m_stride( 375 ),
      m_descriptor_layer( "" ),
      m_logger( kwiver::vital::get_logger( "vital.faster_rcnn" ) )
  { }

  ~priv()
  { }

};


// ==================================================================
faster_rcnn_detector::
faster_rcnn_detector()
  : d( new priv() )
{ }


faster_rcnn_detector::
~faster_rcnn_detector()
{ }


// --------------------------------------------------------------------
vital::config_block_sptr
faster_rcnn_detector::
  get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  kwiver::vital::algo::dynamic_configuration::
    get_nested_algo_configuration( "scaling", config, d->m_dynamic_scaling );

  config->set_value( "classes", d->m_classes_file,
                     "Text file containing the names of the classes supported by this faster rcnn." );
  config->set_value( "prototxt", d->m_prototxt_file,
                     "Points the the Prototxt file" );
  config->set_value( "caffe_model", d->m_caffe_model, "The file that contains the model." );
  config->set_value( "enable_image_resizing", this->d->m_enable_image_resizing,
                     "Specifies whether the image will be resized in order to "
                     "satisfy the specified target_size. Resizing may still "
                     "occur if 'max_size' would otherwise be violated." );
  config->set_value( "target_size", this->d->m_target_size, "If the size of "
                     "the shorter axis of the image is different from"
                     "'target_size', the image will be resized such that its "
                     "shorter axis equals 'target_size' before passing to the"
                     "detector." );
  config->set_value( "max_size", this->d->m_max_size, "Largest size the image "
                     "can be (on one of its sides). If this is exceeded, the "
                     "image will be resampled to a reduced size before passing "
                     "to the detector." );
  config->set_value( "pixel_mean", vital::vector_3d( this->d->m_pixel_means[0], this->d->m_pixel_means[1], this->d->m_pixel_means[2] ),
                     "The mean pixel value for the provided model." );
  config->set_value( "use_gpu", this->d->m_use_gpu, "Use the gpu instead of the cpu." );
  config->set_value( "gpu_id", this->d->m_gpu_id, "What gpu to use." );
  config->set_value( "use_box_deltas", this->d->m_use_box_deltas, "Use the learned jitter deltas." );
  config->set_value( "chip_image",  this->d->m_chip_image, "Break the images into chunks and classify on each chunk" );
  config->set_value( "chip_width", this->d->m_chip_width, "Width for the chunk" );
  config->set_value( "chip_height", this->d->m_chip_height, "Height of the chunk" );
  config->set_value( "stride", this->d->m_stride, "Step size for the chunking (controls if the chunks have overlap)" );
  config->set_value( "descriptor_layer", this->d->m_descriptor_layer,
                     "Layer from the CNN to extract and use as the descriptor "
                     "for detected objects. This layer must be one that has "
                     "the same initial shape dimension as the RoI layer " );

  return config;
}


// --------------------------------------------------------------------
void
faster_rcnn_detector::
set_configuration( vital::config_block_sptr config_in )
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( config_in );

  kwiver::vital::algo::dynamic_configuration::
    set_nested_algo_configuration( "scaling", config, d->m_dynamic_scaling );

  this->d->m_classes_file  = config->get_value< std::string > ( "classes" );
  this->d->m_prototxt_file = config->get_value< std::string > ( "prototxt" );
  this->d->m_caffe_model = config->get_value< std::string > ( "caffe_model" );
  this->d->m_use_gpu = config->get_value< bool > ( "use_gpu" );
  this->d->m_gpu_id = config->get_value< bool > ( "gpu_id" );
  this->d->m_use_box_deltas = config->get_value< bool > ( "use_box_deltas" );
  this->d->m_chip_image = config->get_value< bool > ( "chip_image" );
  this->d->m_chip_width = config->get_value< unsigned int > ( "chip_width" );
  this->d->m_chip_height = config->get_value< unsigned int > ( "chip_height" );
  this->d->m_stride = config->get_value< unsigned int > ( "stride" );
  this->d->m_enable_image_resizing = config->get_value< bool > ( "enable_image_resizing" );
  this->d->m_descriptor_layer = config->get_value< std::string >( "descriptor_layer", "" );

  // Need to check for existence of files.

  if ( d->m_use_gpu )
  {
    Caffe::SetDevice( this->d->m_gpu_id );
    Caffe::set_mode( Caffe::GPU );
  }
  else
  {
    Caffe::set_mode( Caffe::CPU );
  }

  this->d->m_net.reset( new Net< float > ( this->d->m_prototxt_file, TEST ) );
  this->d->m_net->CopyTrainedLayersFrom( this->d->m_caffe_model );
  // TODO(paul.tunison): check that configured descriptor extraction layer exists in network.

  // Should use data stream reader
  std::vector< std::string > labels;
  std::ifstream in( this->d->m_classes_file.c_str() );
  kwiver::vital::data_stream_reader dsr( in );
  dsr.add_editor( new kwiver::vital::edit_operation::left_trim() );

  std::string line;
  while ( dsr.getline( line ) )
  {
    this->d->m_labels.push_back( line );
  }

  this->d->m_target_size = config->get_value< double > ( "target_size" );

  vital::vector_3d tmp = config->get_value< vital::vector_3d > ( "pixel_mean" );
  this->d->m_pixel_means = cv::Scalar( tmp.x(), tmp.y(), tmp.z() );

  this->d->m_max_size = config->get_value< double > ( "max_size" );

  LOG_DEBUG( d->m_logger, "max_size: " << d->m_max_size );

} // faster_rcnn_detector::set_configuration


// --------------------------------------------------------------------
bool
faster_rcnn_detector::
check_configuration( vital::config_block_sptr config ) const
{
  std::string classes = config->get_value< std::string > ( "classes" );
  std::string prototxt = config->get_value< std::string > ( "prototxt" );
  std::string caffemodel = config->get_value< std::string > ( "caffe_model" );
  // optionally defined parameters
  std::string descriptor_layer = config->get_value< std::string >( "descriptor_layer", "" );

  bool success( true );

  if ( config->has_value( "scaling" ))
  {
    success &= kwiver::vital::algo::dynamic_configuration::
      check_nested_algo_configuration( "scaling", config );
  }

  if ( Caffe::mode() != ( ( d->m_use_gpu ) ? Caffe::GPU : Caffe::CPU ) )
  {
    if ( d->m_use_gpu )
    {
      Caffe::SetDevice( this->d->m_gpu_id );
      Caffe::set_mode( Caffe::GPU );
    }
    else
    {
      Caffe::set_mode( Caffe::CPU );
    }
  }

  if ( classes.empty() )
  {
    LOG_ERROR( d->m_logger, "Required classes file not specified" );
    success = false;
  }
  else if ( ! kwiversys::SystemTools::FileExists( classes ) )
  {
    LOG_ERROR( d->m_logger, "classes file \"" << classes << "\" not found." );
    success = false;
  }

  if ( prototxt.empty() )
  {
    LOG_ERROR( d->m_logger, "Required prototxt file not specified" );
    success = false;
  }
  else if ( ! kwiversys::SystemTools::FileExists( prototxt ) )
  {
    LOG_ERROR( d->m_logger, "prototxt file \"" << prototxt << "\" not found." );
    success = false;
  }

  if ( caffemodel.empty() )
  {
    LOG_ERROR( d->m_logger, "Required caffemodel file not specified" );
    success = false;
  }
  else if ( ! kwiversys::SystemTools::FileExists( caffemodel ) )
  {
    LOG_ERROR( d->m_logger, "caffe_model file \"" << caffemodel << "\" not found." );
    success = false;
  }

  // Make a temporary net (without loading the model) to check if the optional
  // descriptor layer exists in the network topology.
  //
  if( success && (descriptor_layer.size() > 0) )
  {
    Net< float > tmp_net( prototxt, TEST );
    using vec_t = std::vector< std::string >;
    const vec_t& blob_names = tmp_net.blob_names();
    vec_t::const_iterator it = std::find( blob_names.begin(), blob_names.end(), descriptor_layer );
    if( it == blob_names.end() )
    {
      LOG_ERROR( d->m_logger, "Invalid layer name \"" + descriptor_layer +
                              "\" specified for descriptor extraction." );
      LOG_ERROR( d->m_logger, "The following are blob layers available in the "
                              "configured network:" );
      for( std::string const & s : blob_names )
      {
        LOG_ERROR( d->m_logger, "Caffe blog name: " + s );
      }
      success = false;
    }
  }

  return success;
} // faster_rcnn_detector::check_configuration


// --------------------------------------------------------------------
vital::detected_object_set_sptr
faster_rcnn_detector::
detect( vital::image_container_sptr image_data ) const
{
  kwiver::vital::cpu_timer cpu_timer;
  kwiver::vital::wall_timer wall_timer;
  cpu_timer.start();
  wall_timer.start();
  LOG_TRACE( d->m_logger, "Received " + std::to_string(image_data->width()) +
             " x " + std::to_string(image_data->height()) + " x " +
             std::to_string(image_data->depth()) + " image" );

  double scale_factor(1.0);

  // Is dynamic scaling configured
  if (d->m_dynamic_scaling)
  {
    auto dyn_cfg = d->m_dynamic_scaling->get_dynamic_configuration();
    scale_factor = dyn_cfg->get_value<double>( "scale_factor", 1.0 );
  }

  //Convert to opencv image
  if ( Caffe::mode() != ( ( d->m_use_gpu ) ? Caffe::GPU : Caffe::CPU ) )
  {
    if ( d->m_use_gpu )
    {
      Caffe::SetDevice( this->d->m_gpu_id );
      Caffe::set_mode( Caffe::GPU );
    }
    else
    {
      Caffe::set_mode( Caffe::CPU );
    }
  }

  cv::Mat image = kwiver::arrows::ocv::image_container::vital_to_ocv( image_data->get_image(), kwiver::arrows::ocv::image_container::BGR);
  std::vector< cv::Mat > image_chips;
  std::vector< unsigned int > chip_x;
  std::vector< unsigned int > chip_y;

  if ( this->d->m_chip_image )
  {
    for ( unsigned int ux = 0; ux < image_data->width(); ux += this->d->m_stride )
    {
      unsigned int tux = ux;
      if ( tux + this->d->m_chip_width > image_data->width() )
      {
        tux = image_data->width() - this->d->m_chip_width - 1;
        if ( tux >= image_data->width() )
        {
          continue;
        }
      }

      for ( unsigned int uy = 0; uy < image_data->height(); uy += this->d->m_stride )
      {
        unsigned int tuy = uy;
        if ( tuy + this->d->m_chip_height > image_data->height() )
        {
          tuy = image_data->height() - this->d->m_chip_height - 1;
          if ( tuy >= image_data->height() )
          {
            continue;
          }
        }
        cv::Mat cropedImage = image( cv::Rect( tux, tuy, this->d->m_chip_width, this->d->m_chip_height ) );
        image_chips.push_back( cropedImage );
        chip_x.push_back( tux );
        chip_y.push_back( tuy );
      }
    }
  }
  else
  {
    image_chips.push_back( image );
    chip_x.push_back( 0 );
    chip_y.push_back( 0 );
  }

  auto detected_objects = std::make_shared<vital::detected_object_set>();

  for ( size_t img_at = 0; img_at < image_chips.size(); ++img_at )
  {
    std::pair< cv::Mat, double > image_scale = this->d->prepair_image( image_chips[img_at] );
    std::vector< Blob< float >* > input_layers = this->d->set_up_inputs( image_scale );
    this->d->m_net->Forward( input_layers );

    //get output - boost required
    boost::shared_ptr< Blob< float > > rois = this->d->m_net->blob_by_name( "rois" );
    boost::shared_ptr< Blob< float > > probs = this->d->m_net->blob_by_name( "cls_prob" );
    boost::shared_ptr< Blob< float > > rois_deltas = this->d->m_net->blob_by_name( "bbox_pred" );
    // Extract appropriate layer blob here for a detection's "descriptor"
    boost::shared_ptr< Blob< float > > descriptors = NULL;
    if( ! this->d->m_descriptor_layer.empty() )
    {
      descriptors = this->d->m_net->blob_by_name( this->d->m_descriptor_layer );
    }

    LOG_TRACE( d->m_logger, "Detected " << rois->count() << " RoIs in img_at "
                            << img_at << "." );

    // Array dimensions of blob contents for the different extracted layers.
    // Definitions: Blob->count() :: Total size of the blob vector
    //                                 (num * channels * heigh * width).
    //              Blob->num()   :: Equivalent to shape[0]; Number of discrete
    //                                 elements in the blob.
    // We should have a prob for each RoI.
    assert( rois->num() == probs->num() );
    const unsigned int roi_dim = rois->count() / rois->num();
    assert( roi_dim == 5 );
    const unsigned int prob_dim = probs->count() / probs->num();
    // We should have a "descriptor" for each RoI.
    unsigned int descr_dim = 0;
    if( descriptors != NULL )
    {
      assert( rois->num() == descriptors->num() );
      descr_dim = descriptors->count() / descriptors->num();
      LOG_TRACE( d->m_logger, "Extracting descriptors from layer \""
                              << d->m_descriptor_layer << "\""
                              << " with dimensionality: " << descr_dim );
    }

    // Loop over "detections"
    for ( int i = 0; i < rois->num(); ++i )
    {
      const float* start = rois->cpu_data() + rois->offset( i );
      double pts[4];

      // Extract detection bounding points, rescaled back to input image
      // coordinates.
      for ( unsigned int j = 1; j < roi_dim; ++j )
      {
        pts[j - 1] = start[j] / image_scale.second;
      }

      vital::bounding_box_d bbox( vital::vector_2d( pts[0] + chip_x[img_at], pts[1] + chip_y[img_at] ),
                                  vital::vector_2d( pts[2] + chip_x[img_at], pts[3] + chip_y[img_at] ) );

      // Extracting "descriptor"
      vital::descriptor_sptr d_sptr;
      if( descriptors != NULL )
      {
        start = descriptors->cpu_data() + descriptors->offset( i );
        d_sptr = std::make_shared< vital::descriptor_dynamic<float> >( descr_dim, start );
      }

      // Vector of probability values.
      start = probs->cpu_data() + probs->offset( i );
      std::vector< double > prob_v(start, start + prob_dim);

      if ( this->d->m_use_box_deltas && ( rois_deltas != NULL ) )
      {
        start = rois_deltas->cpu_data() + rois_deltas->offset( i );

        // Make a single detection for each classification since the bbox will be a little different
        for ( unsigned int j = 0; j < prob_dim; ++j )
        {
          unsigned int ds = j * 4; // TODO calc for more robustness
          float dx = start[ds];
          float dy = start[ds + 1];
          float dw = start[ds + 2];
          float dh = start[ds + 3];
          auto center = bbox.center();
          float w = bbox.width();
          float h = bbox.height();
          center[0] = dx * w + center[0];
          center[1] = dy * h + center[1];
          float pw = exp( dw ) * w;
          float ph = exp( dh ) * h;
          vital::vector_2d halfS( pw * 0.5, ph * 0.5 );

          vital::bounding_box_d pbox( center - halfS, center + halfS );

          auto classification = std::make_shared<vital::detected_object_type>();
          classification->set_score( this->d->m_labels[j], prob_v[j] );

          vital::detected_object_sptr do_sptr = std::make_shared<vital::detected_object>(
              pbox, 1.0, classification );
          do_sptr->set_descriptor( d_sptr );
          detected_objects->add( do_sptr );
        } // end for j
      }
      else
      {
        // just make one detection object with all class-names using one bbox
        auto classification = std::make_shared<vital::detected_object_type>( this->d->m_labels, prob_v );
        vital::detected_object_sptr do_sptr = std::make_shared< vital::detected_object >(
            bbox, 1.0, classification );
        do_sptr->set_descriptor( d_sptr );
        detected_objects->add( do_sptr );
      }
    } // end for over rois
  } // end for over chips

  cpu_timer.stop();
  wall_timer.stop();
  LOG_TRACE( logger(), "Elapsed wall/CPU time detecting objects: " <<
             wall_timer.elapsed() << " / " << cpu_timer.elapsed()  );

  return detected_objects;
} // faster_rcnn_detector::detect


// ==================================================================
std::vector< Blob< float >* >
faster_rcnn_detector::priv::
set_up_inputs( std::pair< cv::Mat, double > const& pair ) const
{
  cv::Size s = pair.first.size();
  int width = s.width;
  int height = s.height;

  std::vector< Blob< float >* > results;

  { // image layers
    std::vector< cv::Mat > input_channels;
    Blob< float >* image_layer = this->m_net->input_blobs()[0];
    image_layer->Reshape( 1, //Number of images
                          pair.first.channels(), height, width );
    float* input_data = image_layer->mutable_cpu_data();

    for ( int i = 0; i < image_layer->channels(); ++i )
    {
      cv::Mat channel( height, width, CV_32FC1, input_data );
      input_channels.push_back( channel );
      input_data += width * height;
    }

    cv::split( pair.first, input_channels );
    results.push_back( image_layer );
  }

  { // image data
    std::vector< int > input( 2 );
    input[0] = 1; //number of images
    input[1] = 3;
    Blob< float >* image_info = this->m_net->input_blobs()[1];
    image_info->Reshape( input );
    float* input_data = image_info->mutable_cpu_data();
    input_data[0] = height;
    input_data[1] = width;
    input_data[2] = pair.second;
    results.push_back( image_info );
  }

  return results;
}


// --------------------------------------------------------------------
std::pair< cv::Mat, double >
faster_rcnn_detector::priv::
prepair_image( cv::Mat const& in_image ) const
{
  cv::Mat im_float;

  in_image.convertTo( im_float, CV_32F );
  im_float = im_float - this->m_pixel_means;

  const double min_size = std::min( im_float.size[0], im_float.size[1] );
  const double max_size = std::max( im_float.size[0], im_float.size[1] );

  double scale;
  if( m_enable_image_resizing )
  {
    scale = this->m_target_size / min_size;
  }
  else
  {
    scale = 1;
  }

  if ( round( scale * max_size ) > this->m_max_size )
  {
    scale = this->m_max_size / max_size;
  }

  cv::Mat scaleImage;
  if( scale != 1 )
  {
    cv::resize( im_float, scaleImage, cv::Size(), scale, scale );
    LOG_TRACE( m_logger, "Rescaling image to " +
               std::to_string(scaleImage.cols) + "x" +
               std::to_string(scaleImage.rows) + "x" +
               std::to_string(scaleImage.channels()) );
  }
  else
  {
    scaleImage = im_float;
  }

  return std::pair< cv::Mat, double > ( scaleImage, scale );
}

} } }     // end namespace
