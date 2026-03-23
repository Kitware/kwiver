// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file refine_tracks_trampoline.txx
 *
 * \brief trampoline for overriding virtual functions of
 *        algorithm_def<refine_tracks> and refine_tracks
 */

#ifndef REFINE_TRACKS_TRAMPOLINE_TXX
#define REFINE_TRACKS_TRAMPOLINE_TXX

#include <pybind11/pybind11.h>
#include <python/kwiver/vital/algo/trampoline/algorithm_trampoline.txx>
#include <vital/algo/refine_tracks.h>
#include <vital/types/object_track_set.h>
#include <vital/types/image_container.h>
#include <vital/types/timestamp.h>

namespace kwiver {
namespace vital  {
namespace python {

template <class algorithm_def_rt_base=kwiver::vital::algorithm_def<kwiver::vital::algo::refine_tracks>>
class algorithm_def_rt_trampoline :
      public algorithm_trampoline<algorithm_def_rt_base>
{
  public:
    using algorithm_trampoline<algorithm_def_rt_base>::algorithm_trampoline;

    std::string type_name() const override
    {
      PYBIND11_OVERLOAD(
        std::string,
        kwiver::vital::algorithm_def<kwiver::vital::algo::refine_tracks>,
        type_name,
      );
    }
};

template <class refine_tracks_base=kwiver::vital::algo::refine_tracks>
class refine_tracks_trampoline :
      public algorithm_def_rt_trampoline<refine_tracks_base>
{
  public:
    using algorithm_def_rt_trampoline<refine_tracks_base>::
              algorithm_def_rt_trampoline;

    kwiver::vital::object_track_set_sptr
    refine( kwiver::vital::timestamp ts,
            kwiver::vital::image_container_sptr image_data,
            kwiver::vital::object_track_set_sptr tracks ) const override
    {
      PYBIND11_OVERLOAD_PURE(
        kwiver::vital::object_track_set_sptr,
        kwiver::vital::algo::refine_tracks,
        refine,
        ts,
        image_data,
        tracks
      );
    }

    kwiver::vital::object_track_set_sptr finalize() const override
    {
      PYBIND11_OVERLOAD(
        kwiver::vital::object_track_set_sptr,
        kwiver::vital::algo::refine_tracks,
        finalize,
      );
    }
};

}
}
}
#endif
