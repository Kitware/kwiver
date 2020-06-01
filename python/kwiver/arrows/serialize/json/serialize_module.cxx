#include <pybind11/pybind11.h>

#include <python/kwiver/arrows/serialize/json/serialize_bounding_box.h>
#include <python/kwiver/arrows/serialize/json/serialize_detected_object.h>
#include <python/kwiver/arrows/serialize/json/serialize_detected_object_type.h>
#include <python/kwiver/arrows/serialize/json/serialize_detected_object_set.h>
#include <python/kwiver/arrows/serialize/json/serialize_image.h>
#include <python/kwiver/arrows/serialize/json/serialize_timestamp.h>
#include <python/kwiver/arrows/serialize/json/serialize_track_state.h>
#include <python/kwiver/arrows/serialize/json/serialize_track.h>

namespace py = pybind11;

PYBIND11_MODULE(json,  m)
{
  serialize_bounding_box(m);
  serialize_detected_object(m);
  serialize_detected_object_type(m);
  serialize_detected_object_set(m);
  serialize_image(m);
  serialize_timestamp(m);
  serialize_track_state(m);
  serialize_track(m);
}
