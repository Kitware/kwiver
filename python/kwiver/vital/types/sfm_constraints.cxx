// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/types/sfm_constraints.h>

#include <python/kwiver/vital/types/type_casters.h>

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>

namespace py = pybind11;
namespace kv = kwiver::vital;

PYBIND11_MODULE( sfm_constraints, m )
{
  py::class_< kv::sfm_constraints,
              std::shared_ptr< kv::sfm_constraints > >( m, "SFMConstraints" )
  .def( py::init<>() )
  .def( py::init< kv::sfm_constraints const& >() )
  .def( py::init< kv::metadata_map_sptr, kv::local_geo_cs const& >() )
  .def_property( "metadata",
                  &kv::sfm_constraints::get_metadata,
                  &kv::sfm_constraints::set_metadata )
  .def_property( "local_geo_cs",
                  &kv::sfm_constraints::get_local_geo_cs,
                  &kv::sfm_constraints::set_local_geo_cs )
  .def( "get_camera_position_prior_local",
        &kv::sfm_constraints::get_camera_position_prior_local )
  .def( "get_camera_orientation_prior_local",
        &kv::sfm_constraints::get_camera_orientation_prior_local )
  .def( "get_camera_position_priors", &kv::sfm_constraints::get_camera_position_priors )
  .def( "store_image_size", &kv::sfm_constraints::store_image_size )
  .def( "get_image_width", &kv::sfm_constraints::get_image_width)
  .def( "get_image_height", &kv::sfm_constraints::get_image_height)
  .def( "get_focal_length_prior", &kv::sfm_constraints::get_focal_length_prior)
  ;
}
