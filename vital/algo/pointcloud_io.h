// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface for pointcloud_io

#ifndef VITAL_POINTCLOUD_IO_H_
#define VITAL_POINTCLOUD_IO_H_

#include <vital/algo/algorithm.h>
#include <vital/types/landmark_map.h>
#include <vital/types/local_geo_cs.h>
#include <vital/types/pointcloud.h>

#include <fstream>
#include <string>

namespace kwiver {
namespace vital {
namespace algo {
class VITAL_ALGO_EXPORT pointcloud_io
  : public kwiver::vital::algorithm_def< pointcloud_io >
{
public:
  virtual
  ~pointcloud_io() = default;

  /// Return the name of this algorithm
  static std::string
  static_type_name() { return "pointcloud_io"; }

  /// Load point cloud from the file
  ///
  /// \throws kwiver::vital::path_not_exists Thrown when the given path does
  ///    not exist.
  ///
  /// \throws kwiver::vital::path_not_a_file Thrown when the given path does
  ///    not point to a file (i.e. it points to a directory).
  ///
  /// \param filename the path to the file to load
  /// \returns an image container refering to the loaded image
  kwiver::vital::pointcloud_d load(
    std::string const& filename ) const;

  /// Save pointcloud to a file
  ///
  /// \throws kwiver::vital::path_not_exists Thrown when the given path's
  ///    containing directory does not exist.
  ///
  /// \throws kwiver::vital::path_not_a_directory Thrown when the given
  ///    path's containing directory is not a directory.
  ///
  /// \throws kwiver::vital::path_not_a_file Thrown when the given path does
  ///    not point to a file (i.e. it points to a directory).
  ///
  /// \param filename the path to the file to save
  /// \param points the vector of points to write
  /// \param colors optional vector of colors corresponding to the points
  void save( std::string const& filename,
             std::vector< vital::vector_3d > const& points,
             std::vector< vital::rgb_color > const& colors = {} );

  /// Save pointcloud to a file from a landmark map
  ///
  /// \param filename the path to the file to save
  /// \param landmarks the landmark map to read from
  void save( std::string const& filename,
             vital::landmark_map_sptr const& landmarks );

  /// Set local geo coordinate system for the pointcloud
  ///
  /// \param lgcs the target local geo coordinate system
  virtual void set_local_geo_cs( vital::local_geo_cs const& lgcs );

protected:
  pointcloud_io();

private:
  virtual pointcloud_d load_( vital::path_t const& filename ) const = 0;

  virtual void save_( vital::path_t const& filename,
                      std::vector< vital::vector_3d > const& points,
                      std::vector< vital::rgb_color > const& colors ) const = 0;
};

/// Shared pointer type for generic write_object_track_set definition type.
typedef std::shared_ptr< pointcloud_io > pointcloud_io_sptr;
} // namespace algo
} // namespace vital
} // namespace kwiver

#endif // VITAL_POINTCLOUD_IO_H_
