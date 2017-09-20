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

#include <sprokit/pipeline/stamp.h>

/**
 * \file PyStamp.cxx
 *
 * \brief Python helper class for \link sprokit::stamp\endlink.
 */

// We need to use this because PyBind11 has weird interactions with pointers
class PyStamp
{
  public:

    PyStamp(sprokit::stamp_t st) {stamp_ptr = st;};

    sprokit::stamp_t stamp_ptr;
    sprokit::stamp_t get_stamp() const {return stamp_ptr;};
};

static PyStamp new_stamp(sprokit::stamp::increment_t const& increment);
static PyStamp incremented_stamp(PyStamp const& st);
static bool stamp_eq(PyStamp const& self, PyStamp const& other);
static bool stamp_lt(PyStamp const& self, PyStamp const& other);

PyStamp
new_stamp(sprokit::stamp::increment_t const& increment)
{
  sprokit::stamp_t st = sprokit::stamp::new_stamp(increment);
  return PyStamp(st);
}

PyStamp
incremented_stamp(PyStamp const& st)
{
  sprokit::stamp_t st_inc = sprokit::stamp::incremented_stamp(st.get_stamp());
  return PyStamp(st_inc);
}

bool
stamp_eq(PyStamp const& self, PyStamp const& other)
{
  return (*(self.get_stamp()) == *(other.get_stamp()));
}

bool
stamp_lt(PyStamp const& self, PyStamp const& other)
{
  return (*(self.get_stamp()) < *(other.get_stamp()));
}
