// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <python/kwiver/vital/algo/trampoline/refine_tracks_trampoline.txx>
#include <python/kwiver/vital/algo/refine_tracks.h>

#include <pybind11/pybind11.h>

namespace kwiver {
namespace vital  {
namespace python {
namespace py = pybind11;

void refine_tracks(py::module &m)
{
  py::class_< kwiver::vital::algo::refine_tracks,
              std::shared_ptr<kwiver::vital::algo::refine_tracks>,
              kwiver::vital::algorithm_def<kwiver::vital::algo::refine_tracks>,
              refine_tracks_trampoline<> >(m, "RefineTracks")
    .def(py::init())
    .def_static("static_type_name",
        &kwiver::vital::algo::refine_tracks::static_type_name)
    .def("refine", &kwiver::vital::algo::refine_tracks::refine)
    .def("finalize", &kwiver::vital::algo::refine_tracks::finalize);
}
}
}
}
