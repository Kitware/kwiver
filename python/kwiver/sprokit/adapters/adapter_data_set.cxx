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
using namespace kwiver::adapter;

PYBIND11_MODULE(adapter_data_set, m)
{
  enum_<adapter_data_set::data_set_type>(m, "DataSetType"
    , "Type of data set.")
    .value("data", adapter_data_set::data)
    .value("end_of_input", adapter_data_set::end_of_input)
  ;

  class_< adapter_data_set, std::shared_ptr<adapter_data_set > >(m, "AdapterDataSet")
    .def_static("create", &adapter_data_set::create
        , (arg("type") = adapter_data_set::data_set_type::data))

    .def("__iter__", [](adapter_data_set &self)
                                      {
                                        return make_iterator(self.cbegin(),self.cend());
                                      }
                        , keep_alive<0,1>())
    .def("type", &adapter_data_set::type)
    .def("is_end_of_data", &adapter_data_set::is_end_of_data)
    .def("add_value", [] (adapter_data_set &self, ::sprokit::process::port_t const& port, object const& obj)
    {
      self.add_value<kwiver::vital::any>(port, obj);
    })
    .def("add_datum", &adapter_data_set::add_datum)

    // The add_value function is templated.
    // To bind the function, we must bind explicit instances of it,
    // each with a different type.
    // First the native C++ types
    .def("add_int", &adapter_data_set::add_value<int>)
    .def("add_float", &adapter_data_set::add_value<float>)
    .def("add_string", &adapter_data_set::add_value<std::string>)

    // Next shared ptrs to kwiver vital types
    .def("add_image_container", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::image_container > >)
    .def("add_descriptor_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::descriptor_set > >)
    .def("add_detected_object_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::detected_object_set > >)
    .def("add_track_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::track_set > >)
    .def("add_feature_track_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::feature_track_set > >)
    .def("add_object_track_set", &adapter_data_set::add_value<std::shared_ptr<kwiver::vital::object_track_set > >)
    //Next shared ptrs to native C++ types
    .def("add_double_vector", &adapter_data_set::add_value<std::shared_ptr<std::vector<double > > >)
    .def("add_string_vector", &adapter_data_set::add_value<std::shared_ptr<std::vector<std::string > > >)
    .def("add_uchar_vector", &adapter_data_set::add_value<std::shared_ptr<std::vector<unsigned char > > >)
    //Next kwiver vital types
    .def("add_bounding_box", &adapter_data_set::add_value<kwiver::vital::bounding_box_d>)
    .def("add_timestamp", &adapter_data_set::add_value<kwiver::vital::timestamp>)
    .def("add_corner_points", &adapter_data_set::add_value<kwiver::vital::geo_polygon>)
    .def("add_f2f_homography", &adapter_data_set::add_value<kwiver::vital::f2f_homography>)

    .def("empty", &adapter_data_set::empty)

    // get_port_data is also templated
    .def("get_port_data", [] (adapter_data_set &self, ::sprokit::process::port_t const& port)
    {
      object dat = none();
      kwiver::vital::any const any = self.get_port_data<kwiver::vital::any>(port);
      dat = kwiver::vital::any_cast<object>(any);
      return dat;
    })
    .def("get_port_data_int", &adapter_data_set::get_port_data<int>)
    .def("get_port_data_float", &adapter_data_set::get_port_data<float>)
    .def("get_port_data_string", &adapter_data_set::get_port_data<std::string>)
    // Next shared ptrs to kwiver vital types
    .def("get_port_data_image_container", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::image_container > >)
    .def("get_port_data_descriptor_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::descriptor_set > >)
    .def("get_port_data_detected_object_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::detected_object_set > >)
    .def("get_port_data_track_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::track_set > >)
    .def("get_port_data_feature_track_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::feature_track_set > >)
    .def("get_port_data_object_track_set", &adapter_data_set::get_port_data<std::shared_ptr<kwiver::vital::object_track_set > >)
    //Next shared ptrs to native C++ types
    .def("get_port_data_double_vector", &adapter_data_set::get_port_data<std::shared_ptr<std::vector<double > > >)
    .def("get_port_data_string_vector", &adapter_data_set::get_port_data<std::shared_ptr<std::vector<std::string > > >)
    .def("get_port_data_uchar_vector", &adapter_data_set::get_port_data<std::shared_ptr<std::vector<unsigned char > > >)
    //Next kwiver vital types
    .def("get_port_data_bounding_box", &adapter_data_set::get_port_data<kwiver::vital::bounding_box_d>)
    .def("get_port_data_timestamp", &adapter_data_set::get_port_data<kwiver::vital::timestamp>)
    .def("get_port_data_corner_points", &adapter_data_set::get_port_data<kwiver::vital::geo_polygon>)
    .def("get_port_data_f2f_homography", &adapter_data_set::get_port_data<kwiver::vital::f2f_homography>)
    ;
}
