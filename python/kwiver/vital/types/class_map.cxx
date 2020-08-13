/*ckwg +29
 * Copyright 2017-2020 by Kitware, Inc.
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

#include <pybind11/stl.h>

#include <vital/types/class_map.h>
#include <vital/types/class_map_types.h>

namespace py = pybind11;
namespace kv = kwiver::vital;
template < typename T >
void declare_class_map(py::module &m, std::string const &typestr )
{
  using Class = kv::class_map<T>;
  const std::string pyclass_name = std::string( "ClassMap") + typestr;

  py::class_<Class, std::shared_ptr< Class >>(m, pyclass_name.c_str() )
  .def(py::init<>())
  .def(py::init<std::vector<std::string>, std::vector<double>>())
  .def(py::init<std::string, double>())

  .def("has_class_name", &Class::has_class_name,
    py::arg("class_name"))
  .def("score", &Class::class_map::score,
    py::arg("class_name"))
  .def("get_most_likely_class", [](std::shared_ptr<Class> self)
    {
      std::string max_name;
      double max_score;
      self->get_most_likely(max_name, max_score);
      return max_name;
    })
  .def("get_most_likely_score", [](std::shared_ptr<Class> self)
    {
      std::string max_name;
      double max_score;
      self->get_most_likely(max_name, max_score);
      return max_score;
    })
  .def("set_score", &Class::set_score,
    py::arg("class_name"), py::arg("score"))
  .def("delete_score", &Class::delete_score,
    py::arg("class_name"))
  .def("class_names", &Class::class_names,
    py::arg("threshold")=Class::INVALID_SCORE)
  .def_static("all_class_names", &Class::all_class_names)
  ;
}


PYBIND11_MODULE(class_map, m)
{
  declare_class_map< kv::detected_object_type >( m, "DetObj" );
  declare_class_map< kv::activity_type >( m, "Activity" );
}
