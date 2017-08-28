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

#include "image_warp_process.h"

#include <vital/algo/warp_image.h>
#include <vital/util/wall_timer.h>

#include <sprokit/processes/kwiver_type_traits.h>
#include <sprokit/pipeline/process_exception.h>

namespace kwiver {

create_config_trait( height, int, "-1", "Width of the warped image (defaults to "
                     "input image height)" );
create_config_trait( width, int, "-1", "Height of the warped image (defaults to "
                     "input image width)" );
create_config_trait( algo, std::string, "", "Algorithm configuration subblock" );

//----------------------------------------------------------------
// Private implementation class
class image_warp_process::priv
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

  vital::algo::warp_image_sptr m_algo;
  kwiver::vital::wall_timer m_timer;

}; // end priv class


// ==================================================================
image_warp_process::
image_warp_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new image_warp_process::priv )
{
  // Attach our logger name to process logger
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


image_warp_process::
~image_warp_process()
{
}


// ------------------------------------------------------------------
void
image_warp_process::
_configure()
{
  vital::config_block_sptr algo_config = get_config();
  
  d->m_height          = config_value_using_trait( height );
  d->m_width           = config_value_using_trait( width );

  // Check config so it will give run-time diagnostic of config problems
  if ( ! vital::algo::warp_image::check_nested_algo_configuration_using_trait( algo, algo_config ) )
  {
    throw sprokit::invalid_configuration_exception( name(), "Configuration check failed." );
  }

  vital::algo::warp_image::set_nested_algo_configuration_using_trait( algo, algo_config, d->m_algo );

  if ( ! d->m_algo )
  {
    throw sprokit::invalid_configuration_exception( name(), "Unable to create warping algorithm" );
  }

  vital::algo::warp_image::get_nested_algo_configuration_using_trait( algo, algo_config, d->m_algo );

}


// ------------------------------------------------------------------
void
image_warp_process::
_step()
{
  LOG_TRACE( logger(), "Starting process");
  d->m_timer.start();
  
  // -- inputs --
  auto input = grab_from_port_using_trait( image );
  auto homog = grab_from_port_using_trait( homography );
  
  // -- outputs --
  kwiver::vital::image_container_sptr result;
  
  // create empty image of desired size
  int height = d->m_height;
  int width = d->m_width;
  
  //  if height or width not provided, use that of the source image
  if( height == -1 )
  {
    height = input->height();
  }
  if( width == -1 )
  {
    width = input->width();
  }
  vital::image im( width, height );
  
  // get pointer to new image container.
  result = std::make_shared<kwiver::vital::simple_image_container>( im );
  
  d->m_algo->warp( input, result, homog );

  LOG_TRACE( logger(), "About to push to port");
  push_to_port_using_trait( image, result );
  LOG_TRACE( logger(), "Pushed to port");
  
  d->m_timer.stop();
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( logger(), "Total processing time: " << elapsed_time << " seconds");
}


// ------------------------------------------------------------------
void
image_warp_process::
make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( image, required );
  declare_input_port_using_trait( homography, required );

  // -- output --
  declare_output_port_using_trait( image, optional );
}


// ------------------------------------------------------------------
void
image_warp_process::
make_config()
{
  declare_config_using_trait( height );
  declare_config_using_trait( width );
  declare_config_using_trait( algo );
}
// ================================================================

} // end namespace kwiver
