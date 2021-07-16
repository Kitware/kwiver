// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the declarations for the types of metadata values.

#ifndef KWIVER_VITAL_METADATA_TYPES_H_
#define KWIVER_VITAL_METADATA_TYPES_H_

#include <vital/vital_export.h>

#include <ostream>

namespace kwiver {
namespace vital {

// ----------------------------------------------------------------------------
// TODO
class VITAL_EXPORT std_0102_lds { };

// ----------------------------------------------------------------------------
VITAL_EXPORT
std::ostream&
operator<<( std::ostream& os, std_0102_lds const& value );

} } // end namespace

#endif
