// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <python/kwiver/vital/algo/trampoline/perform_text_query_trampoline.txx>
#include <python/kwiver/vital/algo/perform_text_query.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace kwiver {
namespace vital  {
namespace python {
namespace py = pybind11;

void perform_text_query(py::module &m)
{
  py::class_< kwiver::vital::algo::perform_text_query,
              std::shared_ptr<kwiver::vital::algo::perform_text_query>,
              kwiver::vital::algorithm_def<kwiver::vital::algo::perform_text_query>,
              perform_text_query_trampoline<> >(m, "PerformTextQuery")
    .def(py::init())
    .def_static("static_type_name",
        &kwiver::vital::algo::perform_text_query::static_type_name)
    .def("perform_query",
        &kwiver::vital::algo::perform_text_query::perform_query,
        py::arg("text_query"),
        py::arg("images"),
        py::arg("timestamps") = std::vector<kwiver::vital::timestamp>(),
        py::arg("input_tracks") = std::vector<kwiver::vital::object_track_set_sptr>());
}
}
}
}
