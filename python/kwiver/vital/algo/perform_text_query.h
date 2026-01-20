// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_VITAL_PYTHON_PERFORM_TEXT_QUERY_H_
#define KWIVER_VITAL_PYTHON_PERFORM_TEXT_QUERY_H_

#include <pybind11/pybind11.h>

namespace kwiver {
namespace vital  {
namespace python {
namespace py = pybind11;

void perform_text_query(py::module &m);
}
}
}

#endif
