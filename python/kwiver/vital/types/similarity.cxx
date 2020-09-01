/*ckwg +29
 * Copyright 2017, 2020 by Kitware, Inc.
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

#include <vital/types/similarity.h>

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>

#include <memory>
namespace py = pybind11;
namespace kwiver {
namespace vital  {
namespace python {
namespace kv = kwiver::vital;

template < typename T >
void declare_similarity( py::module &m,
                         std::string const& class_typestr,
                         std::string const& dtype )
{
  using Class = kv::similarity_< T >;
  const std::string pyclass_name = std::string( "Similarity" ) + class_typestr;

  py::class_< Class, std::shared_ptr< Class > >( m, pyclass_name.c_str() )
    .def( py::init() )
    .def( py::init< kv::similarity_< float > const& >() )
    .def( py::init< kv::similarity_< double > const&>() )
    .def( py::init< T const&, kv::rotation_< T > const&, Eigen::Matrix< T, 3, 1 > const& >() )
    .def( py::init< Eigen::Matrix< T, 4, 4 > const& >() )
    .def( "matrix", &Class::matrix )
    .def( "inverse", &Class::inverse )
    .def( "__mul__", [] ( Class const& self, Class const& other )
    {
      return self * other;
    })
    .def( "__mul__", [] ( Class const& self, Eigen::Matrix< T, 3, 1 > const& rhs )
    {
      return self * rhs;
    })
    .def( "__eq__", [] ( Class const& self, Class const& other )
    {
      return self == other;
    })
    .def( "__ne__", [] ( Class const& self, Class const& other )
    {
      return self != other;
    })
    .def_property_readonly( "scale", &Class::scale )
    .def_property_readonly( "rotation", &Class::rotation )
    .def_property_readonly( "translation", &Class::translation )
    .def_property_readonly( "type_name", [ dtype ] ( Class const& self )
    {
      return dtype;
    })
    ;
}
}
}
}
using namespace kwiver::vital::python;
PYBIND11_MODULE(similarity, m)
{
  declare_similarity< float > ( m, "F", "f" );
  declare_similarity< double >( m, "D", "d" );
  ;
}