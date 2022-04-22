// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface to the KLV UUID parser.

#ifndef KWIVER_ARROWS_KLV_KLV_UUID_H_
#define KWIVER_ARROWS_KLV_KLV_UUID_H_

#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <array>
#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_KLV_EXPORT klv_uuid
{
  klv_uuid();

  klv_uuid( std::initializer_list< uint8_t > const& bytes );

  klv_uuid( std::array< uint8_t, 16 > const& bytes );

  std::array< uint8_t, 16 > bytes;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_uuid const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_uuid )

// ----------------------------------------------------------------------------
template < class Iterator >
klv_uuid
klv_read_uuid( Iterator& data, size_t max_length );

// ----------------------------------------------------------------------------
template < class Iterator >
void
klv_write_uuid( klv_uuid const& value, Iterator& data,
                size_t max_length );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
size_t
klv_uuid_length();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
