/*ckwg +29
 * Copyright 2017-2019 by Kitware, Inc.
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

#include "darknet_detector.h"
#include "darknet_custom_resize.h"

// kwiver includes
#include <vital/util/cpu_timer.h>
#include <vital/exceptions/io.h>
#include <vital/config/config_block_formatter.h>

#include <arrows/ocv/image_container.h>
#include <kwiversys/SystemTools.hxx>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <string>
#include <sstream>
#include <exception>
#include <limits>

// Since we need to wrap darknet.h in an extern C
// We need to include CUDA/CUDNN before darknet does
// so C++ gets a hold of them first
// We may want to wrap all the declarations in darknet.h in
// an extern C rather than have to do it here.

#ifdef DARKNET_USE_GPU
  #define GPU
  #include "cuda_runtime.h"
  #include "curand.h"
  #include "cublas_v2.h"
  #ifdef DARKNET_USE_CUDNN
    #define CUDNN
    #include "cudnn.h"
  #endif
#endif
extern "C" {
#include "darknet.h"
}

namespace kwiver {
namespace arrows {
namespace darknet {

// =============================================================================
class darknet_detector::priv
{
public:
  priv()
    : m_thresh( 0.24 )
    , m_hier_thresh( 0.5 )
    , m_gpu_index( -1 )
    , m_resize_option( "disabled" )
    , m_scale( 1.0 )
    , m_chip_step( 100 )
    , m_nms_threshold( 0.4 )
    , m_gs_to_rgb( true )
    , m_chip_edge_filter( 0 )
    , m_chip_adaptive_thresh( 2000000 )
    , m_is_first( true )
    , m_names( 0 )
  { }

  ~priv()
  {
    free( m_names );
  }

  // Items from the config
  std::string m_net_config;
  std::string m_weight_file;
  std::string m_class_names;

  float m_thresh;
  float m_hier_thresh;
  int m_gpu_index;

  std::string m_resize_option;
  double m_scale;
  int m_chip_step;
  double m_nms_threshold;
  bool m_gs_to_rgb;
  int m_chip_edge_filter;
  int m_chip_adaptive_thresh;
  bool m_is_first;

  // Needed to operate the model
  char **m_names;                 /* list of classes/labels */
  network m_net;

  // Helper functions
  struct region_info
  {
    explicit region_info( cv::Rect r, double s1 )
     : original_roi( r ), edge_filter( 0 ),
       scale1( s1 ), shiftx( 0 ), shifty( 0 ), scale2( 1.0 )
    {}

    explicit region_info( cv::Rect r, int ef,
      double s1, int sx, int sy, double s2 )
     : original_roi( r ), edge_filter( ef ),
       scale1( s1 ), shiftx( sx ), shifty( sy ), scale2( s2 )
    {}

    cv::Rect original_roi;
    int edge_filter;
    double scale1;
    int shiftx, shifty;
    double scale2;
  };

  image cvmat_to_image(
    const cv::Mat& src ,
    unsigned max_channels = std::numeric_limits< unsigned >::max() );

  std::vector< vital::detected_object_set_sptr > process_images(
    const std::vector< cv::Mat >& cv_image );

  vital::detected_object_set_sptr scale_detections(
    const vital::detected_object_set_sptr detections,
    const region_info& roi );

  kwiver::vital::logger_handle_t m_logger;
};


// =============================================================================
darknet_detector
::darknet_detector()
  : d( new priv() )
{
  attach_logger( "arrows.darknet.darknet_detector" );
  d->m_logger = logger();

  // set darknet global GPU index
  gpu_index = d->m_gpu_index;
}


darknet_detector
::~darknet_detector()
{}


// -----------------------------------------------------------------------------
vital::config_block_sptr
darknet_detector
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value( "net_config", d->m_net_config,
    "Name of network config file." );
  config->set_value( "weight_file", d->m_weight_file,
    "Name of optional weight file." );
  config->set_value( "class_names", d->m_class_names,
    "Name of file that contains the class names." );
  config->set_value( "thresh", d->m_thresh,
    "Threshold value." );
  config->set_value( "hier_thresh", d->m_hier_thresh,
    "Hier threshold value." );
  config->set_value( "gpu_index", d->m_gpu_index,
    "GPU index. Only used when darknet is compiled with GPU support." );
  config->set_value( "resize_option", d->m_resize_option,
    "Pre-processing resize option, can be: disabled, maintain_ar, scale, "
    "chip, chip_and_original, or adaptive." );
  config->set_value( "scale", d->m_scale,
    "Image scaling factor used when resize_option is scale or chip." );
  config->set_value( "chip_step", d->m_chip_step,
    "When in chip mode, the chip step size between chips." );
  config->set_value( "nms_threshold", d->m_nms_threshold,
    "Non-maximum suppression threshold." );
  config->set_value( "gs_to_rgb", d->m_gs_to_rgb,
    "Convert input greyscale images to rgb before processing." );
  config->set_value( "chip_edge_filter", d->m_chip_edge_filter,
    "If using chipping, filter out detections this pixel count near borders." );
  config->set_value( "chip_adaptive_thresh", d->m_chip_adaptive_thresh,
    "If using adaptive selection, total pixel count at which we start to chip." );

  return config;
}


// -----------------------------------------------------------------------------
void
darknet_detector
::set_configuration( vital::config_block_sptr config_in )
{
  // Starting with our generated config_block to ensure that assumed values
  // are present. An alternative is to check for key presence before performing
  // a get_value() call.
  vital::config_block_sptr config = this->get_configuration();

  config->merge_config( config_in );

  this->d->m_net_config  = config->get_value< std::string >( "net_config" );
  this->d->m_weight_file = config->get_value< std::string >( "weight_file" );
  this->d->m_class_names = config->get_value< std::string >( "class_names" );
  this->d->m_thresh      = config->get_value< float >( "thresh" );
  this->d->m_hier_thresh = config->get_value< float >( "hier_thresh" );
  this->d->m_gpu_index   = config->get_value< int >( "gpu_index" );
  this->d->m_resize_option = config->get_value< std::string >( "resize_option" );
  this->d->m_scale       = config->get_value< double >( "scale" );
  this->d->m_chip_step   = config->get_value< int >( "chip_step" );
  this->d->m_nms_threshold = config->get_value< double >( "nms_threshold" );
  this->d->m_gs_to_rgb   = config->get_value< bool >( "gs_to_rgb" );
  this->d->m_chip_edge_filter = config->get_value< int >( "chip_edge_filter" );
  this->d->m_chip_adaptive_thresh = config->get_value< int >( "chip_adaptive_thresh" );

  /* the size of this array is a mystery - probably has to match some
   * constant in net description */

#ifdef DARKNET_USE_GPU
  if( d->m_gpu_index >= 0 )
  {
    cuda_set_device( d->m_gpu_index );
  }
#endif

  // Open file and return 'list' of labels
  d->m_names = get_labels( const_cast< char* >( d->m_class_names.c_str() ) );

  network* net = load_network( const_cast< char* >( d->m_net_config.c_str() ),
                               const_cast< char* >( d->m_weight_file.c_str() ), 0 );
  d->m_net = *net;
  free( net );

  set_batch_network( &d->m_net, 1 );

  // This assumes that there are no other users of random number
  // generator in this application.
  srand( 2222222 );
} // darknet_detector::set_configuration


// -----------------------------------------------------------------------------
bool
darknet_detector
::check_configuration( vital::config_block_sptr config ) const
{
  std::string net_config = config->get_value<std::string>( "net_config" );
  std::string class_file = config->get_value<std::string>( "class_names" );

  bool success = true;

  if( net_config.empty() )
  {
    std::stringstream str;
    kwiver::vital::config_block_formatter fmt( config );
    fmt.print( str );
    LOG_ERROR( logger(), "Required net config file not specified. "
      "Configuration is as follows:\n" << str.str() );
    success = false;
  }
  else if( !kwiversys::SystemTools::FileExists( net_config ) )
  {
    LOG_ERROR( logger(), "net config file \"" << net_config << "\" not found." );
    success = false;
  }

  if( class_file.empty() )
  {
    std::stringstream str;
    kwiver::vital::config_block_formatter fmt( config );
    fmt.print( str );
    LOG_ERROR( logger(), "Required class name list file not specified, "
      "Configuration is as follows:\n" << str.str() );
    success = false;
  }
  else if( ! kwiversys::SystemTools::FileExists( class_file ) )
  {
    LOG_ERROR( logger(), "class names file \"" << class_file << "\" not found." );
    success = false;
  }

  return success;
} // darknet_detector::check_configuration


// -----------------------------------------------------------------------------
vital::detected_object_set_sptr
darknet_detector
::detect( vital::image_container_sptr image_data ) const
{
  kwiver::vital::scoped_cpu_timer t( "Time to Detect Objects" );

  if( !image_data )
  {
    LOG_WARN( d->m_logger, "Input image is empty." );
    return std::make_shared< vital::detected_object_set >();
  }

  cv::Mat cv_image = kwiver::arrows::ocv::image_container::vital_to_ocv(
    image_data->get_image(), kwiver::arrows::ocv::image_container::RGB_COLOR );

  if( cv_image.rows == 0 || cv_image.cols == 0 )
  {
    LOG_WARN( d->m_logger, "Input image is empty." );
    return std::make_shared< vital::detected_object_set >();
  }
  else if( d->m_resize_option == "adaptive" )
  {
    if( ( cv_image.rows * cv_image.cols ) >= d->m_chip_adaptive_thresh )
    {
      d->m_resize_option = "chip_and_original";
    }
    else
    {
      d->m_resize_option = "maintain_ar";
    }
  }

  cv::Mat cv_resized_image;

  vital::detected_object_set_sptr detections;

  // resizes image if enabled
  double scale_factor = 1.0;

  if( d->m_resize_option != "disabled" )
  {
    scale_factor = format_image( cv_image, cv_resized_image,
      d->m_resize_option, d->m_scale, d->m_net.w, d->m_net.h );
  }
  else
  {
    cv_resized_image = cv_image;
  }

  if( d->m_gs_to_rgb && cv_resized_image.channels() == 1 )
  {
    cv::Mat color_image;
    cv::cvtColor( cv_resized_image, color_image, CV_GRAY2RGB );
    cv_resized_image = color_image;
  }

  // Run detector
  detections = std::make_shared< vital::detected_object_set >();

  cv::Rect original_dims( 0, 0, cv_image.cols, cv_image.rows );

  std::vector< cv::Mat > regions_to_process;
  std::vector< priv::region_info > region_properties;

  if( d->m_resize_option != "chip" && d->m_resize_option != "chip_and_original" )
  {
    regions_to_process.push_back( cv_resized_image );

    region_properties.push_back(
      priv::region_info( original_dims, 1.0 / scale_factor ) );
  }
  else
  {
    // Chip up scaled image
    for( int li = 0;
         li < cv_resized_image.cols - d->m_net.w + d->m_chip_step;
         li += d->m_chip_step )
    {
      int ti = std::min( li + d->m_net.w, cv_resized_image.cols );

      for( int lj = 0;
           lj < cv_resized_image.rows - d->m_net.h + d->m_chip_step;
           lj += d->m_chip_step )
      {
        int tj = std::min( lj + d->m_net.h, cv_resized_image.rows );

        cv::Rect resized_roi( li, lj, ti-li, tj-lj );
        cv::Rect original_roi( li / scale_factor,
                               lj / scale_factor,
                               (ti-li) / scale_factor,
                               (tj-lj) / scale_factor );

        cv::Mat cropped_chip = cv_resized_image( resized_roi );
        cv::Mat scaled_crop, tmp_cropped;

        double scaled_crop_scale = scale_image_maintaining_ar(
          cropped_chip, scaled_crop, d->m_net.w, d->m_net.h );

        regions_to_process.push_back( scaled_crop );

        region_properties.push_back(
          priv::region_info( original_roi,
            d->m_chip_edge_filter,
            1.0 / scaled_crop_scale,
            li, lj,
            1.0 / scale_factor ) );
      }
    }

    // Extract full sized image chip if enabled
    if( d->m_resize_option == "chip_and_original" )
    {
      cv::Mat scaled_original;

      double scaled_original_scale = scale_image_maintaining_ar( cv_image,
        scaled_original, d->m_net.w, d->m_net.h );

      if( d->m_gs_to_rgb && scaled_original.channels() == 1 )
      {
        cv::Mat color_image;
        cv::cvtColor( scaled_original, color_image, CV_GRAY2RGB );
        scaled_original = color_image;
      }

      regions_to_process.push_back( scaled_original );

      region_properties.push_back(
        priv::region_info( original_dims, 1.0 / scaled_original_scale ) );
    }
  }

  // Process all regions
  unsigned max_count = d->m_net.batch;

  for( unsigned i = 0; i < regions_to_process.size(); i+= max_count )
  {
    unsigned batch_size = std::min( max_count,
      static_cast< unsigned >( regions_to_process.size() ) - i );

    std::vector< cv::Mat > imgs;

    for( unsigned j = 0; j < batch_size; j++ )
    {
      imgs.push_back( regions_to_process[ i + j ] );
    }

    std::vector< vital::detected_object_set_sptr > out = d->process_images( imgs );

    for( unsigned j = 0; j < batch_size; j++ )
    {
      detections->add( d->scale_detections( out[ j ], region_properties[ i + j ] ) );
    }
  }

  return detections;
} // darknet_detector::detect


// =============================================================================
std::vector< vital::detected_object_set_sptr >
darknet_detector::priv
::process_images( const std::vector< cv::Mat >& cv_images )
{
  // set batch size to 1 if on the first frame we're just given 1 frame, it's
  // almost guaranteed on all other frames we'll also just be given one. Why
  // use batching then?
  if( m_is_first )
  {
    if( cv_images.size() == 1 && m_net.batch != 1 )
    {
      set_batch_network( &m_net, 1 );
    }
    m_is_first = false;
  }

  // copies and converts to floating pixel value.
  unsigned image_size = m_net.w * m_net.h * m_net.c;
  float* X = static_cast< float* >( calloc( m_net.batch * image_size, sizeof( float ) ) );
  layer l = m_net.layers[ m_net.n - 1 ]; /* last network layer */

  for( unsigned i = 0; i < cv_images.size(); i++ )
  {
    // Error checking
    if( cv_images[i].channels() != m_net.c && !( cv_images[i].channels() == 5 && m_net.c == 3 ) )
    {
      throw std::runtime_error( "Model channel count (" + std::to_string( m_net.c ) + ") "
        + "does not match input image count (" + std::to_string( cv_images[i].channels() ) + ")" );
    }

    // Copy in image
    image img = cvmat_to_image( cv_images[i], m_net.c );

    if( img.w == m_net.w && img.h == m_net.h )
    {
      memcpy( X + i * image_size, img.data, image_size * sizeof( float ) );
    }
    else
    {
      image sized = resize_image( img, m_net.w, m_net.h );
      memcpy( X + i * image_size, sized.data, image_size * sizeof( float ) );
      free_image( sized );
    }

    free_image( img );
  }

  /* run image through network */
  network_predict( &m_net, X );

  /* get boxes around detected objects */
  std::vector< vital::detected_object_set_sptr > output;

  std::vector< float* > original_outputs( m_net.n );

  if( cv_images.size() > 1 )
  {
    for( int j = 0; j < m_net.n; j++ )
    {
      layer *l = &( m_net.layers[j] );

      if( l->type == YOLO || l->type == REGION || l->type == DETECTION )
      {
        original_outputs[j] = l->output;
      }
    }
  }

  for( unsigned i = 0; i < cv_images.size(); i++ )
  {
    auto detected_objects = std::make_shared< vital::detected_object_set >();

    int det_count;

    detection* dets = get_network_boxes(
      &m_net,
      m_net.w, m_net.h, /* i: w, h */
      m_thresh,         /* i: caller supplied threshold */
      m_hier_thresh,    /* i: tree thresh, relative */
      0,                /* i: map */
      1,                /* i: tree thresh, relative */
      &det_count );     /* i: number of detections */

    if( l.softmax_tree && m_nms_threshold )
    {
      do_nms_obj( dets, det_count, l.classes, m_nms_threshold );
    }
    else if( m_nms_threshold )
    {
      do_nms_sort( dets, det_count, l.classes, m_nms_threshold );
    }
    else
    {
      LOG_ERROR( m_logger, "Internal error - nms == 0" );
    }

<<<<<<< HEAD
    // Iterate over all classes and collect all names over the threshold, and max score
    double conf = 0.0;

    for ( int class_idx = 0; class_idx < l.classes; ++class_idx )
    {
      const double prob = static_cast< double >( d->m_probs[i][class_idx] );
      if ( prob >= d->m_thresh )
      {
        const std::string class_name( d->m_names[class_idx] );
        dot->set_score( class_name, prob );
        conf = std::max( conf, prob );
        has_name = true;
=======
    // -- extract detections and convert to our format --
    for( int d = 0; d < det_count; ++d )
    {
      const box b = dets[d].bbox;

      int left  = ( b.x - b.w / 2. ) * m_net.w;
      int right = ( b.x + b.w / 2. ) * m_net.w;
      int top   = ( b.y - b.h / 2. ) * m_net.h;
      int bot   = ( b.y + b.h / 2. ) * m_net.h;

      /* clip box to image bounds */
      if( left < 0 )
      {
        left = 0;
      }
      if( right > m_net.w - 1 )
      {
        right = m_net.w - 1;
      }
      if( top < 0 )
      {
        top = 0;
      }
      if( bot > m_net.h - 1 )
      {
        bot = m_net.h - 1;
      }

      kwiver::vital::bounding_box_d bbox( left, top, right, bot );

      auto dot = std::make_shared< kwiver::vital::detected_object_type >();
      bool has_name = false;

      // Iterate over all classes and collect all names over the threshold
      double conf = 0.0;

      for( int class_idx = 0; class_idx < l.classes; ++class_idx )
      {
        const double prob = static_cast< double >( dets[d].prob[class_idx] );

        if( prob >= m_thresh )
        {
          const std::string class_name( m_names[ class_idx ] );
          dot->set_score( class_name, prob );
          conf = std::max( conf, prob );
          has_name = true;
        }
      }

      if( has_name )
      {
        detected_objects->add(
          std::make_shared< kwiver::vital::detected_object >(
            bbox, conf, dot ) );
>>>>>>> origin/viame/master
      }
    }

    free_detections( dets, det_count );

    output.push_back( detected_objects );

    if( cv_images.size() > 1 )
    {
<<<<<<< HEAD
      detected_objects->add( std::make_shared< kwiver::vital::detected_object >( bbox, conf, dot ) );
=======
      for( int j = 0; j < m_net.n; j++ )
      {
        layer *l = &( m_net.layers[j] );

        if( l->type == YOLO || l->type == REGION || l->type == DETECTION )
        {
          l->output += l->outputs;
        }
      }
>>>>>>> origin/viame/master
    }
  }

  if( cv_images.size() > 1 )
  {
    for( int j = 0; j < m_net.n; j++ )
    {
      layer *l = &( m_net.layers[j] );

      if( l->type == YOLO || l->type == REGION || l->type == DETECTION )
      {
        l->output = original_outputs[j];
      }
    }
  }

  free( X );

  return output;
}


image
darknet_detector::priv
::cvmat_to_image( const cv::Mat& src, unsigned max_channels )
{
  unsigned h = src.rows;
  unsigned w = src.cols;
  unsigned c = std::min( static_cast< unsigned >( src.channels() ), max_channels );

  unsigned istep = src.step[ 1 ];
  unsigned jstep = src.step[ 0 ];

  image out = make_image( w, h, c );

  unsigned int i, j, k;

  unsigned char* input = (unsigned char*)src.data;
  float* output = out.data;

  if( src.depth() == CV_8U )
  {
    for( k = 0; k < c; k++ )
    {
      for( j = 0; j < h; j++ )
      {
        for( i = 0; i < w; i++, output++ )
        {
          *output = input[j*jstep + i*istep + k] / 255.;
        }
      }
    }
  }
  else if( src.depth() == CV_16U )
  {
    for( k = 0; k < c; k++ )
    {
      for( j = 0; j < h; j++ )
      {
        for( i = 0; i < w; i++, output++ )
        {
          *output = *(unsigned short*)(input + j*jstep + i*istep + k) / 65535.;
        }
      }
    }
  }
  else
  {
    throw std::runtime_error( "Invalid image type received" );
  }

  return out;
}


vital::detected_object_set_sptr
darknet_detector::priv
::scale_detections(
  const vital::detected_object_set_sptr dets,
  const region_info& info )
{
  if( info.scale1 != 1.0 )
  {
    dets->scale( info.scale1 );
  }

  if( info.shiftx != 0 || info.shifty != 0 )
  {
    dets->shift( info.shiftx, info.shifty );
  }

  if( info.scale2 != 1.0 )
  {
    dets->scale( info.scale2 );
  }

  const int dist = info.edge_filter;

  if( dist <= 0 )
  {
    return dets;
  }

  const cv::Rect& roi = info.original_roi;

  std::vector< vital::detected_object_sptr > filtered_dets;

  for( auto det : *dets )
  {
    if( !det )
    {
      continue;
    }
    if( roi.x > 0 && det->bounding_box().min_x() < roi.x + dist )
    {
      continue;
    }
    if( roi.y > 0 && det->bounding_box().min_y() < roi.y + dist )
    {
      continue;
    }
    if( det->bounding_box().max_x() > roi.x + roi.width - dist )
    {
      continue;
    }
    if( det->bounding_box().max_y() > roi.y + roi.height - dist )
    {
      continue;
    }

    filtered_dets.push_back( det );
  }

  return vital::detected_object_set_sptr(
    new vital::detected_object_set( filtered_dets ) );
}


} } } // end namespace
