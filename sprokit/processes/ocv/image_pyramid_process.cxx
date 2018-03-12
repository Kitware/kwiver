/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS [yas] elisp error!AS IS''
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

#include "image_pyramid_process.h"

#include <vital/vital_types.h>
#include <vital/types/homography.h>
#include <vital/types/image_container.h>
#include <vital/util/wall_timer.h>
#include <arrows/ocv/image_container.h>
#include <sprokit/pipeline/process_exception.h>
#include <sprokit/processes/kwiver_type_traits.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace kwiver {

// ==================================================
// TBD A better approach would be to remove the warp algorithm reference
// in this process and do the warping in the pipeline.

create_config_trait( num_iterations, int, "1", "Number of times to apply the "
                     "transformation." );
create_config_trait( up_or_down, std::string, "down", "Indicates whether to "
                     "apply upsampling (\'up\') or downsampling (\'down\')." );


// On traversing up the pyramid, the scale doubles, but there is also a
// translational shift. The center of the upper left pixel in the source image,
// with image indices and image coordinate (0,0), is mapped to the shared corner
// of the four upper left pixels, which has image coordinates (0.5,0.5).
// Therefore, to map from the source image to the output image, you have to
// apply the following homography for each iteration.
//
//     |  2  0  0.5 |
// H = |  0  2  0.5 |
//     |  0  0   1  |
//
static const double coefficents[9] = {2,0,0,0,2,0,0.5,0.5,1};
static const Eigen::Matrix< double, 3, 3 > pyr_up_eigen_homog(coefficents);
static const Eigen::Matrix< double, 3, 3 > pyr_down_eigen_homog = pyr_up_eigen_homog.inverse();


//----------------------------------------------------------------
// Private implementation class
class image_pyramid_process::priv
{
public:
  // Configuration values
  int m_num_iterations;
  std::string m_up_or_down;
  kwiver::vital::wall_timer m_timer;
  Eigen::Matrix< double, 3, 3 > pyr_up_homog, pyr_down_homog;

  priv()
  {
  }

  // Apply pyr_up or pyr_down to the src image
  void
  pyr( const cv::Mat &src, cv::Mat &dst )
  {
    if( src.channels() <= 3 )
    {
      if( m_up_or_down == "up" )
      {
        pyr_up( src, dst );
      }
      else
      {
        pyr_down( src, dst );
      }
    }
    else
    {
      throw vital::invalid_data( "Image must have 1 or 3 channels but instead "
                                 "had "  + std::to_string( src.channels() ));
    }
  }

  // Pyramid upsampling
  void
  pyr_up( const cv::Mat &src, cv::Mat &dst )
  {
    cv::pyrUp( src, dst );
    for( int i=1; i < m_num_iterations; ++i)
    {
      cv::Mat temp;
      cv::pyrUp( dst, temp );
      dst = temp;
    }
  }

  // Pyramid downsampling
  void
  pyr_down( const cv::Mat &src, cv::Mat &dst )
  {
    cv::pyrDown( src, dst );
    for( int i=1; i < m_num_iterations; ++i)
    {
      cv::pyrDown( dst, dst );
    }
  }

}; // end priv class

// ================================================================

image_pyramid_process
::image_pyramid_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new image_pyramid_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


image_pyramid_process
::~image_pyramid_process()
{
}


// ----------------------------------------------------------------
void image_pyramid_process
::_configure()
{
  kwiver::vital::config_block_sptr algo_config = get_config();

  d->m_num_iterations = config_value_using_trait( num_iterations );
  d->m_up_or_down = config_value_using_trait( up_or_down );

  // Check config so it will give run-time diagnostic of config problems
  if ( d->m_up_or_down != "up" && d->m_up_or_down != "down" )
  {
    throw sprokit::invalid_configuration_exception( name(), "Parameter "
            "\'up_or_down\' must be \'up\' or \'down\'." );
  }

  if ( d->m_up_or_down == "up" && d->m_num_iterations > 1 )
  {
    throw sprokit::invalid_configuration_exception( name(), "Bug with "
            "upsampling iterations higher than 1 has not yet been fixed." );
  }
}


// ----------------------------------------------------------------
void
image_pyramid_process
::_step()
{
  d->m_timer.start();
  // -- inputs --
  kwiver::vital::image_container_sptr in_image = grab_from_port_using_trait( image );

  if ( in_image == NULL )
  {
    throw kwiver::vital::invalid_value( "Input image pointer is NULL." );
  }

  LOG_TRACE( logger(), "Received image ([" + std::to_string(in_image->width()) +
             ", " + std::to_string(in_image->height()) + ", " +
             std::to_string(in_image->depth()) + "]");

  // --------------------- Convert Input Images to OCV Format -----------------
  const cv::Mat img_ocv0 {arrows::ocv::image_container::vital_to_ocv( in_image->get_image() )};
  cv::Mat img_ocv;
  d->pyr(img_ocv0, img_ocv);

  // Convert back to an image_container_sptr and push to port
  vital::image_container_sptr img_out;
  img_out = std::make_shared<arrows::ocv::image_container>(img_ocv);

  LOG_TRACE( logger(), "Outputting image ([" + std::to_string(img_out->width()) +
             ", " + std::to_string(img_out->height()) + ", " +
             std::to_string(img_out->depth()) + "]");

  push_to_port_using_trait( image, img_out );

  // Test to see if optional port is connected.
  if( count_output_port_edges_using_trait( homography ) > 0 )
  {
    Eigen::Matrix< double, 3, 3 > eigen_homog;
    if( d->m_up_or_down == "down" )
    {
      // Want to use the inverse transform, which is pyr_up_homog
      eigen_homog = pyr_up_eigen_homog;
      for( int i=1; i < d->m_num_iterations; ++i)
      {
        eigen_homog = eigen_homog * pyr_up_eigen_homog;
      }
    }
    else
    {
      // Want to use the inverse transform, which is pyr_down_homog
      eigen_homog = pyr_down_eigen_homog;
      for( int i=1; i < d->m_num_iterations; ++i)
      {
        eigen_homog = eigen_homog * pyr_down_eigen_homog;
      }
    }

    push_to_port_using_trait( homography,
                              vital::homography_sptr(new kwiver::vital::homography_< double >( eigen_homog )) );
  }

  d->m_timer.stop();
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( logger(), "Total processing time: " << elapsed_time << " seconds");
}


// ----------------------------------------------------------------
void image_pyramid_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( image, required );

  // -- output --
  declare_output_port_using_trait( image, required );
  declare_output_port_using_trait( homography, optional );
}


// ----------------------------------------------------------------
void image_pyramid_process
::make_config()
{
  declare_config_using_trait( num_iterations );
  declare_config_using_trait( up_or_down );
}


} // end namespace
