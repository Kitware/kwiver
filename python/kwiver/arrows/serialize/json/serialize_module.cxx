#include <pybind11/pybind11.h>

#include <python/kwiver/arrows/serialize/json/serialize_bounding_box.h>
#include <python/kwiver/arrows/serialize/json/serialize_detected_object.h>
#include <python/kwiver/arrows/serialize/json/serialize_detected_object_type.h>

namespace py = pybind11;

PYBIND11_MODULE(json,  m)
{
  serialize_bounding_box(m);
  serialize_detected_object(m);
  serialize_detected_object_type(m);
}
