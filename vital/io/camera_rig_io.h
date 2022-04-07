// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header file for loading camera collections

#ifndef VITAL_CAMERA_MAP_IO_H_
#define VITAL_CAMERA_RIG_IO_H_

#include <vital/types/camera_rig.h>

#include <vital/vital_types.h>

#include <vector>

namespace  kwiver {
namespace vital {

/// Load a camera rig from krtd files.
///
/// \throws invalid_data
///   Unable to find any camera krtd files in the given directory
/// \throw path_not_exists
///   The specified directory does not exist
///
/// \param cam_files a list of krtd file names
/// \return a new camera collection created after parsing all krtd files
camera_rig_sptr
VITAL_EXPORT read_camera_rig( path_list_t const& cam_files );

} // vital
} // kwiver

#endif // VITAL_CAMERA_MAP_IO_H_
