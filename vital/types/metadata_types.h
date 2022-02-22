// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the declarations for the types of metadata values.

#ifndef KWIVER_VITAL_METADATA_TYPES_H_
#define KWIVER_VITAL_METADATA_TYPES_H_

#include <vital/vital_export.h>

#include <ostream>
#include <string>

#include <cstdint>

namespace kwiver {
namespace vital {

// ----------------------------------------------------------------------------
// TODO
class VITAL_EXPORT std_0102_lds { };

// ----------------------------------------------------------------------------
VITAL_EXPORT
std::ostream&
operator<<( std::ostream& os, std_0102_lds const& value );

// ----------------------------------------------------------------------------
/// Converts EG0104 datetime string to a UNIX timestamp.
///
/// \param value UTC datetime string in \c YYYYMMDDThhmmss format.
///
/// \returns Microseconds since Jan. 1, 1970 (UTC).
VITAL_EXPORT
uint64_t
std_0104_datetime_to_unix_timestamp( std::string const& value );

} } // end namespace

#endif
