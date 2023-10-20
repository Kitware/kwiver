// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_segments.h"

namespace ka = kwiver::arrows;

#include <vital/exceptions/base.h>

#include <ostream>
#include <initializer_list>
#include <vector>

namespace kwiver {

namespace arrows {

namespace stanag {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_segment_header const& value )
{
    os << "{ "
       << "Segment Type: " << value.segment_type << ", "
       << "Segment Size: " << value.segment_size
       << " }";

    return os;
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_segment_header,
  &stanag_4607_segment_header::segment_type,
  &stanag_4607_segment_header::segment_size
)

// ----------------------------------------------------------------------------
stanag_4607_segment_header_format
::stanag_4607_segment_header_format()
{}

// ----------------------------------------------------------------------------
stanag_4607_segment_header
stanag_4607_segment_header_format
::read( ptr_t& ptr ) const
{
    stanag_4607_segment_header result;

    result.segment_type = static_cast< stanag_4607_segment_type >(
        ka::klv::klv_read_int< uint64_t >( ptr, 1 ));
    result.segment_size = ka::klv::klv_read_int< size_t >( ptr, 4 );

    return result;
}

} // namespace stanag

} // namespace arrows

} // namespace kwiver
