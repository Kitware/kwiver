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

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <python/kwiver/vital/algo/trampoline/compute_depth_trampoline.txx>
#include <python/kwiver/vital/algo/compute_depth.h>

namespace py = pybind11;
namespace kwiver {
namespace vital  {
namespace python {
void compute_depth(py::module &m)
{
  py::class_< kwiver::vital::algo::compute_depth,
              std::shared_ptr<kwiver::vital::algo::compute_depth>,
              kwiver::vital::algorithm_def<kwiver::vital::algo::compute_depth>,
              compute_depth_trampoline<> >( m, "ComputeDepth" )
    .def(py::init())
    .def_static("static_type_name",
                &kwiver::vital::algo::compute_depth::static_type_name)
    .def("compute", static_cast<kwiver::vital::image_container_sptr
                                (kwiver::vital::algo::compute_depth::*)
                                (std::vector<kwiver::vital::image_container_sptr> const&,
                                 std::vector<kwiver::vital::camera_perspective_sptr> const&,
                                 double, double,
                                 unsigned int,
                                 vital::bounding_box<int> const&,
                                 std::vector<kwiver::vital::image_container_sptr> const&) const>
                    (&kwiver::vital::algo::compute_depth::compute))
    .def("compute", static_cast<kwiver::vital::image_container_sptr
                                (kwiver::vital::algo::compute_depth::*)
                                (std::vector<kwiver::vital::image_container_sptr> const&,
                                 std::vector<kwiver::vital::camera_perspective_sptr> const&,
                                 double, double,
                                 unsigned int,
                                 vital::bounding_box<int> const&,
                                 kwiver::vital::image_container_sptr&,
                                 std::vector<kwiver::vital::image_container_sptr> const&) const>
                    (&kwiver::vital::algo::compute_depth::compute))
    .def("set_callback",
        &kwiver::vital::algo::compute_depth::set_callback);
}
}
}
}
