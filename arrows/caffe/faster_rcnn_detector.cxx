/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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

#include <caffe/common.hpp>

#include "faster_rcnn_detector.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <vital/types/vector.h>
#include <vital/io/eigen_io.h>
#include <vital/util/cpu_timer.h>
#include <vital/util/data_stream_reader.h>
#include <vital/logger/logger.h>
#include <kwiversys/SystemTools.hxx>

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

  kwiver::vital::logger_handle_t m_logger;

  std::pair< cv::Mat, double > prepair_image( cv::Mat const& in_image ) const;
  std::vector< Blob< float >* > set_up_inputs( std::pair< cv::Mat, double > const& pair ) const;

// ------------------------------------------------------------------
  priv()
    : m_target_size( 600 ),
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

  config->set_value( "classes", d->m_classes_file,
                     "Text file containing the names of the classes supported by this faster rcnn." );
  config->set_value( "prototxt", d->m_prototxt_file,
                     "Points the the Prototxt file" );
  config->set_value( "caffe_model", d->m_caffe_model, "The file that contains the model." );
  config->set_value( "target_size", this->d->m_target_size, "TODO" );
  config->set_value( "max_size", this->d->m_max_size, "TODO" );
  config->set_value( "pixel_mean", vital::vector_3d( this->d->m_pixel_means[0], this->d->m_pixel_means[1], this->d->m_pixel_means[2] ),
                     "The mean pixel value for the provided model." );
  config->set_value( "use_gpu", this->d->m_use_gpu, "Use the gpu instead of the cpu." );
  config->set_value( "gpu_id", this->d->m_gpu_id, "What gpu to use." );
  config->set_value( "use_box_deltas", this->d->m_use_box_deltas, "Use the learned jitter deltas." );
  config->set_value( "chip_image",  this->d->m_chip_image, "TODO" );
  config->set_value( "chip_width", this->d->m_chip_width, "TODO" );
  config->set_value( "chip_height", this->d->m_chip_height, "TODO" );
  config->set_value( "stride", this->d->m_stride, "TODO" );

  return config;
}


// --------------------------------------------------------------------
void
faster_rcnn_detector::
set_configuration( vital::config_block_sptr config_in )
{
  //+ is this merge really needed?
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( config_in );

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

  // Need to check for existance of files.

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
} // faster_rcnn_detector::set_configuration


// --------------------------------------------------------------------
bool
faster_rcnn_detector::
check_configuration( vital::config_block_sptr config ) const
{
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

  std::string classes = config->get_value< std::string > ( "classes" );
  std::string prototxt = config->get_value< std::string > ( "prototxt" );
  std::string caffemodel = config->get_value< std::string > ( "caffe_model" );

  bool success( true );

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

  return success;
} // faster_rcnn_detector::check_configuration


// --------------------------------------------------------------------
vital::detected_object_set_sptr
faster_rcnn_detector::
detect( vital::image_container_sptr image_data ) const
{
  //Convert to opencv image
  if ( image_data == NULL ) //+ this should never happen
  {
    return vital::detected_object_set_sptr();
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

  kwiver::vital::scoped_cpu_timer t( "Time to Detect Objects" );
  cv::Mat image = kwiver::arrows::ocv::image_container::vital_to_ocv( image_data->get_image() );
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

    const unsigned int roi_dim = rois->count() / rois->num();
    assert( roi_dim == 5 );
    const unsigned int prob_dim = probs->count() / probs->num();
    assert( rois->num() == probs->num() );

    // Loop over "detections"
    for ( int i = 0; i < rois->num(); ++i )
    {
      const float* start = rois->cpu_data() + rois->offset( i );
      double pts[4];

      for ( unsigned int j = 1; j < roi_dim; ++j )
      {
        pts[j - 1] = start[j] / image_scale.second;
      }

      vital::bounding_box_d bbox( vital::vector_2d( pts[0] + chip_x[img_at], pts[1] + chip_y[img_at] ),
                                  vital::vector_2d( pts[2] + chip_x[img_at], pts[3] + chip_y[img_at] ) );

      start = probs->cpu_data() + probs->offset( i );

      // Convert array to vector for safe keeping
      std::vector< double > tmpv(start, start + prob_dim);

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
          classification->set_score( this->d->m_labels[j], tmpv[j] );

          detected_objects->add( std::make_shared<vital::detected_object>( pbox, 1.0, classification ) );
        } // end for j
      }
      else
      {
        // just make one detection object with all class-names using one bbox
        auto classification = std::make_shared<vital::detected_object_type>( this->d->m_labels, tmpv );
        detected_objects->add( std::make_shared<vital::detected_object>( bbox, 1.0, classification ) );
      }
    } // end for over rois
  } // end for over chips

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

  double scale = this->m_target_size / min_size;

  if ( round( scale * max_size ) > this->m_max_size )
  {
    scale = this->m_max_size / max_size;
  }

  cv::Mat scaleImage;
  cv::resize( im_float, scaleImage, cv::Size(), scale, scale );

  return std::pair< cv::Mat, double > ( scaleImage, scale );
}

} } }     // end namespace
