// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header file for loading camera collections

#ifndef VITAL_CAMERA_RIG_IO_H_
#define VITAL_CAMERA_RIG_IO_H_

#include <vital/types/camera_rig.h>

#include <vital/vital_types.h>

#include <vector>

namespace  kwiver {
namespace vital {

/// Load a camera rig from KRtd file(s).
///
/// \throws invalid_data
///   Unable to find any camera files in the given directory
/// \throw path_not_exists
///   The specified directory does not exist
///
/// \param cam_files a list of camera names
/// \return a new camera rig
camera_rig_sptr
VITAL_EXPORT read_camera_rig( path_list_t const& cam_files );

/// Load a stereo rig from a file or directory.
///
/// Supports the following formats:
/// - .json: JSON format with camera intrinsics and extrinsics
/// - .yml/.yaml: OpenCV YAML format with camera matrices
/// - .npz: NumPy compressed archive (requires ZLIB)
/// - directory: OpenCV calibration directory with intrinsics.yml and extrinsics.yml
///
/// \throws invalid_data
///   Unable to find any camera files in the given directory
/// \throw path_not_exists
///   The specified directory does not exist
///
/// \param FN input file name or directory path
/// \return a new stereo rig
camera_rig_stereo_sptr
VITAL_EXPORT read_stereo_rig( path_t const& FN );

/// Load a stereo rig from an OpenCV calibration directory.
///
/// Expects a directory containing intrinsics.yml and extrinsics.yml files
/// in OpenCV FileStorage format with matrices M1, D1, M2, D2, R, T.
///
/// \param dir_path path to the calibration directory
/// \return a new stereo rig
camera_rig_stereo_sptr
VITAL_EXPORT read_stereo_rig_from_ocv_dir( path_t const& dir_path );

/// Save a camera rig to KRtd file(s)
///
/// \throws invalid_data
///   Unable to find any camera files in the given directory
/// \throw path_not_exists
///   The specified directory does not exist
///
/// \param rig camera rig
/// \param ext type of output file(s): .krtd
void
VITAL_EXPORT write_camera_rig( camera_rig_sptr rig );

/// Save a stereo rig to a file: .json
///
/// \throws invalid_data
///   Unable to find any camera files in the given directory
/// \throw path_not_exists
///   The specified directory does not exist
///
/// \param rig stereo rig
/// \param FN output file name
void
VITAL_EXPORT write_stereo_rig( camera_rig_stereo_sptr rig,
                               std::string const & FN );

} // vital
} // kwiver

#endif // VITAL_CAMERA_MAP_IO_H_
