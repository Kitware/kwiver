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

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>

/**
 * \file python_gil.h
 *
 * \brief Helper utilities for grabbing the Python GIL.
 */

namespace sprokit {
namespace python {

/**
 * \class python_gil python_gil.h <sprokit/python/util/python_gil.h>
 *
 * \brief Deprecated - Grabs the Python GIL and uses RAII to ensure it is released.
 */
class SPROKIT_PYTHON_UTIL_EXPORT python_gil
  : private kwiver::vital::noncopyable
{
  public:
    /**
     * \brief Constructor.
     */
    python_gil();
    /**
     * \brief Destructor.
     */
    ~python_gil();
  private:
    PyGILState_STATE const state;
};

/**
 * \brief Grabs the Python GIL using pybind11 after releasing it, but only if we're
 * in a pythread. If we're not in a pythread, the lock is acquired without release.
 */
#define PYBIND_COND_GIL_RELEASE_AND_ACQUIRE( ACTION_TO_PERFORM, EXTRA_CHECK )     \
  if( EXTRA_CHECK && pybind11::detail::get_thread_state_unchecked() != NULL )     \
  {                                                                               \
    pybind11::gil_scoped_release release;                                         \
    {                                                                             \
      pybind11::gil_scoped_acquire acquire;                                       \
                                                                                  \
      (void) release;                                                             \
      (void) acquire;                                                             \
                                                                                  \
      ACTION_TO_PERFORM                                                           \
    }                                                                             \
  }                                                                               \
  else                                                                            \
  {                                                                               \
    pybind11::gil_scoped_acquire acquire;                                         \
                                                                                  \
    (void) acquire;                                                               \
                                                                                  \
    ACTION_TO_PERFORM                                                             \
  }

#define PYBIND_ALWAYS_GIL_RELEASE_AND_ACQUIRE( ACTION_TO_PERFORM )                \
  PYBIND_ALWAYS_GIL_RELEASE_AND_ACQUIRE( ACTION_TO_PERFORM, TRUE )

}
}

#endif // SPROKIT_PYTHON_UTIL_PYTHON_GIL_H
