/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <vital/types/feature_track_set.h>
#include <python/kwiver/vital/util/pybind11.h>
#include <typeinfo>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
namespace kwiver {
namespace vital  {
namespace python {

typedef kwiver::vital::feature_track_state feat_track_state;
typedef kwiver::vital::feature_track_set feat_track_set;

namespace py = pybind11;
namespace kv = kwiver::vital;

// std::shared_ptr<feat_track_state>
// new_feat_track_state(int64_t frame,
//                      std::shared_ptr<PyFeatureBase> py_feat,
//                      std::shared_ptr<kwiver::vital::descriptor> d)
// {
//   std::shared_ptr<kwiver::vital::feature> f;
//   f = std::shared_ptr<kwiver::vital::feature>(py_feat->get_feature());
//   return std::shared_ptr<feat_track_state>(new feat_track_state(frame, f, d));
// }

std::shared_ptr<kwiver::vital::track>
get_track(std::shared_ptr<feat_track_set> &self, uint64_t id)
{
  auto track = self->get_track(id);
  if(!track)
  {
    throw py::index_error("Track does not exist in set");
  }

  return track;
}



class feature_track_set_trampoline : public kv::feature_track_set
{
public:
  using kv::feature_track_set::feature_track_set;
  kv::feature_set_sptr last_frame_features() const override;
  kv::descriptor_set_sptr last_frame_descriptors() const override;
  kv::feature_set_sptr frame_features(kv::frame_id_t offset = -1) const override;
  kv::descriptor_set_sptr frame_descriptors(kv::frame_id_t offset = -1) const override;
  std::vector< kv::feature_track_state_sptr > frame_feature_track_states(kv::frame_id_t offset = -1) const override;
  std::set< kv::frame_id_t > keyframes() const override;
};
}
}
}
using namespace kwiver::vital::python;
PYBIND11_MODULE(feature_track_set, m)
{
  py::module::import("kwiver.vital.types.track");
  
  py::class_< feat_track_state, kwiver::vital::track_state, std::shared_ptr< feat_track_state > >(m, "FeatureTrackState")
  .def(py::init< kv::frame_id_t, kv::feature_sptr, kv::descriptor_sptr, bool >())
  .def(py::init< feat_track_state >())
  .def(py::init([](kv::frame_id_t frame, kv::feature_sptr const& feature, kv::descriptor_sptr const &descriptor)
  {
    return std::shared_ptr< feat_track_state > (new feat_track_state(frame, feature, descriptor));
  }))
  .def("clone", ([](feat_track_state& self)
  {
    return self.clone(kv::clone_type::DEEP);
  }))
  .def("downcast", &kv::feature_track_state::downcast)
  //.def_property_readonly_static("downcast_transform", &feat_track_state::downcast_transform)
  .def_property_readonly("frame_id", &kwiver::vital::track_state::frame)
  .def_readwrite("feature", &feat_track_state::feature)
  .def_readwrite("descriptor", &feat_track_state::descriptor)
  .def_readwrite("inlier", &feat_track_state::inlier)
  ;

  py::module::import("kwiver.vital.types.track_set");

  py::class_<feat_track_set, kwiver::vital::track_set, kwiver::vital::python::feature_track_set_trampoline, std::shared_ptr<feat_track_set>>(m, "FeatureTrackSet")
  .def(py::init<>())
  .def(py::init<std::vector<std::shared_ptr<kwiver::vital::track>>>())
  .def("all_frame_ids", &feat_track_set::all_frame_ids)
  .def("get_track", &get_track,
    py::arg("id"))
  .def("first_frame", &feat_track_set::first_frame)
  .def("last_frame", &feat_track_set::last_frame)
  .def("size", &feat_track_set::size)
  .def("tracks", &feat_track_set::tracks)
  // this will require an override in an inheriting python class
  .def("__len__", &feat_track_set::size)
  // this will still require an override in an inheriting python class
  .def("clone", ([] (feat_track_set& self)
  {
    return self.clone(kv::clone_type::DEEP);
  }))
  .def("last_frame_features", &feat_track_set::last_frame_features)
  .def("last_frame_descriptors", &feat_track_set::last_frame_descriptors)
  .def("frame_features", &feat_track_set::frame_features, 
    py::arg("offset") = -1)
  .def("frame_descriptors", &feat_track_set::frame_descriptors,
    py::arg("offset") = -1)
  .def("frame_feature_track_states", &feat_track_set::frame_feature_track_states,
    py::arg("offset") = -1)
  .def("keyframes", &feat_track_set::keyframes)
  ;
}

kv::feature_set_sptr
feature_track_set_trampoline
::last_frame_features() const
{
  VITAL_PYBIND11_OVERLOAD(
    kv::feature_set_sptr,
    kv::feature_track_set,
    last_frame_features,
  );
}

kv::descriptor_set_sptr
feature_track_set_trampoline
::last_frame_descriptors() const
{
  VITAL_PYBIND11_OVERLOAD(
    kv::descriptor_set_sptr,
    kv::feature_track_set,
    last_frame_descriptors,
  );
}

kv::feature_set_sptr
feature_track_set_trampoline
::frame_features(kv::frame_id_t offset) const
{
  VITAL_PYBIND11_OVERLOAD(
    kv::feature_set_sptr,
    kv::feature_track_set,
    frame_features,
    offset
  );
}

kv::descriptor_set_sptr
feature_track_set_trampoline
::frame_descriptors(kv::frame_id_t offset) const
{
  VITAL_PYBIND11_OVERLOAD(
    kv::descriptor_set_sptr,
    kv::feature_track_set,
    frame_descriptors,
    offset
  );
}

std::vector< kv::feature_track_state_sptr >
feature_track_set_trampoline
::frame_feature_track_states(kv::frame_id_t offset) const
{
  VITAL_PYBIND11_OVERLOAD(
    std::vector< kv::feature_track_state_sptr >,
    kv::feature_track_set,
    frame_feature_track_states,
    offset
  );
}

std::set< kv::frame_id_t >
feature_track_set_trampoline
::keyframes() const
{
  VITAL_PYBIND11_OVERLOAD(
    std::set< kv::frame_id_t >,
    kv::feature_track_set,
    keyframes,
  );
}