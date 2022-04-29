// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declaration of KLV-vital conversion functions.

#include "klv_timeline.h"

#include <vital/types/metadata.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Create a \c metadata object from the KLV data present at \p timestamp.
///
/// \note Not all information is preserved in the conversion, nor is it meant
/// to be. This function exists merely to allow access to some
/// computer-vision-relevant metadata for each frame image using the unified
/// `vital` interface, not to transform the `klv_timeline` wholesale. If
/// re-encoding into KLV or comprehensive export of all available metadata is
/// desired, the original `klv_timeline` or `klv_packet`s should be preserved
/// for that purpose.
///
/// \param klv_data Timeline of KLV data.
/// \param timestamp
///   MISP Precision Timestamp (microseconds) denoting at what point the
///   'snapshot' of KLV metadata should be taken. Ideally this should be
///   derived from a MISP timestamp embedded in the video file.
///
/// \return The vital-friendly metadata which can be extracted from
///         \p klv_data.
KWIVER_ALGO_KLV_EXPORT
kwiver::vital::metadata_sptr
klv_to_vital_metadata( klv_timeline const& klv_data, uint64_t timestamp );

} // namespace klv

} // namespace arrows

} // namespace kwiver
