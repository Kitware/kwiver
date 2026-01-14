// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <pybind11/pybind11.h>
#include <python/kwiver/vital/algo/trampoline/track_objects_trampoline.txx>
#include <python/kwiver/vital/algo/track_objects.h>

namespace py = pybind11;
namespace kwiver {
namespace vital  {
namespace python {
void track_objects(py::module &m)
{
  py::class_< kwiver::vital::algo::track_objects,
              std::shared_ptr<kwiver::vital::algo::track_objects>,
              kwiver::vital::algorithm_def<kwiver::vital::algo::track_objects>,
              track_objects_trampoline<> >(m, "TrackObjects")
    .def(py::init())
    .def_static("static_type_name", &kwiver::vital::algo::track_objects::static_type_name)
    .def("track",
         static_cast<kwiver::vital::object_track_set_sptr
           (kwiver::vital::algo::track_objects::*)(
             kwiver::vital::timestamp,
             kwiver::vital::image_container_sptr,
             kwiver::vital::detected_object_set_sptr) const>(
           &kwiver::vital::algo::track_objects::track),
         py::arg("ts"), py::arg("image"), py::arg("detections"),
         "Track objects in a new frame")
    .def("track_with_homography",
         static_cast<kwiver::vital::object_track_set_sptr
           (kwiver::vital::algo::track_objects::*)(
             kwiver::vital::timestamp,
             kwiver::vital::image_container_sptr,
             kwiver::vital::detected_object_set_sptr,
             kwiver::vital::f2f_homography_sptr) const>(
           &kwiver::vital::algo::track_objects::track),
         py::arg("ts"), py::arg("image"), py::arg("detections"), py::arg("src_to_ref"),
         "Track objects with homography support")
    .def("track_with_existing",
         static_cast<kwiver::vital::object_track_set_sptr
           (kwiver::vital::algo::track_objects::*)(
             kwiver::vital::timestamp,
             kwiver::vital::image_container_sptr,
             kwiver::vital::detected_object_set_sptr,
             kwiver::vital::object_track_set_sptr) const>(
           &kwiver::vital::algo::track_objects::track),
         py::arg("ts"), py::arg("image"), py::arg("detections"), py::arg("existing_tracks"),
         "Track objects with existing tracks")
    .def("initialize", &kwiver::vital::algo::track_objects::initialize,
         py::arg("ts"), py::arg("image"), py::arg("seed_detections"),
         "Initialize the tracker for a new sequence")
    .def("finalize", &kwiver::vital::algo::track_objects::finalize,
         "Finalize tracking and return all tracks")
    .def("reset", &kwiver::vital::algo::track_objects::reset,
         "Reset the tracker state");
}
}
}
}
