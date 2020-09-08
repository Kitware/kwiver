/*ckwg +29
 * Copyright 2020 by Kitware, Inc.
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

#include <vital/types/geo_MGRS.h>

#include <pybind11/pybind11.h>

#include <memory>
#include <sstream>

namespace py=pybind11;
namespace kv=kwiver::vital;

PYBIND11_MODULE( geo_MGRS, m )
{
  py::class_< kv::geo_MGRS, std::shared_ptr< kv::geo_MGRS > >( m, "GeoMGRS" )
  .def( py::init<>() )
  .def( py::init<const std::string&>() )
  .def( "is_empty", &kv::geo_MGRS::is_empty )
  .def( "is_valid", &kv::geo_MGRS::is_valid )
  .def( "set_coord", &kv::geo_MGRS::set_coord )
  .def( "coord", &kv::geo_MGRS::coord )
  .def( "__eq__", [] ( kv::geo_MGRS const& self, kv::geo_MGRS const& other )
  {
    return self == other;
  })
  .def( "__ne__", [] ( kv::geo_MGRS const& self, kv::geo_MGRS const& other )
  {
    return self != other;
  })
  .def( "__str__", [] ( kv::geo_MGRS const& self )
  {
    std::stringstream str;
    str << self;
    return str.str();
  })
  ;
}
