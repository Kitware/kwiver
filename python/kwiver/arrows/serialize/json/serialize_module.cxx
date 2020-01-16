#include <pybind11/pybind11.h>

#include <python/kwiver/arrows/serialize/json/serialize_bounding_box.h>

namespace py = pybind11;

PYBIND11_MODULE(json,  m)
{
  serialize_bounding_box(m);
}
