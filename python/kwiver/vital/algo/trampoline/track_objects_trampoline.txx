// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file track_objects_trampoline.txx
 *
 * \brief trampoline for overriding virtual functions of
 *        algorithm_def<track_objects> and track_objects
 */

#ifndef TRACK_OBJECTS_TRAMPOLINE_TXX
#define TRACK_OBJECTS_TRAMPOLINE_TXX

#include <pybind11/pybind11.h>
#include <python/kwiver/vital/algo/trampoline/algorithm_trampoline.txx>
#include <vital/algo/track_objects.h>
#include <vital/types/detected_object_set.h>
#include <vital/types/image_container.h>
#include <vital/types/object_track_set.h>
#include <vital/types/timestamp.h>
#include <vital/types/homography_f2f.h>

namespace kwiver {
namespace vital  {
namespace python {

template <class algorithm_def_to_base=kwiver::vital::algorithm_def<kwiver::vital::algo::track_objects>>
class algorithm_def_to_trampoline :
      public algorithm_trampoline<algorithm_def_to_base>
{
  public:
    using algorithm_trampoline<algorithm_def_to_base>::algorithm_trampoline;

    std::string type_name() const override
    {
      PYBIND11_OVERLOAD(
        std::string,
        kwiver::vital::algorithm_def<kwiver::vital::algo::track_objects>,
        type_name,
      );
    }
};

template <class track_objects_base=kwiver::vital::algo::track_objects>
class track_objects_trampoline :
      public algorithm_def_to_trampoline<track_objects_base>
{
  public:
    using algorithm_def_to_trampoline<track_objects_base>::
              algorithm_def_to_trampoline;

    kwiver::vital::object_track_set_sptr
    track( kwiver::vital::timestamp ts,
           kwiver::vital::image_container_sptr image,
           kwiver::vital::detected_object_set_sptr detections ) const override
    {
      PYBIND11_OVERLOAD_PURE(
        kwiver::vital::object_track_set_sptr,
        kwiver::vital::algo::track_objects,
        track,
        ts,
        image,
        detections
      );
    }

    kwiver::vital::object_track_set_sptr
    track( kwiver::vital::timestamp ts,
           kwiver::vital::image_container_sptr image,
           kwiver::vital::detected_object_set_sptr detections,
           kwiver::vital::f2f_homography_sptr src_to_ref ) const override
    {
      PYBIND11_OVERLOAD(
        kwiver::vital::object_track_set_sptr,
        kwiver::vital::algo::track_objects,
        track,
        ts,
        image,
        detections,
        src_to_ref
      );
    }

    kwiver::vital::object_track_set_sptr
    track( kwiver::vital::timestamp ts,
           kwiver::vital::image_container_sptr image,
           kwiver::vital::detected_object_set_sptr detections,
           kwiver::vital::object_track_set_sptr existing_tracks ) const override
    {
      PYBIND11_OVERLOAD(
        kwiver::vital::object_track_set_sptr,
        kwiver::vital::algo::track_objects,
        track,
        ts,
        image,
        detections,
        existing_tracks
      );
    }

    kwiver::vital::object_track_set_sptr
    initialize( kwiver::vital::timestamp ts,
                kwiver::vital::image_container_sptr image,
                kwiver::vital::detected_object_set_sptr seed_detections ) const override
    {
      PYBIND11_OVERLOAD(
        kwiver::vital::object_track_set_sptr,
        kwiver::vital::algo::track_objects,
        initialize,
        ts,
        image,
        seed_detections
      );
    }

    kwiver::vital::object_track_set_sptr
    finalize() const override
    {
      PYBIND11_OVERLOAD(
        kwiver::vital::object_track_set_sptr,
        kwiver::vital::algo::track_objects,
        finalize,
      );
    }

    void reset() const override
    {
      PYBIND11_OVERLOAD(
        void,
        kwiver::vital::algo::track_objects,
        reset,
      );
    }
};
}
}
}
#endif
