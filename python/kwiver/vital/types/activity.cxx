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

#include <vital/vital_types.h>
#include <vital/types/activity.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace kv = kwiver::vital;

PYBIND11_MODULE(activity, m)
{
  py::class_<kv::activity,
             std::shared_ptr<kv::activity>>(m, "Activity")
    .def(py::init<>())
    .def(py::init<kv::activity_id_t,
                  kv::activity_label_t,
                  kv::activity_confidence_t,
                  kv::activity_type_sptr,
                  kv::timestamp,
                  kv::timestamp,
                  kv::object_track_set_sptr>(),
          py::arg("activity_id"),
          py::arg("activity_label") = kv::UNDEFINED_ACTIVITY,
          py::arg("activity_confidence") = -1.0,
          py::arg("activity_type") = nullptr,
          py::arg("start_time") = kv::timestamp(-1, -1),
          py::arg("end_time") = kv::timestamp(-1, -1),
          py::arg("participants") = nullptr)
    .def_property("id", &kv::activity::id,
                        &kv::activity::set_id)
    .def_property("label", &kv::activity::label,
                           &kv::activity::set_label)
    .def_property("activity_type", &kv::activity::activity_type,
                                   &kv::activity::set_activity_type)
    .def_property("confidence", &kv::activity::confidence,
                                &kv::activity::set_confidence)
    .def_property("start_time", &kv::activity::start,
                                &kv::activity::set_start)
    .def_property("end_time", &kv::activity::end,
                              &kv::activity::set_end)
    .def_property("participants", &kv::activity::participants,
                                  &kv::activity::set_participants)
    .def_property_readonly("duration", &kv::activity::duration)
    ;
}
