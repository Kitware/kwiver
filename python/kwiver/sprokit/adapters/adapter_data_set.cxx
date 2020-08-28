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
#include <pybind11/embed.h>
#include <pybind11/stl_bind.h>

// Type conversions
#include <vital/types/image_container.h>
#include <vital/types/detected_object_set.h>
#include <vital/types/descriptor_set.h>
#include <vital/types/track_set.h>
#include <vital/types/object_track_set.h>
#include <vital/types/feature_track_set.h>
#include <vital/types/timestamp.h>
#include <vital/types/geo_polygon.h>
#include <vital/types/homography_f2f.h>

#include <memory>

PYBIND11_MAKE_OPAQUE(std::vector<unsigned char>);
PYBIND11_MAKE_OPAQUE(std::vector<double>);
PYBIND11_MAKE_OPAQUE(std::vector<std::string>);

using namespace kwiver::adapter;
namespace py = pybind11;
namespace kwiver{
namespace sprokit{
namespace python{

// Accept a generic python object and cast to correct type before adding.
// This keeps a C++ process from having to deal with a py::object.
// We'll also accept datums directly so users can always use the index operator.
// When these are updated to add more types, the same will need to be done for datum
void add_value_correct_type(adapter_data_set &self, ::sprokit::process::port_t const& port, py::object obj)
{
  if (py::isinstance<::sprokit::datum>(obj))
  {
    ::sprokit::datum_t casted_obj = obj.cast<::sprokit::datum_t>();
    self.add_datum(port, casted_obj);
    return;
  }

  #define ADD_OBJECT(PYTYPE, TYPE) \
  if (py::isinstance<PYTYPE>(obj)) \
  { \
    TYPE casted_obj = obj.cast<TYPE>(); \
    self.add_value<TYPE>(port, casted_obj); \
    return; \
  }

  ADD_OBJECT(py::int_, int)
  ADD_OBJECT(py::float_, float)
  ADD_OBJECT(py::str, std::string)
  ADD_OBJECT(kwiver::vital::image_container, std::shared_ptr<kwiver::vital::image_container>)
  ADD_OBJECT(kwiver::vital::descriptor_set, std::shared_ptr<kwiver::vital::descriptor_set>)
  ADD_OBJECT(kwiver::vital::detected_object_set, std::shared_ptr<kwiver::vital::detected_object_set>)
  ADD_OBJECT(kwiver::vital::track_set, std::shared_ptr<kwiver::vital::track_set>)
  ADD_OBJECT(kwiver::vital::feature_track_set, std::shared_ptr<kwiver::vital::feature_track_set>)
  ADD_OBJECT(kwiver::vital::object_track_set, std::shared_ptr<kwiver::vital::object_track_set>)
  ADD_OBJECT(std::vector<double>, std::shared_ptr<std::vector<double>>)
  ADD_OBJECT(std::vector<std::string>, std::shared_ptr<std::vector<std::string>>)
  ADD_OBJECT(std::vector<unsigned char>, std::shared_ptr<std::vector<unsigned char>>)
  ADD_OBJECT(kwiver::vital::bounding_box_d, kwiver::vital::bounding_box_d)
  ADD_OBJECT(kwiver::vital::timestamp, kwiver::vital::timestamp)
  ADD_OBJECT(kwiver::vital::geo_polygon, kwiver::vital::geo_polygon)
  ADD_OBJECT(kwiver::vital::f2f_homography, kwiver::vital::f2f_homography)

  #undef ADD_OBJECT

  throw std::runtime_error("Unable to add object to adapter data set");
}

// Take data of an unknown type from a port and return. Can't return as an "any" object,
// so need to cast
py::object get_port_data_correct_type(adapter_data_set &self, ::sprokit::process::port_t const& port)
{
  kwiver::vital::any const any = self.get_port_data<kwiver::vital::any>(port);

  #define GET_OBJECT(TYPE) \
  if (typeid(TYPE) == any.type()) \
  { \
    return py::cast(kwiver::vital::any_cast<TYPE>(any)); \
  }

  GET_OBJECT(int)
  GET_OBJECT(float)
  GET_OBJECT(std::string)
  GET_OBJECT(std::shared_ptr<kwiver::vital::image_container>)
  GET_OBJECT(std::shared_ptr<kwiver::vital::descriptor_set>)
  GET_OBJECT(std::shared_ptr<kwiver::vital::detected_object_set>)
  GET_OBJECT(std::shared_ptr<kwiver::vital::track_set>)
  GET_OBJECT(std::shared_ptr<kwiver::vital::feature_track_set>)
  GET_OBJECT(std::shared_ptr<kwiver::vital::object_track_set>)
  GET_OBJECT(std::shared_ptr<std::vector<double>>)
  GET_OBJECT(std::shared_ptr<std::vector<std::string>>)
  GET_OBJECT(std::shared_ptr<std::vector<unsigned char>>)
  GET_OBJECT(kwiver::vital::bounding_box_d)
  GET_OBJECT(kwiver::vital::timestamp)
  GET_OBJECT(kwiver::vital::geo_polygon)
  GET_OBJECT(kwiver::vital::f2f_homography)

  #undef GET_OBJECT

  std::string msg("Unable to convert object found at adapter data set port: ");
  msg += port;
  msg += ". Data is of type: ";
  msg += any.type_name();
  throw std::runtime_error(msg);
}

}
}
}


PYBIND11_MODULE(adapter_data_set, m)
{
  py::bind_vector<std::vector<unsigned char>, std::shared_ptr<std::vector<unsigned char>>>(m, "VectorUChar", py::module_local(true));
  py::bind_vector<std::vector<double>, std::shared_ptr<std::vector<double>>>(m, "VectorDouble", py::module_local(true));
  py::bind_vector<std::vector<std::string>, std::shared_ptr<std::vector<std::string>>>(m, "VectorString", py::module_local(true));

  py::enum_<adapter_data_set::data_set_type>(m, "DataSetType"
    , "Type of data set.")
    .value("data", adapter_data_set::data)
    .value("end_of_input", adapter_data_set::end_of_input)
  ;

  py::class_< adapter_data_set, std::shared_ptr<adapter_data_set > > ads(m, "AdapterDataSet");
    ads.def_static("create", &adapter_data_set::create
        , (py::arg("type") = adapter_data_set::data_set_type::data))

    .def("__iter__", [](adapter_data_set &self)
    {
      return py::make_iterator(self.cbegin(), self.cend());
    }, py::keep_alive<0,1>())

    // Members
    .def("type", &adapter_data_set::type)
    .def("is_end_of_data", &adapter_data_set::is_end_of_data)

    // General add_value which adds any type, and __setitem__
    .def("add_value", &kwiver::sprokit::python::add_value_correct_type)
    .def("__setitem__", &kwiver::sprokit::python::add_value_correct_type)

    .def("add_datum", &adapter_data_set::add_datum)

    // General get_value which gets data of any type from a port and __getitem__
    .def("get_port_data", &kwiver::sprokit::python::get_port_data_correct_type)
    .def("__getitem__", &kwiver::sprokit::python::get_port_data_correct_type)

    // The add_value function is templated.
    // To bind the function, we must bind explicit instances of it,
    // each with a different type.
    // First the native C++ types
    .def("_add_int", &adapter_data_set::add_value<int>)
    .def("_add_float", &adapter_data_set::add_value<float>)
    .def("_add_string", &adapter_data_set::add_value<std::string>)

    // Next shared ptrs to kwiver vital types
    .def("_add_image_container", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::image_container > >)
    .def("_add_descriptor_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::descriptor_set > >)
    .def("_add_detected_object_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::detected_object_set > >)
    .def("_add_track_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::track_set > >)
    .def("_add_feature_track_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::feature_track_set > >)
    .def("_add_object_track_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::object_track_set > >)
    // Next shared ptrs to native C++ types
    .def("_add_double_vector", &adapter_data_set::add_value<std::shared_ptr<std::vector<double>>>)
    .def("_add_string_vector", &adapter_data_set::add_value<std::shared_ptr<std::vector<std::string>>>)
    .def("_add_uchar_vector", &adapter_data_set::add_value<std::shared_ptr<std::vector<unsigned char>>>)
    // Next kwiver vital types
    .def("_add_bounding_box", &adapter_data_set::add_value<kwiver::vital::bounding_box_d>)
    .def("_add_timestamp", &adapter_data_set::add_value<kwiver::vital::timestamp>)
    .def("_add_corner_points", &adapter_data_set::add_value<kwiver::vital::geo_polygon>)
    .def("_add_f2f_homography", &adapter_data_set::add_value<kwiver::vital::f2f_homography>)

    .def("empty", &adapter_data_set::empty)

    // get_port_data is also templated
    .def("_get_port_data_int", &adapter_data_set::get_port_data<int>)
    .def("_get_port_data_float", &adapter_data_set::get_port_data<float>)
    .def("_get_port_data_string", &adapter_data_set::get_port_data<std::string>)
    // Next shared ptrs to kwiver vital types
    .def("_get_port_data_image_container", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::image_container > >)
    .def("_get_port_data_descriptor_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::descriptor_set > >)
    .def("_get_port_data_detected_object_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::detected_object_set > >)
    .def("_get_port_data_track_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::track_set > >)
    .def("_get_port_data_feature_track_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::feature_track_set > >)
    .def("_get_port_data_object_track_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::object_track_set > >)
    //Next shared ptrs to native C++ types
    .def("_get_port_data_double_vector", &adapter_data_set::get_port_data<std::shared_ptr<std::vector<double>>>)
    .def("_get_port_data_string_vector", &adapter_data_set::get_port_data<std::shared_ptr<std::vector<std::string>>>)
    .def("_get_port_data_uchar_vector", &adapter_data_set::get_port_data<std::shared_ptr<std::vector<unsigned char>>>)
    //Next kwiver vital types
    .def("_get_port_data_bounding_box", &adapter_data_set::get_port_data<kwiver::vital::bounding_box_d>)
    .def("_get_port_data_timestamp", &adapter_data_set::get_port_data<kwiver::vital::timestamp>)
    .def("_get_port_data_corner_points", &adapter_data_set::get_port_data<kwiver::vital::geo_polygon>)
    .def("_get_port_data_f2f_homography", &adapter_data_set::get_port_data<kwiver::vital::f2f_homography>)
    .def("__nice__", [](const adapter_data_set& self) -> std::string {
    auto locals = py::dict(py::arg("self")=self);
      py::exec(R"(
          retval = 'size={}'.format(len(self))
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
        from kwiver.sprokit.pipeline import datum

        classname = self.__class__.__name__
        devnice = self.__nice__()
        retval = '<%s(%s)>\n' % (classname, devnice)
        retval += '\t{'
        for i, (port, datum_obj) in enumerate(self):
            if i:
                retval += ', '
            retval += port
            retval += ": "
            retval += str(datum_obj.get_datum())
        retval += '}'
    )", py::globals(), locals);
    return locals["retval"].cast<std::string>();
    })
  .def("__len__", &adapter_data_set::size)
  ;

  ads.doc() = R"(
      Python bindings for kwiver::adapter::adapter_data_set

      Example:
          >>> from kwiver.sprokit.adapters import adapter_data_set
          >>> # Following ads has type "data". We can add/get data to/from ports
          >>> ads = adapter_data_set.AdapterDataSet.create()
          >>> assert ads.type() == adapter_data_set.DataSetType.data
          >>> # Can add as a general python object
          >>> ads["port1"] = "a_string"
          >>> # Can also add by specifying type
          >>> ads._add_int("port2", 5)
          >>> # Get both values
          >>> assert ads["port1"] == "a_string"
          >>> assert ads._get_port_data_int("port2") == 5
      )";
}
