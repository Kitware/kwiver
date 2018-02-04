/*ckwg +29
 * Copyright 2011-2018 by Kitware, Inc.
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

#ifndef SPROKIT_PYTHON_UTIL_PYTHON_GIL_H
#define SPROKIT_PYTHON_UTIL_PYTHON_GIL_H

#include <sprokit/python/util/sprokit_python_util_export.h>

#include <vital/noncopyable.h>

#include <sprokit/python/util/python.h>
#include <sprokit/python/util/python_exceptions.h>

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>

#include <memory>

/**
 * \file python_gil.h
 *
 * \brief Helper utilities for grabbing the Python GIL.
 */

namespace sprokit {
namespace python {

/**
 * \class python_gil_cond_release python_gil.h <sprokit/python/util/python_gil.h>
 *
 * \brief Releases the python gil if held by this thread for the duration this
 * class is in scope, but only if this is a valid python thread otherwise this
 * is a no-op.
 */
class SPROKIT_PYTHON_UTIL_EXPORT python_gil_cond_release
  : private kwiver::vital::noncopyable
{
  public:
    /**
     * \brief Constructor.
     */
    python_gil_cond_release();
    /**
     * \brief Destructor.
     */
    ~python_gil_cond_release();

  private:
    std::unique_ptr< pybind11::gil_scoped_release > rel;
};

/**
 * \brief Helper macros for warningless pybind11 GIL scoped acquisitions
 */
#define SPROKIT_SCOPED_GIL_ACQUIRE_START                                          \
  {                                                                               \
    pybind11::gil_scoped_acquire acquire;                                         \
                                                                                  \
    (void) acquire;

#define SPROKIT_SCOPED_GIL_ACQUIRE_END                                            \
  }


#define SPROKIT_SCOPED_GIL_RELEASE_AND_ACQUIRE_START                              \
  {                                                                               \
    pybind11::gil_scoped_release release;                                         \
    {                                                                             \
      pybind11::gil_scoped_acquire acquire;                                       \
                                                                                  \
      (void) release;                                                             \
      (void) acquire;

#define SPROKIT_SCOPED_GIL_RELEASE_AND_ACQUIRE_END                                \
    }                                                                             \
  }

/**
 * \brief Helper function, determines if the current thread is likely a pythread
 * 
 * Note: when false the thread is definitly not a pythread, when true the thread
 * is likely a pythread or a c-thread which has had pythread information assigned
 * to it with a very high probability, but not 100% guaranteed. 
 */
#define SPROKIT_IS_CURRENT_PYTHREAD                                               \
  ( pybind11::detail::get_thread_state_unchecked() != NULL  &&                    \
    pybind11::detail::get_thread_state_unchecked() ==                             \
      PyGILState_GetThisThreadState() )

/**
 * \brief Grabs the Python GIL using pybind11 after releasing it, but only if we're
 * in a pythread. If we're not in a pythread, the lock is acquired without release.
 * 
 * Implicit conversions from python to C++ exceptions is also performed.
 */
#define SPROKIT_COND_GIL_RELEASE_AND_ACQUIRE( call, use_rel_and_acq )             \
  if( use_rel_and_acq && SPROKIT_IS_CURRENT_PYTHREAD )                            \
  {                                                                               \
    SPROKIT_SCOPED_GIL_RELEASE_AND_ACQUIRE_START                                  \
    SPROKIT_PYTHON_TRANSLATE_EXCEPTION_NO_LOCK( call )                            \
    SPROKIT_SCOPED_GIL_RELEASE_AND_ACQUIRE_END                                    \
  }                                                                               \
  else                                                                            \
  {                                                                               \
    SPROKIT_SCOPED_GIL_ACQUIRE_START                                              \
    SPROKIT_PYTHON_TRANSLATE_EXCEPTION_NO_LOCK( call )                            \
    SPROKIT_SCOPED_GIL_ACQUIRE_END                                                \
  }

}
}

#endif // SPROKIT_PYTHON_UTIL_PYTHON_GIL_H
