/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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

#include "warp_image_process.h"

#include <vital/vital_types.h>
#include <vital/types/timestamp.h>
#include <vital/types//homography.h>
#include <vital/types//homography_f2f.h>
#include <vital/types/timestamp_config.h>
#include <vital/types/image_container.h>
#include <vital/algo/stabilize_video.h>
#include <vital/algo/warp_image.h>
#include <vital/util/wall_timer.h>
#include <arrows/ocv/image_container.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace algo = kwiver::vital::algo;

namespace kwiver {

// ==================================================

create_config_trait( height, int, "-1", "Width of the warped image (defaults to "
                     "input image height)" );
create_config_trait( width, int, "-1", "Height of the warped image (defaults to "
                     "input image width)" );
create_config_trait( warp, std::string, "", "Warping algorithm configuration subblock" );

//----------------------------------------------------------------
// Private implementation class
class warp_image_process::priv
{
public:
  priv()
    : m_height(-1),
      m_width(-1)
  {
  };
  ~priv()
  {
  };

  // Configuration values
  int m_height;
  int m_width;
  algo::warp_image_sptr m_warp;
  kwiver::vital::wall_timer m_timer;

}; // end priv class

// ================================================================

warp_image_process
::warp_image_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new warp_image_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


warp_image_process
::~warp_image_process()
{
}


// ----------------------------------------------------------------
void warp_image_process
::_configure()
{
  kwiver::vital::config_block_sptr algo_config = get_config();

  d->m_height          = config_value_using_trait( height );
  d->m_width           = config_value_using_trait( width );
  
  // Check config so it will give run-time diagnostic of config problems
  if ( ! algo::warp_image::check_nested_algo_configuration( "algo", algo_config ) )
  {
    throw sprokit::invalid_configuration_exception( name(), "Configuration check failed." );
  }

  algo::warp_image::set_nested_algo_configuration( "algo", algo_config, d->m_warp );
  if ( ! d->m_warp )
  {
    throw sprokit::invalid_configuration_exception( name(), "Unable to create image warping algorithm" );
  }
}


// ----------------------------------------------------------------
void
warp_image_process
::_step()
{
  LOG_TRACE( logger(), "Starting process");
  
  d->m_timer.start();
  
  // input homography
  kwiver::vital::homography_f2f_sptr s2r_homog = grab_from_port_using_trait( homography_src_to_ref );
  kwiver::vital::homography_sptr homog = s2r_homog->homography();
  
  // input image
  kwiver::vital::image_container_sptr in_image = grab_from_port_using_trait( image );

  // -- outputs --
  kwiver::vital::image_container_sptr warped_image;
  
  // create empty image of desired size
  int height = d->m_height;
  int width = d->m_width;
  
  //  If height or width not provided, use that of the source image
  if( height == -1 )
  {
    height = in_image->height();
  }
  if( width == -1 )
  {
    width = in_image->width();
  }
  vital::image im( width, height );
  
  // get pointer to new image container.
  warped_image = std::make_shared<kwiver::vital::simple_image_container>( im );
  
  d->m_warp->warp( in_image, warped_image, homog );
  
  LOG_TRACE( logger(), "About to push to port");
  push_to_port_using_trait( image, warped_image );
  LOG_TRACE( logger(), "Pushed to port");
  
  d->m_timer.stop();
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( logger(), "Total processing time: " << elapsed_time << " seconds");
}


// ----------------------------------------------------------------
void warp_image_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( homography_src_to_ref, optional );
  declare_input_port_using_trait( image, required );

  // -- output --
  declare_output_port_using_trait( image, optional );
}


// ----------------------------------------------------------------
void warp_image_process
::make_config()
{
  declare_config_using_trait( warp );
  declare_config_using_trait( height );
  declare_config_using_trait( width );
}

} // end namespace kwiver
