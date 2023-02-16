// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/types/track_descriptor.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>

namespace py=pybind11;
namespace kv=kwiver::vital;

PYBIND11_MODULE(track_descriptor, m)
{
  // First the history_entry class nested in track_descriptor
  py::class_<kv::track_descriptor::history_entry, std::shared_ptr<kv::track_descriptor::history_entry>>(m, "HistoryEntry")
  .def(py::init<const kv::timestamp&,
                const kv::track_descriptor::history_entry::image_bbox_t&,
                const kv::track_descriptor::history_entry::world_bbox_t&>())
  .def(py::init<const kv::timestamp&,
                const kv::track_descriptor::history_entry::world_bbox_t&>())
  .def("get_timestamp", &kv::track_descriptor::history_entry::get_timestamp)
  .def("get_image_location", &kv::track_descriptor::history_entry::get_image_location)
  .def("get_world_location", &kv::track_descriptor::history_entry::get_world_location)
  .def("__eq__", [](kv::track_descriptor::history_entry self, kv::track_descriptor::history_entry other)
                  {
                    return(self.get_timestamp() == other.get_timestamp() &&
                           self.get_image_location() == other.get_image_location() &&
                           self.get_world_location() == other.get_world_location());
                  })
  .def("__ne__", [](kv::track_descriptor::history_entry self, kv::track_descriptor::history_entry other)
                  {
                    return(self.get_timestamp() != other.get_timestamp() ||
                           self.get_image_location() != other.get_image_location() ||
                           self.get_world_location() != other.get_world_location());
                  })
  ;

  // Now the track_descriptor_class
  py::class_<kv::track_descriptor, std::shared_ptr<kv::track_descriptor>>(m, "TrackDescriptor")
  .def_static("create",
        static_cast<kv::track_descriptor_sptr (*) (std::string const&)>(&kv::track_descriptor::create))
  .def_static("create",
        static_cast<kv::track_descriptor_sptr (*) (kv::track_descriptor_sptr)>(&kv::track_descriptor::create))
  .def_property("type", &kv::track_descriptor::get_type, &kv::track_descriptor::set_type)
  .def_property("uid", &kv::track_descriptor::get_uid, &kv::track_descriptor::set_uid)
  .def("add_track_id", &kv::track_descriptor::add_track_id)
  .def("add_track_ids", &kv::track_descriptor::add_track_ids)
  .def("get_track_ids", &kv::track_descriptor::get_track_ids)
  .def("set_descriptor", &kv::track_descriptor::set_descriptor)
  .def("get_descriptor",
        (kv::track_descriptor::descriptor_data_sptr& (kv::track_descriptor::*) ())
        &kv::track_descriptor::get_descriptor)
  .def("at",
        (double& (kv::track_descriptor::*) (size_t))
        &kv::track_descriptor::at)
  .def("__getitem__", [] (kv::track_descriptor& self, size_t idx)
    {
      return self.at(idx);
    })
  .def("__setitem__", [] (kv::track_descriptor& self, size_t idx, double val)
    {
      self.at(idx) = val;
    })
  .def("descriptor_size", &kv::track_descriptor::descriptor_size)
  .def("resize_descriptor", (void (kv::track_descriptor::*) (size_t)) &kv::track_descriptor::resize_descriptor)
  .def("resize_descriptor", (void (kv::track_descriptor::*) (size_t, double)) &kv::track_descriptor::resize_descriptor)
  .def("has_descriptor", &kv::track_descriptor::has_descriptor)
  .def("set_history", &kv::track_descriptor::set_history)
  .def("add_history_entry", &kv::track_descriptor::add_history_entry)
  .def("get_history", &kv::track_descriptor::get_history)
  ;
}
