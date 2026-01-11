// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <python/kwiver/vital/algo/trampoline/train_tracker_trampoline.txx>
#include <python/kwiver/vital/algo/train_tracker.h>

#include <pybind11/pybind11.h>

namespace kwiver {
namespace vital  {
namespace python {
namespace py = pybind11;

void train_tracker(py::module &m)
{
  py::class_< kwiver::vital::algo::train_tracker,
              std::shared_ptr<kwiver::vital::algo::train_tracker>,
              kwiver::vital::algorithm_def<kwiver::vital::algo::train_tracker>,
              train_tracker_trampoline<> >(m, "TrainTracker")
    .def(py::init())
    .def_static("static_type_name",
        &kwiver::vital::algo::train_tracker::static_type_name)
    .def("add_data_from_disk",
         &kwiver::vital::algo::train_tracker::add_data_from_disk)
    .def("add_data_from_memory",
         &kwiver::vital::algo::train_tracker::add_data_from_memory)
    .def("update_model",
         &kwiver::vital::algo::train_tracker::update_model);
}

}
}
}
