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

#include "stabilize_video_process.h"

#include <vital/vital_types.h>
#include <vital/types/timestamp.h>
#include <vital/types/timestamp_config.h>
#include <vital/types/image_container.h>
#include <vital/algo/stabilize_video.h>
#include <vital/algo/warp_image.h>
#include <arrows/ocv/image_container.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace algo = kwiver::vital::algo;

namespace kwiver {

// ==================================================
// TBD A better approach would be to remove the warp algorithm reference
// in this process and do the warping in the pipeline.

create_config_trait( stabilize, std::string, "", "Stabilization algorithm configuration subblock" );
create_config_trait( warp, std::string, "", "Warping algorithm configuration subblock" );

//----------------------------------------------------------------
// Private implementation class
class stabilize_video_process::priv
{
public:
  priv();
  ~priv();


  // Configuration values
  algo::stabilize_video_sptr m_stabilize;
  algo::warp_image_sptr m_warp;

}; // end priv class

// ================================================================

stabilize_video_process
::stabilize_video_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new stabilize_video_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


stabilize_video_process
::~stabilize_video_process()
{
}


// ----------------------------------------------------------------
void stabilize_video_process
::_configure()
{
  kwiver::vital::config_block_sptr algo_config = get_config();

  // Check config so it will give run-time diagnostic of config problems
  if ( ! algo::stabilize_video::check_nested_algo_configuration( "stabilize", algo_config ) )
  {
    throw sprokit::invalid_configuration_exception( name(), "Configuration check failed." );
  }

  algo::stabilize_video::set_nested_algo_configuration( "stabilize", algo_config, d->m_stabilize );
  if ( ! d->m_stabilize )
  {
    throw sprokit::invalid_configuration_exception( name(), "Unable to create stabilization algorithm" );
  }

  // Check config so it will give run-time diagnostic of config problems
  if ( ! algo::warp_image::check_nested_algo_configuration( "warp", algo_config ) )
  {
    throw sprokit::invalid_configuration_exception( name(), "Configuration check failed." );
  }

  algo::warp_image::set_nested_algo_configuration( "warp", algo_config, d->m_warp );
  if ( ! d->m_stabilize )
  {
    throw sprokit::invalid_configuration_exception( name(), "Unable to create image warping algorithm" );
  }
}


// ----------------------------------------------------------------
void
stabilize_video_process
::_step()
{
  kwiver::vital::homography_f2f_sptr src_to_ref_homography;

  // timestamp
  kwiver::vital::timestamp frame_time = grab_from_port_using_trait( timestamp );

  // image
  kwiver::vital::image_container_sptr in_image = grab_from_port_using_trait( image );

  // LOG_DEBUG - this is a good thing to have in all processes that handle frames.
  LOG_DEBUG( logger(), "Processing frame " << frame_time );

  // -- outputs --
  kwiver::vital::homography_f2f_sptr s2r_homog;
  bool new_ref;
  kwiver::vital::image_container_sptr stab_image;
  
  // create empty image of desired size
  vital::image im( in_image->width() - 50, in_image->height() - 50);
  
  // get pointer to new image container.
  stab_image = std::make_shared<kwiver::arrows::ocv::image_container>( im );

  d->m_stabilize->process_image( frame_time, in_image,
                                 s2r_homog, new_ref);

  d->m_warp->warp( in_image, stab_image, s2r_homog->homography() );

  // return by value
  push_to_port_using_trait( homography_src_to_ref, s2r_homog );
  push_to_port_using_trait( image, stab_image );
  push_to_port_using_trait( coordinate_system_updated, new_ref );
}


// ----------------------------------------------------------------
void stabilize_video_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( timestamp, required );
  declare_input_port_using_trait( image, required );

  // -- output --
  declare_output_port_using_trait( homography_src_to_ref, optional );
  declare_output_port_using_trait( image, optional );
  declare_output_port_using_trait( coordinate_system_updated, optional );
}


// ----------------------------------------------------------------
void stabilize_video_process
::make_config()
{
  declare_config_using_trait( stabilize );
  declare_config_using_trait( warp );
}


// ================================================================
stabilize_video_process::priv
::priv()
{
}


stabilize_video_process::priv
::~priv()
{
}

} // end namespace
