// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definitions of type aliases used in the KLV system.

#include <vector>

#include <cstdint>

namespace kwiver {

namespace arrows {

namespace klv {

using klv_read_iter_t = uint8_t const*;
using klv_write_iter_t = uint8_t*;
using klv_bytes_t = std::vector< uint8_t >;

} // namespace klv

} // namespace arrows

} // namespace kwiver
