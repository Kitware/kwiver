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

#include "rt_process_instrumentation.h"

#include <sprokit/pipeline/process.h>
#include <cstdlib>

namespace sprokit {

#define INIT_COLOR -1
#define RESET_COLOR -1
#define FLUSH_COLOR -1
#define STEP_COLOR -1
#define CONFIGURE_COLOR -1
#define RECONFIGURE_COLOR -1


// ------------------------------------------------------------------
rt_process_instrumentation::
rt_process_instrumentation()
{
}


// ------------------------------------------------------------------
rt_process_instrumentation::
~rt_process_instrumentation()
{ }


// ------------------------------------------------------------------
void
rt_process_instrumentation::
configure( kwiver::vital::config_block_sptr const conf )
{
  // Starting with our generated vital::config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  auto local_config = get_configuration();
  local_config->merge_config( conf );

  char* end;
  std::string rgb_str = local_config->get_value<std::string>("init_color", INIT_COLOR );
  int rgb = strtol( rgb_str.c_str(), &end, 10 );
  m_init_event.reset( new RightTrack::BoundedEvent( process()->name() + ".init",
                                                    process()->name(), rgb ) );

  rgb_str = local_config->get_value<std::string>("reset_color", RESET_COLOR );
  rgb = strtol( rgb_str.c_str(), &end, 10 );
  m_reset_event.reset( new RightTrack::BoundedEvent( process()->name() + ".reset",
                                                     process()->name(), rgb ) );

  rgb_str = local_config->get_value<std::string>("flush_color", FLUSH_COLOR );
  rgb = strtol( rgb_str.c_str(), &end, 10 );
  m_flush_event.reset( new RightTrack::BoundedEvent( process()->name() + ".flush",
                                                     process()->name(), rgb ) );

  rgb_str = local_config->get_value<std::string>("step_color", STEP_COLOR );
  rgb = strtol( rgb_str.c_str(), &end, 10 );
  m_step_event.reset( new RightTrack::BoundedEvent( process()->name()+ ".step",
                                                    process()->name(), rgb ) );

  rgb_str = local_config->get_value<std::string>("configure_color", CONFIGURE_COLOR );
  rgb = strtol( rgb_str.c_str(), &end, 10 );
  m_configure_event.reset( new RightTrack::BoundedEvent( process()->name() + ".configure",
                                                         process()->name(), rgb ) );

  rgb_str = local_config->get_value<std::string>("reconfigure_color", RECONFIGURE_COLOR );
  rgb = strtol( rgb_str.c_str(), &end, 10 );
  m_reconfigure_event.reset( new RightTrack::BoundedEvent( process()->name() + ".reconfigure",
                                                           process()->name(), rgb ) );
}


// ------------------------------------------------------------------
kwiver::vital::config_block_sptr
rt_process_instrumentation::
get_configuration() const
{
  auto conf = kwiver::vital::config_block::empty_config();

  conf->set_value( "init_color",        -1, "Color for the init event display. Color is specified as 0x00rrggbb." );
  conf->set_value( "reset_color",       -1, "Color for the reset event display. Color is specified as 0x00rrggbb." );
  conf->set_value( "flush_color",       -1, "Color for the flush event display. Color is specified as 0x00rrggbb." );
  conf->set_value( "step_color",        -1, "Color for the step event display. Color is specified as 0x00rrggbb." );
  conf->set_value( "configure_color",   -1, "Color for the configure event display. Color is specified as 0x00rrggbb." );
  conf->set_value( "reconfigure_color", -1, "Color for the reconfigure event display. Color is specified as 0x00rrggbb." );

  return conf;
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
start_init_processing( std::string const& data )
{
  m_init_event->Start( /* data */ );
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
stop_init_processing()
{
  m_init_event->End();
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
start_reset_processing( std::string const& data )
{
  m_reset_event->Start( /* data */ );
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
stop_reset_processing()
{
  m_reset_event->End();
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
start_flush_processing( std::string const& data )
{
  m_flush_event->Start( /* data */ );
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
stop_flush_processing()
{
  m_flush_event->End();
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
start_step_processing( std::string const& data )
{
  m_step_event->Start( /* data */ );
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
stop_step_processing()
{
  m_step_event->End();
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
start_configure_processing( std::string const& data )
{
  m_configure_event->Start( /* data */ );
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
stop_configure_processing()
{
  m_configure_event->End();
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
start_reconfigure_processing( std::string const& data )
{
  m_reconfigure_event->Start( /* data */ );
}


// ------------------------------------------------------------------
void
rt_process_instrumentation::
stop_reconfigure_processing()
{
  m_reconfigure_event->End();
}

} // end namespace
