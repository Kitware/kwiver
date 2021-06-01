// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef VITAL_TYPE_CASTER
#define VTIAL_TYPE_CASTER

#include <vital/optional.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace kw = kwiver::vital;

namespace pybind11 {

namespace detail {

template < typename T >
struct type_caster< kw::optional< T > >
  : optional_caster< kw::optional< T > >
{
};

} // namespace detail

} // namespace pybind11

#endif
