/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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


#include "file_name.h"
#include <kwiversys/SystemTools.hxx>
typedef kwiversys::SystemTools ST;

namespace kwiver {
namespace vital {

// ------------------------------------------------------------------
file_name::
file_name( const std::string& data)
  : m_name( data )
{
}


file_name::
file_name( const char* data )
  : m_name( data )
{
}


file_name::
file_name()
{ }


// ------------------------------------------------------------------
bool
file_name::
exists( const kwiver::vital::path_t& search_directory ) const
{
  std::string file_path = search_directory + "/" + m_name;
  return ST::FileExists(file_path);
}


// ------------------------------------------------------------------
std::string const&
file_name::
name() const
{
  return m_name;
}

// ------------------------------------------------------------------
bool
file_name::
operator==( const file_name& other ) const
{
  return this->m_name == other.m_name;
}


// ------------------------------------------------------------------
bool
file_name::
operator!=( const file_name& other ) const
{
  return this->m_name != other.m_name;
}

} } // end namespace
