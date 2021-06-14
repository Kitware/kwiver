// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <python/kwiver/vital/algo/trampoline/initialize_cameras_landmarks_trampoline.txx>
#include <python/kwiver/vital/algo/initialize_cameras_landmarks.h>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

namespace kwiver {
namespace vital  {
namespace python {
namespace py = pybind11;

void initialize_cameras_landmarks(py::module &m)
{
  py::class_< kwiver::vital::algo::initialize_cameras_landmarks,
              std::shared_ptr<kwiver::vital::algo::initialize_cameras_landmarks>,
              kwiver::vital::algorithm_def<kwiver::vital::algo::initialize_cameras_landmarks>,
              initialize_cameras_landmarks_trampoline<> >(m, "InitializeCamerasLandmarks")
    .def(py::init())
    .def_static("static_type_name",
        &kwiver::vital::algo::initialize_cameras_landmarks::static_type_name)
    .def("initialize",
        &kwiver::vital::algo::initialize_cameras_landmarks::initialize)
    .def("set_callback",
        &kwiver::vital::algo::initialize_cameras_landmarks::set_callback);
}
}
}
}
