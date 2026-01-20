// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <python/kwiver/vital/algo/trampoline/segment_via_points_trampoline.txx>
#include <python/kwiver/vital/algo/segment_via_points.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace kwiver {
namespace vital  {
namespace python {
namespace py = pybind11;

void segment_via_points(py::module &m)
{
  py::class_< kwiver::vital::algo::segment_via_points,
              std::shared_ptr<kwiver::vital::algo::segment_via_points>,
              kwiver::vital::algorithm_def<kwiver::vital::algo::segment_via_points>,
              segment_via_points_trampoline<> >(m, "SegmentViaPoints")
    .def(py::init())
    .def_static("static_type_name",
        &kwiver::vital::algo::segment_via_points::static_type_name)
    .def("segment",
        &kwiver::vital::algo::segment_via_points::segment,
        py::arg("image"),
        py::arg("points"),
        py::arg("point_labels"));
}
}
}
}
