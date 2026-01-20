// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file perform_text_query_trampoline.txx
 *
 * \brief trampoline for overriding virtual functions of
 *        algorithm_def<perform_text_query> and perform_text_query
 */

#ifndef PERFORM_TEXT_QUERY_TRAMPOLINE_TXX
#define PERFORM_TEXT_QUERY_TRAMPOLINE_TXX

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <python/kwiver/vital/algo/trampoline/algorithm_trampoline.txx>
#include <vital/algo/perform_text_query.h>

namespace kwiver {
namespace vital  {
namespace python {

template < class algorithm_def_ptq_base=
            kwiver::vital::algorithm_def<
              kwiver::vital::algo::perform_text_query > >
class algorithm_def_ptq_trampoline :
      public algorithm_trampoline<algorithm_def_ptq_base>
{
  public:
    using algorithm_trampoline<algorithm_def_ptq_base>::algorithm_trampoline;

    std::string type_name() const override
    {
      PYBIND11_OVERLOAD(
        std::string,
        kwiver::vital::algorithm_def<kwiver::vital::algo::perform_text_query>,
        type_name,
      );
    }
};

template< class perform_text_query_base=
                kwiver::vital::algo::perform_text_query >
class perform_text_query_trampoline :
      public algorithm_def_ptq_trampoline< perform_text_query_base >
{
  public:
    using algorithm_def_ptq_trampoline< perform_text_query_base>::
              algorithm_def_ptq_trampoline;

    std::vector< kwiver::vital::object_track_set_sptr >
    perform_query(
      std::string const& text_query,
      std::vector< kwiver::vital::image_container_sptr > const& images,
      std::vector< kwiver::vital::timestamp > const& timestamps,
      std::vector< kwiver::vital::object_track_set_sptr > const& input_tracks
    ) const override
    {
      PYBIND11_OVERLOAD_PURE(
        std::vector< kwiver::vital::object_track_set_sptr >,
        kwiver::vital::algo::perform_text_query,
        perform_query,
        text_query,
        images,
        timestamps,
        input_tracks
      );
    }
};

}
}
}

#endif
