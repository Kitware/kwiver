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

#include "interpolate_track_from_video.h"

namespace kwiver {
namespace arrows {
namespace core {

// ============================================================================
class interpolate_track_from_video::priv
{
public:

  // Video input algorithm
  kwiver::vital::algo::video_input_sptr m_video_input;

};


// ----------------------------------------------------------------------------
interpolate_track_from_video::
interpolate_track_from_video()
  : d( new priv )
{
}


// ----------------------------------------------------------------------------
interpolate_track_from_video::
~interpolate_track_from_video()
{
}



// ----------------------------------------------------------------------------
vital::config_block_sptr
interpolate_track_from_video::
get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config =
      vital::algo::interpolate_track::get_configuration();

  config->set_value("image_source", "",
                    "Algorithm to supply images for the interpolation algorithm" );

  // Other config parameters as needed

}

// ----------------------------------------------------------------------------
void
interpolate_track_from_video::
set_configuration(vital::config_block_sptr in_config)
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  // Check config so it will give run-time diagnostic of config problems
  if ( ! vital::algo::video_input::check_nested_algo_configuration( "image_source", config ) )
  {
    throw sprokit::invalid_configuration_exception( name(), "Configuration check failed." );
  }

  vital::algo::video_input::set_nested_algo_configuration( "image_source", config, m_video_input );
  if ( ! d->m_video_input )
  {
    throw sprokit::invalid_configuration_exception( name(), "Unable to create algorithm" );
  }

  // Get other parameters as necessary
  // need video input name string (e.g. file name, or directory name )
}


// ----------------------------------------------------------------------------
bool
interpolate_track_from_video::
check_configuration(vital::config_block_sptr config) const
{
  // Check the reader configuration.
  bool status = vital::algo::image_io::check_nested_algo_configuration( "image_source", config );

  // other config parameters as needed

  return status;
}


// ----------------------------------------------------------------------------
object_track&
interpolate_track_from_video::
interpolate( const object_track& init_states )
{
  try
  {
    // open video_input
    m_video_input->open( <name> );
  }
  catch ( const kwiver::vital::vital_core_base_exception& e )
  {
    // Can throw multiple exceptions
    // handle error
  }

  // optionally check capabilities

  // skip forward to starting frame
  while( xxx )
  {
    m_video_input->next_frame();
  }

  // do interpolation here


  m_video_input->close();
}


} } } // end namespace
