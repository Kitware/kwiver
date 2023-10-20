// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_PYTHON_ARROW_STANAG_4607_SEGMENTS_H_
#define KWIVER_PYTHON_ARROW_STANAG_4607_SEGMENTS_H_

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <arrows/stanag/segments/stanag_4607_segments.h>

namespace py = pybind11;

namespace kas = kwiver::arrows::stanag;

void stanag_4607_segments( py::module& m );

#endif
