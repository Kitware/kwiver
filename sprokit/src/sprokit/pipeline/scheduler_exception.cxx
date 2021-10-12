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

#include "scheduler_exception.h"

#include <sstream>

/**
 * \file scheduler_exception.cxx
 *
 * \brief Implementation of exceptions used within \link sprokit::scheduler schedulers\endlink.
 */

namespace sprokit
{

scheduler_exception
::scheduler_exception() noexcept
  : pipeline_exception()
{
}

scheduler_exception
::~scheduler_exception() noexcept
{
}

incompatible_pipeline_exception
::incompatible_pipeline_exception(std::string const& reason) noexcept
  : scheduler_exception()
  , m_reason(reason)
{
  std::ostringstream sstr;

  sstr << "The pipeline cannot be run: " << m_reason;

  m_what = sstr.str();
}

incompatible_pipeline_exception
::~incompatible_pipeline_exception() noexcept
{
}

null_scheduler_config_exception
::null_scheduler_config_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A NULL configuration was passed to a scheduler";

  m_what = sstr.str();
}

null_scheduler_config_exception
::~null_scheduler_config_exception() noexcept
{
}

null_scheduler_pipeline_exception
::null_scheduler_pipeline_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A NULL pipeline was passed to a scheduler";

  m_what = sstr.str();
}

null_scheduler_pipeline_exception
::~null_scheduler_pipeline_exception() noexcept
{
}

restart_scheduler_exception
::restart_scheduler_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A scheduler was restarted";

  m_what = sstr.str();
}

restart_scheduler_exception
::~restart_scheduler_exception() noexcept
{
}

wait_before_start_exception
::wait_before_start_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A scheduler was waited on before it was started";

  m_what = sstr.str();
}

wait_before_start_exception
::~wait_before_start_exception() noexcept
{
}

pause_before_start_exception
::pause_before_start_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A scheduler was paused before it was started";

  m_what = sstr.str();
}

pause_before_start_exception
::~pause_before_start_exception() noexcept
{
}

repause_scheduler_exception
::repause_scheduler_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A paused scheduler was paused";

  m_what = sstr.str();
}

repause_scheduler_exception
::~repause_scheduler_exception() noexcept
{
}

resume_before_start_exception
::resume_before_start_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A scheduler was resumed before it was started";

  m_what = sstr.str();
}

resume_before_start_exception
::~resume_before_start_exception() noexcept
{
}

resume_unpaused_scheduler_exception
::resume_unpaused_scheduler_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "An unpaused scheduler was resumed";

  m_what = sstr.str();
}

resume_unpaused_scheduler_exception
::~resume_unpaused_scheduler_exception() noexcept
{
}

stop_before_start_exception
::stop_before_start_exception() noexcept
  : scheduler_exception()
{
  std::ostringstream sstr;

  sstr << "A scheduler was stopped before it was started";

  m_what = sstr.str();
}

stop_before_start_exception
::~stop_before_start_exception() noexcept
{
}

}
