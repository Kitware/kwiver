/*ckwg +29
 * Copyright 2017, 2019 by Kitware, Inc.
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

#ifndef PY_BBOX_H
#define PY_BBOX_H

#include <string.h>
#include <vital/types/bounding_box.h>

#include <Eigen/Core>

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/embed.h>

namespace py = pybind11;


template<typename T>
void bounding_box(py::module &m, const char * typestr)
{

  /*
   *

    Developer:
      python -c "import vital.types; help(vital.types.BoundingBox)"
      python -c "import vital.types; help(vital.types.BoundingBox)"
      python -m xdoctest vital.types BoundingBox --xdoc-dynamic

   *
   */
  typedef kwiver::vital::bounding_box<T> bbox;
  char pyclass_name[20];
  strcpy(pyclass_name, "BoundingBox");
  strcat(pyclass_name, typestr);

  py::class_<bbox, std::shared_ptr<bbox>>(m, pyclass_name, R"(
    Coordinate aligned bounding box.

    Example:
        >>> from vital.types import *
        >>> bbox = BoundingBox(0, 10, 100, 50)
        >>> print(str(bbox))
        <BoundingBox(0.0, 10.0, 100.0, 50.0)>
        >>> print(bbox.area())
        4000.0

    )")
  .def(py::init<Eigen::Matrix<T,2,1>, Eigen::Matrix<T,2,1>>())
  .def(py::init<Eigen::Matrix<T,2,1>, T, T>())
  .def(py::init<T, T, T, T>(), py::doc(R"(
        Create a box from four coordinates

        Args:
            xmin (float):  min x coord
            ymin (float):  min y coord
            xmax (float):  max x coord
            ymax (float):  max y coord
        )"))

  .def("is_valid", &bbox::is_valid)
  .def("center", &bbox::center)
  .def("upper_left", &bbox::upper_left)
  .def("lower_right", &bbox::lower_right)
  .def("min_x", &bbox::min_x)
  .def("min_y", &bbox::min_y)
  .def("max_x", &bbox::max_x)
  .def("max_y", &bbox::max_y)
  .def("width", &bbox::width)
  .def("height", &bbox::height)
  .def("area", &bbox::area)

  .def("__nice__", [](bbox& self) -> std::string {
    auto locals = py::dict(py::arg("self")=self);
    py::exec(R"(
        retval = '{}, {}, {}, {}'.format(self.min_x(), self.min_y(),
                                         self.max_x(), self.max_y())
    )", py::globals(), locals);
    return locals["retval"].cast<std::string>();
    })
  .def("__repr__", [](py::object& self) -> std::string {
    auto locals = py::dict(py::arg("self")=self);
    py::exec(R"(
        classname = self.__class__.__name__
        devnice = self.__nice__()
        retval = '<%s(%s) at %s>' % (classname, devnice, hex(id(self)))
    )", py::globals(), locals);
    return locals["retval"].cast<std::string>();
    })
  .def("__str__", [](py::object& self) -> std::string {
    auto locals = py::dict(py::arg("self")=self);
    py::exec(R"(
        classname = self.__class__.__name__
        devnice = self.__nice__()
        retval = '<%s(%s)>' % (classname, devnice)
    )", py::globals(), locals);
    return locals["retval"].cast<std::string>();
    })
  ;
}


#endif