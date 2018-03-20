/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include <vital/algo/interpolate_track.h>

namespace py = pybind11;

typedef kwiver::vital::algorithm kv_algorithm;
typedef kwiver::vital::algo::interpolate_track kv_interpolate_track;
typedef std::shared_ptr<kv_interpolate_track> kv_interpolate_track_t;

/*
// This doesn't work quite right, because there are two do_callback functions with different argument signatures
// will clear up soon.
class wrap_interpolate
  : public kv_interpolate_track
{
  public:

    using kv_interpolate_track::interpolate_track;
    using kv_interpolate_track::do_callback;
};
*/

class interpolate_trampoline
  : public kv_interpolate_track
{
  public:

    using kv_interpolate_track::interpolate_track;

    // Virtual interpolate_track functions
    kwiver::vital::track_sptr interpolate( kwiver::vital::track_sptr init_states ) override;

    // These are pure virtual kwiver::vital::algorithm functions that interpolate_track doesn't instantiate
    // Make sure our trampoline isn't virtual
    void set_configuration(kwiver::vital::config_block_sptr config) override;
    bool check_configuration(kwiver::vital::config_block_sptr config) const override;
};

kwiver::vital::track_sptr
interpolate_trampoline
::interpolate( kwiver::vital::track_sptr init_states )
{
  PYBIND11_OVERLOAD_PURE(
    kwiver::vital::track_sptr,
    kv_interpolate_track,
    interpolate,
    init_states
  );
}

void
interpolate_trampoline
::set_configuration(kwiver::vital::config_block_sptr config)
{
  PYBIND11_OVERLOAD_PURE(
    void,
    kv_interpolate_track,
    set_configuration,
    config
  );
}

bool
interpolate_trampoline
::check_configuration(kwiver::vital::config_block_sptr config) const
{
  PYBIND11_OVERLOAD_PURE(
    bool,
    kv_interpolate_track,
    check_configuration,
    config
  );
}

PYBIND11_MODULE(interpolate_track, m)
{
  py::class_<kv_interpolate_track, kv_algorithm, interpolate_trampoline, kv_interpolate_track_t >(m, "InterpolateTrack")
  .def(py::init<>())
  .def("type_name", &kv_interpolate_track::type_name, py::call_guard<py::gil_scoped_release>(),
    "Return the name of the base algorithm.")
  .def("set_video_input", &kv_interpolate_track::set_video_input, py::call_guard<py::gil_scoped_release>(),
    py::arg("input"),
    "Supply a video input algorithm.")
  .def("interpolate", &kv_interpolate_track::interpolate, py::call_guard<py::gil_scoped_release>(),
    py::arg("init_states"),
    "Interpolate missing track states.")
  .def("set_progress_callback", &kv_interpolate_track::set_progress_callback, py::call_guard<py::gil_scoped_release>(),
    py::arg("init_states"),
    "Establish a callback to periodically report on progress.")
  /*.def("do_callback", &wrap_interpolate::do_callback, py::call_guard<py::gil_scoped_release>(),
    py::arg("progress"),
    "Call the supplied callback function if one is active.")
  .def("do_callback", &wrap_interpolate::do_callback, py::call_guard<py::gil_scoped_release>(),
    py::arg("steps"), py::arg("total"),
    "Call the supplied callback function if one is active.")*/
  ;
}
