// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file segment_via_points_trampoline.txx
 *
 * \brief trampoline for overriding virtual functions of
 *        algorithm_def<segment_via_points> and segment_via_points
 */

#ifndef SEGMENT_VIA_POINTS_TRAMPOLINE_TXX
#define SEGMENT_VIA_POINTS_TRAMPOLINE_TXX

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <python/kwiver/vital/algo/trampoline/algorithm_trampoline.txx>
#include <vital/algo/segment_via_points.h>

namespace kwiver {
namespace vital  {
namespace python {

template < class algorithm_def_svp_base=
            kwiver::vital::algorithm_def<
              kwiver::vital::algo::segment_via_points > >
class algorithm_def_svp_trampoline :
      public algorithm_trampoline<algorithm_def_svp_base>
{
  public:
    using algorithm_trampoline<algorithm_def_svp_base>::algorithm_trampoline;

    std::string type_name() const override
    {
      PYBIND11_OVERLOAD(
        std::string,
        kwiver::vital::algorithm_def<kwiver::vital::algo::segment_via_points>,
        type_name,
      );
    }
};

template< class segment_via_points_base=
                kwiver::vital::algo::segment_via_points >
class segment_via_points_trampoline :
      public algorithm_def_svp_trampoline< segment_via_points_base >
{
  public:
    using algorithm_def_svp_trampoline< segment_via_points_base>::
              algorithm_def_svp_trampoline;

    kwiver::vital::detected_object_set_sptr
    segment(
      kwiver::vital::image_container_sptr image,
      std::vector< kwiver::vital::point_2d > const& points,
      std::vector< int > const& point_labels
    ) const override
    {
      PYBIND11_OVERLOAD_PURE(
        kwiver::vital::detected_object_set_sptr,
        kwiver::vital::algo::segment_via_points,
        segment,
        image,
        points,
        point_labels
      );
    }
};

}
}
}

#endif
