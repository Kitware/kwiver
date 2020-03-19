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

#include <vital/types/homography.h>

#include <Eigen/Core>

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>

namespace py=pybind11;
namespace kwiver {
namespace vital  {
namespace python {

namespace kv=kwiver::vital;

// Easy way to automate bindings of templated classes.
// For more information, see below link
// https://stackoverflow.com/questions/47487888/pybind11-template-class-of-many-types
template< typename T >
void declare_homogaphy( py::module &m, std::string const& typestr )
{
  using Class = kv::homography_< T >;
  using matrix_t = Eigen::Matrix< T, 3, 3>;
  const std::string pyclass_name = std::string( "Homography" ) + typestr;

  py::class_< Class, std::shared_ptr< Class >, kv::homography >(m, pyclass_name.c_str())
  .def(py::init())
  .def(py::init< matrix_t const& >())
  .def_static("random", [] ()
  {
    return Class(matrix_t::Random(3, 3));
  })
  .def("matrix", (matrix_t& (Class::*) ()) &Class::get_matrix)
  .def("inverse", &Class::inverse)
  .def("map", &Class::map,
    py::arg("point"))
  .def("normalize", &Class::normalize)
  .def("__mul__", &Class::operator*)
  .def_property_readonly( "type_name", [] ( Class const& self )
  {
    return self.data_type().name()[0];
  })
  ;
}

}
}
}

using namespace kwiver::vital::python;
PYBIND11_MODULE(homography, m)
{
  py::module::import("kwiver.vital.types.transform_2d");
  py::class_< kv::homography,
              kv::transform_2d,
              std::shared_ptr< kv::homography > >(m, "BaseHomography");
  declare_homogaphy< float  >(m, "F");
  declare_homogaphy< double >(m, "D");
}
