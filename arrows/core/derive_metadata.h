// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header for metadata derivation utility functions.

#ifndef KWIVER_ARROWS_CORE_DERIVE_METADATA_H_
#define KWIVER_ARROWS_CORE_DERIVE_METADATA_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/video_input.h>

#include <vital/types/bounding_box.h>
#include <vital/types/camera_perspective.h>
#include <vital/types/camera_perspective_map.h>
#include <vital/types/landmark.h>

#include <functional>
#include <vector>

namespace kwiver {

namespace arrows {

namespace core {

/// Fills in metadata values which can be calculated from other metadata.
///
/// \param metadata_vec A vector of metadata packets.
/// \param frame_width Width of the image in pixels.
/// \param frame_height Height of the image in pixels.
///
/// \returns An amended metadata vector.
///
/// \todo
///   Eventually this should be replaced with a version which takes in an image
///   as a parameter instead of just frame dimensions and incorporates pixel
///   data into computations.
KWIVER_ALGO_CORE_EXPORT
kwiver::vital::metadata_vector
compute_derived_metadata( kwiver::vital::metadata_vector const& metadata_vec,
                          size_t frame_width, size_t frame_height );

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
