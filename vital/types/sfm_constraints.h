// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
* \file
* \brief Header for kwiver::vital::sfm_constraints class storing constraints to be
*        used in SfM.
*/

#include <vital/vital_export.h>
#include <vital/vital_config.h>

#include <vital/types/metadata_map.h>
#include <vital/types/rotation.h>
#include <vital/types/local_geo_cs.h>

#include <vital/optional.h>

#ifndef KWIVER_VITAL_SFM_CONSTRAINTS_H_
#define KWIVER_VITAL_SFM_CONSTRAINTS_H_

namespace kwiver {

namespace vital {

class VITAL_EXPORT sfm_constraints {
public:

  sfm_constraints();

  sfm_constraints(const sfm_constraints& other);

  /// constructor
  /**
  * \param[in] md metadata map sptr to store
  * \param[in] lgcs the local geo coordinate system
  */
  sfm_constraints(
    metadata_map_sptr md,
    local_geo_cs const& lgcs);

  /// desctructor
  ~sfm_constraints();

  /// returns a pointer to the metadata map
  metadata_map_sptr get_metadata();

  /// set the metadata
  /**
  * \param[in] md metadata map sptr to store
  */
  void set_metadata(metadata_map_sptr md);

  /// returns the local geo coodinate system
  local_geo_cs get_local_geo_cs();

  /// sets the local geo coordinate system
  /**
  * \param[in] lgcs the local geo coordinate system to set
  */
  void set_local_geo_cs(local_geo_cs const& lgcs);

  /// Get the metadata specified camera position prior in the local coordinate
  /// frame.
  ///
  /// \param fid The frame for which to get the position.
  /// \return
  ///   The local coordinate frame position prior if recoverable from the
  ///   metadata, otherwise a disengaged optional.
  optional<vector_3d> get_camera_position_prior_local(frame_id_t fid) const;

  /// Get the metadata specified camera orientation prior in the local
  /// coordinate frame.
  ///
  /// \param fid The frame for which to get the orientation.
  /// \return
  ///   The local coordinate frame orientation prior (as rotation) if
  ///   recoverable from the metadata, otherwise a disengaged optional.
  optional<rotation_d> get_camera_orientation_prior_local(
    frame_id_t fid) const;

  typedef std::map<frame_id_t, vector_3d> position_map;

  /// get the camera position prior map
  position_map get_camera_position_priors() const;

  /// Store the image size for a particular frame.
  ///
  /// \param fid The frame for which to store the image size.
  /// \param image_width The width of the image.
  /// \param image_height The height of the image.
  void store_image_size(
    frame_id_t fid, unsigned image_width, unsigned image_height);

  /// Get the image width.
  ///
  /// \param fid The frame for which to get the image width.
  /// \return
  ///   The width of the image with frame id \p fid if provided by the
  ///   constraints, otherwise a disengaged optional.
  optional<unsigned> get_image_width(frame_id_t fid) const;

  /// Get the image height.
  ///
  /// \param fid The frame for which to get the image height.
  /// \return
  ///   The height of the image with frame id \p fid if provided by the
  ///   constraints, otherwise a disengaged optional.
  optional<unsigned> get_image_height(frame_id_t fid) const;

  /// Get the focal length estimate from the metadata.
  ///
  /// \param fid The frame whose focal length we want to recover.
  /// \return
  ///   The focal length according to the metadata, if provided, otherwise a
  ///   disengaged optional.
  optional<float> get_focal_length_prior(frame_id_t fid) const;

protected:
  class priv;
  const std::unique_ptr<priv> m_priv;
};

typedef std::shared_ptr<sfm_constraints> sfm_constraints_sptr;

} // namespace vital

} // namespace kwiver

#endif
