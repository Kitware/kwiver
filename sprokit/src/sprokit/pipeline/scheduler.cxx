/*ckwg +29
 * Copyright 2011-2017 by Kitware, Inc.
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

#include "scheduler.h"
#include "scheduler_exception.h"

#include "pipeline.h"

#include <boost/thread/locks.hpp>
#ifndef BOOST_NO_HAVE_REVERSE_LOCK
#include <boost/thread/reverse_lock.hpp>
#endif
#include <boost/thread/shared_mutex.hpp>

/**
 * \file scheduler.cxx
 *
 * \brief Implementation of the base class for \link sprokit::scheduler schedulers\endlink.
 */

namespace sprokit {

class scheduler::priv
{
  public:
    priv(scheduler* sched, pipeline_t const& pipe);
    ~priv();

    void stop();

    scheduler* const q;
    pipeline_t const p;
    bool paused;
    bool running;

    typedef boost::shared_mutex mutex_t;
    typedef boost::upgrade_lock<mutex_t> upgrade_lock_t;
    typedef boost::unique_lock<mutex_t> unique_lock_t;
#ifndef BOOST_NO_HAVE_REVERSE_LOCK
    typedef boost::reverse_lock<unique_lock_t> reverse_unique_lock_t;
#endif
    typedef boost::upgrade_to_unique_lock<mutex_t> upgrade_to_unique_lock_t;

    mutex_t mut;
};


// ============================================================================
scheduler
::~scheduler()
{
}


scheduler
::scheduler(pipeline_t const& pipe, kwiver::vital::config_block_sptr const& config)
  : m_logger( kwiver::vital::get_logger( "scheduler.base" ) )
  , d()
{
  if (!config)
  {
    throw null_scheduler_config_exception();
  }

  if (!pipe)
  {
    throw null_scheduler_pipeline_exception();
  }

  d.reset(new priv(this, pipe));
}


// ----------------------------------------------------------------------------
void
scheduler
::start()
{
  priv::unique_lock_t const lock(d->mut);

  (void)lock;

  if (d->running)
  {
    throw restart_scheduler_exception();
  }

  d->p->start();

  _start();

  d->running = true;
}


// ----------------------------------------------------------------------------
void
scheduler
::wait()
{
  priv::unique_lock_t lock(d->mut);

  if (!d->running)
  {
    throw wait_before_start_exception();
  }

  // Allow many threads to wait on the scheduler.
  {
#ifndef BOOST_NO_HAVE_REVERSE_LOCK
    priv::reverse_unique_lock_t const rev_lock(lock);

    (void)rev_lock;
#else
    lock.unlock();
#endif

    _wait();

#ifdef BOOST_NO_HAVE_REVERSE_LOCK
    lock.lock();
#endif
  }

  // After each thread, only one should call stop. Let threads through
  // one-at-a-time to see if the pipeline needs to be stopped yet.
  if (d->running)
  {
    d->stop();
  }
}


// ----------------------------------------------------------------------------
void
scheduler
::pause()
{
  priv::upgrade_lock_t lock(d->mut);

  if (!d->running)
  {
    throw pause_before_start_exception();
  }

  priv::upgrade_to_unique_lock_t const write_lock(lock);

  (void)write_lock;

  if (d->paused)
  {
    throw repause_scheduler_exception();
  }

  _pause();

  d->paused = true;
}


// ----------------------------------------------------------------------------
void
scheduler
::resume()
{
  priv::unique_lock_t const lock(d->mut);

  (void)lock;

  if (!d->running)
  {
    throw resume_before_start_exception();
  }

  if (!d->paused)
  {
    throw resume_unpaused_scheduler_exception();
  }

  _resume();

  d->paused = false;
}


// ----------------------------------------------------------------------------
void
scheduler
::stop()
{
  priv::unique_lock_t const lock(d->mut);

  (void)lock;

  if (!d->running)
  {
    throw stop_before_start_exception();
  }

  d->stop();
}


// ----------------------------------------------------------------------------
void
scheduler
::shutdown()
{
  priv::unique_lock_t const lock(d->mut);

  (void)lock;

  if (d->running)
  {
    d->stop();
  }
}


// ----------------------------------------------------------------------------
pipeline_t
scheduler
::pipeline() const
{
  return d->p;
}


// ----------------------------------------------------------------------------
kwiver::vital::logger_handle_t
scheduler
::logger()
{
  return m_logger;
}


// ============================================================================
scheduler::priv
::priv(scheduler* sched, pipeline_t const& pipe)
  : q(sched)
  , p(pipe)
  , paused(false)
  , running(false)
  , mut()
{
}

scheduler::priv
::~priv()
{
}


// ----------------------------------------------------------------------------
void
scheduler::priv
::stop()
{
  // Tell the subclass that we want to stop.
  q->_stop();

  // Unpause the scheduler.
  if (paused)
  {
    q->_resume();

    paused = false;
  }

  // Stop the pipeline.
  p->stop();
  running = false;
}

}
