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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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

/**
 * \file tpp_scheduler_trace.cxx
 *
 * \brief Implementation of the thread-per-process scheduler.
 */

#include "tpp_scheduler_trace.h"

#include <vital/config/config_block.h>
#include <vital/vital_foreach.h>
#include <vital/logger/logger.h>

#include <sprokit/pipeline/datum.h>
#include <sprokit/pipeline/edge.h>
#include <sprokit/pipeline/pipeline.h>
#include <sprokit/pipeline/scheduler_exception.h>
#include <sprokit/pipeline/utils.h>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/make_shared.hpp>

namespace sprokit {

class tpp_scheduler_trace::priv
{
public:
  priv();
  ~priv();

  void run_process( process_t const& process );

  boost::scoped_ptr< boost::thread_group > process_threads;

  typedef boost::shared_mutex mutex_t;
  typedef boost::shared_lock< mutex_t > shared_lock_t;

  mutable mutex_t m_pause_mutex;

  kwiver::vital::logger_handle_t m_logger;
};


// ------------------------------------------------------------------
tpp_scheduler_trace
::tpp_scheduler_trace( pipeline_t const& pipe, kwiver::vital::config_block_sptr const& config )
  : scheduler( pipe, config )
  , d( new priv )
{
  pipeline_t const p = pipeline();
  process::names_t const names = p->process_names();

  VITAL_FOREACH( process::name_t const & name, names )
  {
    process_t const proc = p->process_by_name( name );
    process::properties_t const consts = proc->properties();

    if ( consts.count( process::property_no_threads ) )
    {
      std::string const reason = "The process \'" + name + "\' does "
                                                           "not support being in its own thread";

      throw incompatible_pipeline_exception( reason );
    }
  }
}


// ------------------------------------------------------------------
tpp_scheduler_trace
::~tpp_scheduler_trace()
{
  shutdown();
}


// ------------------------------------------------------------------
void
tpp_scheduler_trace
::_start()
{
  pipeline_t const p = pipeline();
  process::names_t const names = p->process_names();

  d->process_threads.reset( new boost::thread_group );

  VITAL_FOREACH( process::name_t const & name, names )
  {
    process_t const process = pipeline()->process_by_name( name );

    d->process_threads->create_thread( boost::bind( &priv::run_process, d.get(), process ) );
  }
}


// ------------------------------------------------------------------
void
tpp_scheduler_trace
::_wait()
{
  d->process_threads->join_all();
}


// ------------------------------------------------------------------
void
tpp_scheduler_trace
::_pause()
{
  d->m_pause_mutex.lock();
}


// ------------------------------------------------------------------
void
tpp_scheduler_trace
::_resume()
{
  d->m_pause_mutex.unlock();
}


// ------------------------------------------------------------------
void
tpp_scheduler_trace
::_stop()
{
  d->process_threads->interrupt_all();
}


// ------------------------------------------------------------------
tpp_scheduler_trace::priv
::priv()
  : process_threads()
  , m_pause_mutex()
  , m_logger( kwiver::vital::get_logger( "tpp_scheduler_trace" ) )
{
}


// ------------------------------------------------------------------
tpp_scheduler_trace::priv
::~priv()
{
}


static kwiver::vital::config_block_sptr monitor_edge_config();


// ------------------------------------------------------------------
void
tpp_scheduler_trace::priv
::run_process( process_t const & process )
{
  kwiver::vital::config_block_sptr const edge_conf = monitor_edge_config();

  name_thread( process->name() );
  edge_t monitor_edge = boost::make_shared< edge > ( edge_conf );

  process->connect_output_port( process::port_heartbeat, monitor_edge );

  bool complete = false;

  while ( ! complete )
  {
    // This locking will cause this thread to pause if the scheduler
    // pause() method is called.
    shared_lock_t const lock( m_pause_mutex );

    (void)lock;

    // This call allows an exception to be thrown (boost::thread_interrupted)
    // Since this exception is not caught, it causes the thread to terminate.
    boost::this_thread::interruption_point();

    LOG_INFO( m_logger, "Calling step() for process: " << process->name() );

    process->step();

    LOG_INFO( m_logger, "step() returned for process: " << process->name() );

    while ( monitor_edge->has_data() )
    {
      edge_datum_t const edat = monitor_edge->get_datum();
      datum_t const dat = edat.datum;

      if ( dat->type() == datum::complete )
      {
        complete = true;
        LOG_INFO( m_logger, "process: " << process->name() << " has completed" );
      }
    } // end while has_data
  } // while ! complete
}


// ------------------------------------------------------------------
kwiver::vital::config_block_sptr
monitor_edge_config()
{
  kwiver::vital::config_block_sptr conf = kwiver::vital::config_block::empty_config();

  return conf;
}


}
