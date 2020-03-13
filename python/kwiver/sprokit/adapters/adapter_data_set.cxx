/*ckwg +29
 * Copyright 2016-2017, 2019 by Kitware, Inc.
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
#include <pybind11/stl_bind.h>
#include <memory>

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

using namespace pybind11;


void
ads_add_value_py(kwiver::adapter::adapter_data_set &self, sprokit::process::port_t const& port, object const& obj);

object
ads_get_port_data_py(kwiver::adapter::adapter_data_set &self, sprokit::process::port_t const& port);



PYBIND11_MODULE(adapter_data_set, m)
{


  enum_<kwiver::adapter::adapter_data_set::data_set_type>(m, "DataSetType"
    , "Type of data set.")
    .value("data", kwiver::adapter::adapter_data_set::data)
    .value("end_of_input", kwiver::adapter::adapter_data_set::end_of_input)
  ;

  m.def("create", &kwiver::adapter::adapter_data_set::create
    , (arg("type") = kwiver::adapter::adapter_data_set::data_set_type::data))
  ;

  class_< kwiver::adapter::adapter_data_set, std::shared_ptr<kwiver::adapter::adapter_data_set > >(m, "AdapterDataSet")
    .def("__iter__", [](kwiver::adapter::adapter_data_set &self){return make_iterator(self.cbegin(),self.cend());}, keep_alive<0,1>())
    .def("type", &kwiver::adapter::adapter_data_set::type)
    .def("is_end_of_data", &kwiver::adapter::adapter_data_set::is_end_of_data)
    .def("add_value", &ads_add_value_py)
    .def("add_datum", &kwiver::adapter::adapter_data_set::add_datum)

    // The add_value function is templated.
    // To bind the function, we must bind explicit instances of it,
    // each with a different type.
    // First the native C++ types
    .def("add_int", &kwiver::adapter::adapter_data_set::add_value<int>)
    .def("add_float", &kwiver::adapter::adapter_data_set::add_value<float>)
    .def("add_string", &kwiver::adapter::adapter_data_set::add_value<std::string>)

    // Next shared ptrs to kwiver vital types
    .def("add_image_container", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<kwiver::vital::image_container > >)
    .def("add_descriptor_set", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<kwiver::vital::descriptor_set > >)
    .def("add_detected_object_set", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<kwiver::vital::detected_object_set > >)
    .def("add_track_set", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<kwiver::vital::track_set > >)
    .def("add_feature_track_set", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<kwiver::vital::feature_track_set > >)
    .def("add_object_track_set", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<kwiver::vital::object_track_set > >)
    //Next shared ptrs to native C++ types
    .def("add_double_vector", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<std::vector<double > > >)
    .def("add_string_vector", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<std::vector<std::string > > >)
    .def("add_uchar_vector", &kwiver::adapter::adapter_data_set::add_value<std::shared_ptr<std::vector<unsigned char > > >)
    //Next kwiver vital types
    .def("add_bounding_box", &kwiver::adapter::adapter_data_set::add_value<kwiver::vital::bounding_box_d>)
    .def("add_timestamp", &kwiver::adapter::adapter_data_set::add_value<kwiver::vital::timestamp>)
    .def("add_corner_points", &kwiver::adapter::adapter_data_set::add_value<kwiver::vital::geo_polygon>)
    .def("add_f2f_homography", &kwiver::adapter::adapter_data_set::add_value<kwiver::vital::f2f_homography>)

    .def("empty", &kwiver::adapter::adapter_data_set::empty)

    // get_port_data is also templated
    .def("get_port_data", &ads_get_port_data_py)
    .def("get_port_data_int", &kwiver::adapter::adapter_data_set::get_port_data<int>)
    .def("get_port_data_float", &kwiver::adapter::adapter_data_set::get_port_data<float>)
    .def("get_port_data_string", &kwiver::adapter::adapter_data_set::get_port_data<std::string>)
    // Next shared ptrs to kwiver vital types
    .def("get_port_data_image_container", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::image_container > >)
    .def("get_port_data_descriptor_set", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::descriptor_set > >)
    .def("get_port_data_detected_object_set", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::detected_object_set > >)
    .def("get_port_data_track_set", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::track_set > >)
    .def("get_port_data_feature_track_set", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::feature_track_set > >)
    .def("get_port_data_object_track_set", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::object_track_set > >)
    //Next shared ptrs to native C++ types
    .def("get_port_data_double_vector", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<std::vector<double > > >)
    .def("get_port_data_string_vector", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<std::vector<std::string > > >)
    .def("get_port_data_uchar_vector", &kwiver::adapter::adapter_data_set::get_port_data<std::shared_ptr<std::vector<unsigned char > > >)
    //Next kwiver vital types
    .def("get_port_data_bounding_box", &kwiver::adapter::adapter_data_set::get_port_data<kwiver::vital::bounding_box_d>)
    .def("get_port_data_timestamp", &kwiver::adapter::adapter_data_set::get_port_data<kwiver::vital::timestamp>)
    .def("get_port_data_corner_points", &kwiver::adapter::adapter_data_set::get_port_data<kwiver::vital::geo_polygon>)
    .def("get_port_data_f2f_homography", &kwiver::adapter::adapter_data_set::get_port_data<kwiver::vital::f2f_homography>)
    ;
}

void
ads_add_value_py(kwiver::adapter::adapter_data_set &self, sprokit::process::port_t const& port, object const& obj)
{
  self.add_value<kwiver::vital::any>(port, obj);
}

object
ads_get_port_data_py(kwiver::adapter::adapter_data_set &self, sprokit::process::port_t const& port)
{
  // TODO: might need to check if self is type data
  object dat = none();
  kwiver::vital::any const any = self.get_port_data<kwiver::vital::any>(port);
  dat = kwiver::vital::any_cast<object>(any);
  return dat;
}
