/*ckwg +29
 * Copyright 2014-2018 by Kitware, Inc.
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
 * \file
 * \brief Implementation for image exceptions
 */

#include "image.h"

#include <sstream>

namespace kwiver {
namespace vital {


image_exception
::image_exception() noexcept
{
  m_what = "An image exception";
}

image_exception
::~image_exception() noexcept
{
}


// ------------------------------------------------------------------
image_load_exception
::image_load_exception(std::string message) noexcept
  : m_message(message)
{
  m_what = message;
}

image_load_exception
::~image_load_exception() noexcept
{
}


// ------------------------------------------------------------------
image_type_mismatch_exception
::image_type_mismatch_exception(const std::string& message) noexcept
  : m_message(message)
{
  m_what = message;
}

image_type_mismatch_exception
::~image_type_mismatch_exception() noexcept
{
}


// ------------------------------------------------------------------
image_size_mismatch_exception
::image_size_mismatch_exception( const std::string& message,
                                 size_t correct_w, size_t correct_h,
                                 size_t given_w, size_t given_h) noexcept
  : m_message(message),
    m_correct_w(correct_w),
    m_correct_h(correct_h),
    m_given_w(given_w),
    m_given_h(given_h)
{
  std::ostringstream ss;
  ss << message
     << " (given: [" << given_w << ", " << given_h << "],"
     << " should be: [" << correct_w << ", " << correct_h << "])";
  m_what = ss.str();
}

image_size_mismatch_exception
::~image_size_mismatch_exception() noexcept
{
}

} } // end vital namespace
