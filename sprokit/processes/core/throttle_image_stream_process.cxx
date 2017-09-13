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

#include "throttle_image_stream_process.h"
#include <vital/types/image_container.h>
#include <vital/types/timestamp_config.h>
#include <vital/vital_types.h>
#include <vital/types/timestamp.h>

#include <sprokit/processes/kwiver_type_traits.h>
#include <sprokit/pipeline/process_exception.h>

namespace kwiver {

create_config_trait( rate, double, "1", "Desired rate (Hz) of the output image "
                     "stream." );

//----------------------------------------------------------------
// Private implementation class
class throttle_image_stream_process::priv
{
public:
  double m_tstep {1};
  double m_next_time;
  bool m_initialized {false};
  priv();
  ~priv();

}; // end priv class


// ==================================================================
throttle_image_stream_process::
throttle_image_stream_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new throttle_image_stream_process::priv )
{

  make_ports();
  make_config();
}


throttle_image_stream_process::
~throttle_image_stream_process()
{
}


// ------------------------------------------------------------------
void
throttle_image_stream_process::
_configure()
{
  double rate = config_value_using_trait( rate );

  if( rate <= 0)
  {
    throw sprokit::invalid_configuration_exception( name(), "Rate must be greater than zero." );
  }

  d->m_tstep = 1/rate;
}


// ------------------------------------------------------------------
void
throttle_image_stream_process::
_step()
{
  kwiver::vital::timestamp frame_time = grab_input_using_trait( timestamp );

  if( !d->m_initialized )
  {
    vital::image_container_sptr input = grab_from_port_using_trait( image );
    d->m_next_time = frame_time.get_time_seconds() + d->m_tstep;
    d->m_initialized = true;
    LOG_TRACE( logger(), "Initializing frame time: " <<
               frame_time.get_time_seconds() << " seconds");
    push_to_port_using_trait( image, input );
    push_to_port_using_trait( timestamp, frame_time );
    return;
  }
  else if( frame_time.get_time_seconds() >= d->m_next_time )
  {
    LOG_TRACE( logger(), "Pushing image with frame time " <<
               frame_time.get_time_seconds() << " to port");
    vital::image_container_sptr input = grab_from_port_using_trait( image );
    push_to_port_using_trait( image, input );
    push_to_port_using_trait( timestamp, frame_time );
    d->m_next_time += d->m_tstep;
    return;
  }
  else
  {
    LOG_TRACE( logger(), "Received image with frame time " +
             std::to_string(frame_time.get_time_seconds()) +
             ", but waiting for frame time >= " +
             std::to_string(d->m_next_time) );
    vital::image_container_sptr input = grab_from_port_using_trait( image );
    return;
  }
}


// ------------------------------------------------------------------
void
throttle_image_stream_process::
make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( timestamp, required );
  declare_input_port_using_trait( image, required );

  // -- output --
  declare_output_port_using_trait( timestamp, required );
  declare_output_port_using_trait( image, required );
}


// ------------------------------------------------------------------
void
throttle_image_stream_process::
make_config()
{
  declare_config_using_trait( rate );
}


// ================================================================
throttle_image_stream_process::priv
::priv()
{
}


throttle_image_stream_process::priv
::~priv()
{
}

} // end namespace kwiver
