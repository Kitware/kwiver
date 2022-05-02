// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_VITAL_PYTHON_COMPUTE_TRACK_DESCRIPTORS_H_
#define KWIVER_VITAL_PYTHON_COMPUTE_TRACK_DESCRIPTORS_H_

#include <pybind11/pybind11.h>

namespace py = pybind11;
namespace kwiver {

namespace vital {

namespace python {

void compute_track_descriptors( py::module& m );

} // namespace python

} // namespace vital

} // namespace kwiver
#endif
