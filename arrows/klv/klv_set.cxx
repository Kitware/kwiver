// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Instantiations of \c klv_set and \c klv_set_format.

#include "klv_set.h"

namespace kwiver {

namespace arrows {

namespace klv {

template class klv_set< klv_lds_key >;
template class klv_set< klv_uds_key >;

template class klv_set_format< klv_lds_key >;
template class klv_set_format< klv_uds_key >;

} // namespace klv

} // namespace arrows

} // namespace kwiver
