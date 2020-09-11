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

#include <vital/types/feature.h>

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>

#include <sstream>

namespace py=pybind11;
namespace kv=kwiver::vital;
namespace kwiver {
namespace vital  {
namespace python {

// Easy way to automate bindings of templated classes.
// For more information, see below link
// https://stackoverflow.com/questions/47487888/pybind11-template-class-of-many-types
template< typename T >
void declare_feature( py::module &m, std::string const& typestr )
{
  using Class = kv::feature_< T >;
  const std::string pyclass_name = std::string( "Feature" ) + typestr;

  py::class_< Class,
              std::shared_ptr< Class >,
              kv::feature >( m, pyclass_name.c_str() )
  .def( py::init<>() )
  .def( py::init< kv::feature const& >() )
  .def( py::init< Eigen::Matrix< T, 2, 1 > const&, T, T, T, kv::rgb_color const& >(),
    py::arg( "loc" ),
    py::arg( "mag" ) = 0.0,
    py::arg( "scale" ) = 1.0,
    py::arg( "angle" ) = 0.0,
    py::arg( "rgb_color" ) = kv::rgb_color() )
  .def( "clone", &Class::clone )
  .def( "__str__", [] ( const Class& self )
  {
    std::stringstream s;
    s << self;
    return s.str();
  })

  .def_property( "location",   &Class::get_loc,       &Class::set_loc )
  .def_property( "magnitude",  &Class::get_magnitude, &Class::set_magnitude )
  .def_property( "scale",      &Class::get_scale,     &Class::set_scale )
  .def_property( "angle",      &Class::get_angle,     &Class::set_angle )
  .def_property( "covariance", &Class::get_covar,     &Class::set_covar )
  .def_property( "color",      &Class::get_color,     &Class::set_color )
  .def_property_readonly( "type_name", [] ( const Class& self )
  {
    return self.data_type().name()[0];
  })

  ;
}
}
}
}

using namespace kwiver::vital::python;
PYBIND11_MODULE(feature, m)
{
  py::class_< kv::feature, std::shared_ptr< kv::feature > >( m, "Feature" )
  .def( "__eq__", [] ( const kv::feature& self, const kv::feature& other )
    {
      return self == other;
    })
  .def( "equal_except_for_angle", [] ( const kv::feature& self, const kv::feature& other )
    {
      return self.equal_except_for_angle( other );
    })
  .def( "__ne__", [] ( const kv::feature& self, const kv::feature& other )
    {
      return self != other;
    })
  ;
  declare_feature< float  >( m, "F" );
  declare_feature< double >( m, "D" );
}
