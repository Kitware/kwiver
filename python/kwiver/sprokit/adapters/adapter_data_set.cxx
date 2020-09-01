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

#include <sprokit/processes/adapters/adapter_data_set.h>

#include <pybind11/pybind11.h>

// Type conversions
#include <vital/types/timestamp.h>

#include <memory>

using namespace kwiver::adapter;
namespace py = pybind11;
namespace kwiver{
namespace sprokit{
namespace python{

// Accepts a generic python object. If the object is either an integer or
// a timestamp, we can convert it and add it to a port
void add_value_correct_type(adapter_data_set &self, ::sprokit::process::port_t const& port, py::object obj)
{
  if (py::isinstance<py::int_>(obj))
  {
    int casted_obj = obj.cast<int>();
    self.add_value<int>(port, casted_obj);
    return;
  }

  if (py::isinstance<vital::timestamp>(obj))
  {
    vital::timestamp casted_obj = obj.cast<vital::timestamp>();
    self.add_value<vital::timestamp>(port, casted_obj);
    return;
  }
}

// Take data of an unknown type from a port and return. If data is either an int or
// a timestamp, we can convert it and return
py::object get_port_data_correct_type(adapter_data_set &self, ::sprokit::process::port_t const& port)
{
  kwiver::vital::any const any = self.get_port_data<kwiver::vital::any>(port);

  if (typeid(int) == any.type())
  {
    return py::cast(kwiver::vital::any_cast<int>(any));
  }

  if (typeid(vital::timestamp) == any.type())
  {
    return py::cast(kwiver::vital::any_cast<vital::timestamp>(any));
  }

  std::string msg("Unable to convert object found at adapter data set port: ");
  msg += port;
  msg += ". Data is of type: ";
  msg += any.type_name();
  throw py::type_error(msg);
}

}
}
}

// Simplification of adapter_data_set. Can only add and get ints and timestamps
// to and from various ports. Can think of adapter_data_set as an std::map
PYBIND11_MODULE(adapter_data_set, m)
{
  py::enum_<adapter_data_set::data_set_type>(m, "DataSetType"
    , "Type of data set.")
    .value("data", adapter_data_set::data)
    .value("end_of_input", adapter_data_set::end_of_input)
  ;

  py::class_< adapter_data_set, std::shared_ptr<adapter_data_set > > ads(m, "AdapterDataSet");
    ads.def_static("create", &adapter_data_set::create
        , (py::arg("type") = adapter_data_set::data_set_type::data))

    // General add_value which can recognize ints and timestamps
    .def("add_value", &kwiver::sprokit::python::add_value_correct_type)

    // General get_value which can recognize ints and timestamps
    .def("get_port_data", &kwiver::sprokit::python::get_port_data_correct_type)
    ;
}