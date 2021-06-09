// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Header for metadata derivation utility functions.
 */

#ifndef DERIVE_METADATA_H_
#define DERIVE_METADATA_H_

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

/**
 * \param metadata_vec a vector of metadata packets
 * TODO fix the fact that a shallow copy is being updated
 */
KWIVER_ALGO_CORE_EXPORT
kwiver::vital::metadata_vector
compute_derived_metadata( kwiver::vital::metadata_vector metadata_vec );

} } } // end namespace

#endif
