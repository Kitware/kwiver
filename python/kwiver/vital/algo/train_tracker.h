// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_VITAL_PYTHON_TRAIN_TRACKER_H_
#define KWIVER_VITAL_PYTHON_TRAIN_TRACKER_H_

#include <pybind11/pybind11.h>

namespace kwiver {

namespace vital {

namespace python {

namespace py = pybind11;

void train_tracker( py::module& m );

} // namespace python

} // namespace vital

} // namespace kwiver

#endif
