/*ckwg +29
* Copyright 2018 by Kitware, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  * Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  * Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  * Neither name of Kitware, Inc. nor the names of any contributors may be used
*    to endorse or promote products derived from this software without specific
*    prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

  /// get the metadata specified camera position in the local coordinate frame
  /**
  * \param[in]  fid the frame to get the positionfor
  * \pramm[out] pos_loc the local coordinate frame position prior
  * \return true if position prior is recovered from metadata, false otherwise
  */
  bool get_camera_position_prior_local(frame_id_t fid, vector_3d &pos_loc) const;

  /// get the metadata specified camera orientation prior in the local coordinate frame
  /**
  * \param[in] fid the frame to get the orientaiton for
  * \param[out] R_loc the rotation in the local frame according to the metadata
  * \return return true if orientation prior is recovered from metadata, false otherwise
  */
  bool get_camera_orientation_prior_local(frame_id_t fid, rotation_d &R_loc) const;

  typedef std::map<frame_id_t, vector_3d> position_map;

  /// get the camera position prior map
  position_map get_camera_position_priors() const;

  ///  store the image size for a particular frame
  /**
  * \param[in] fid the frame whose image size we will store
  * \param[in] image_width the width of the image
  * \param[in] image_height the height of the image
  */
  void store_image_size(frame_id_t fid, int image_width, int image_height);

  /// get the image width
  /**
  * \param[in] fid the frame
  * \param[out] image_width the width of the image with frame id fid
  * \return true if the image width is recovered from the constraints, false otherwise
  */
  bool get_image_width(frame_id_t fid, int &image_width) const;

  /// get the image height
  /**
  * \param[in] fid the frame
  * \param[out] image_height the height of the image with frame id fid
  * \return true if the image height is recovered from the constraints, false otherwise
  */
  bool get_image_height(frame_id_t fid, int &image_height) const;

  /// get the focal length estimate from the metadata
  /**
  * \param[in] fid the frame whose focal length we want to recover
  * \param[out] focal_length the focal length according to the metadata
  * \return true if the focal length prior is found in the metadata, false otherwise
  */
  bool get_focal_length_prior(frame_id_t fid, float &focal_length) const;

protected:

  class priv;
  const std::unique_ptr<priv> m_priv;

};

typedef std::shared_ptr<sfm_constraints> sfm_constraints_sptr;

}} ///end namespace kwiver vital
#endif
