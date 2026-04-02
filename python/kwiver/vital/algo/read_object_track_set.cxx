// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <python/kwiver/vital/algo/trampoline/read_object_track_set_trampoline.txx>
#include <python/kwiver/vital/algo/read_object_track_set.h>

#include <pybind11/pybind11.h>

namespace kwiver {
namespace vital  {
namespace python {
namespace py = pybind11;

using rots = kwiver::vital::algo::read_object_track_set;

void read_object_track_set(py::module &m)
{
  py::class_< rots,
              std::shared_ptr<rots>,
              kwiver::vital::algorithm_def<rots>,
              read_object_track_set_trampoline<> >(m, "ReadObjectTrackSet")
    .def(py::init())
    .def_static("static_type_name", &rots::static_type_name)
    .def("open", &rots::open)
    .def("close", &rots::close)
    .def("read_set",
      [](rots& self) {
        kwiver::vital::object_track_set_sptr result;
        bool has_result = self.read_set(result);
        return has_result ? py::cast(result) : py::cast(nullptr);
      },
      R"(Return the next ObjectTrackSet, or None if the input is exhausted)");
}
}
}
}
