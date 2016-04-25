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

#include "output_adapter_process.h"

#include <vital/vital_foreach.h>

#include <stdexcept>
#include <sstream>

namespace kwiver {

// ------------------------------------------------------------------
output_adapter_process
::output_adapter_process( kwiver::vital::config_block_sptr const& config )
  : process( config )
{
  // Attach our logger name to process logger
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach
}


output_adapter_process
::~output_adapter_process()
{ }


// ------------------------------------------------------------------
  kwiver::adapter::ports_info_t
output_adapter_process
::get_ports()
{
  kwiver::adapter::ports_info_t port_info;

  // formulate list of current output ports
  sprokit::process::ports_t ports = this->output_ports();
  VITAL_FOREACH( auto port, ports )
  {
    port_info[port] = this->output_port_info( port );
  }

  return port_info;
}


// ------------------------------------------------------------------
sprokit::process::port_info_t
output_adapter_process
::_output_port_info(port_t const& port)
{
  // If we have not created the port, then make a new one.
  if ( m_active_ports.count( port ) == 0 )
  {
    port_flags_t required;
    required.insert(flag_required);

    // create a new port
    declare_output_port( port, // port name
                        type_any, // port type
                        required,
                        port_description_t("Output for " + port)
      );

    // Add to our list of existing ports
    m_active_ports.insert( port );
  }

  return process::_output_port_info(port);
}


// ------------------------------------------------------------------
void
output_adapter_process
::_configure()
{
  // handle config items here

}


// ------------------------------------------------------------------
void
output_adapter_process
::_init()
{
  // post connection initialization

}


// ------------------------------------------------------------------
void
output_adapter_process
::_step()
{
  auto data_set = kwiver::adapter::adapter_data_set::create();

  // The grab call is blocking, so it will wait until data is there.
  VITAL_FOREACH( auto p, m_active_ports )
  {
    data_set->add( p, this->grab_datum_from_port( p ) );
  }

  // Possible option to see if queue is full and handle this set differently

  // Send received data to consumer thread
  this->get_interface_queue()->Send( data_set );

  return;
}

} // end namespace
