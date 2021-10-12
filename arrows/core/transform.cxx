/*ckwg +29
 * Copyright 2014-2019 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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
 * \brief Implementation of kwiver::arrows::core::transform functions to apply
 * similarity transformations
 */

#include "transform.h"
#include <Eigen/Geometry>


namespace kwiver {
namespace arrows {
namespace core {


/// Transform the camera by applying a similarity transformation in place
void
transform_inplace(vital::simple_camera_perspective& cam,
                  const vital::similarity_d& xform)
{
  cam.set_center( xform * cam.get_center() );
  cam.set_rotation( cam.get_rotation() * xform.rotation().inverse() );
  cam.set_center_covar( transform(cam.get_center_covar(), xform) );
}


/// Transform the camera map by applying a similarity transformation in place
void transform_inplace(vital::simple_camera_perspective_map& cameras,
                       const vital::similarity_d& xform)
{
  auto cam_map = cameras.T_cameras();
  for (auto& p : cam_map)
  {
    transform_inplace(*p.second, xform);
  }
}


/// Transform the landmark by applying a similarity transformation in place
template <typename T>
void
transform_inplace(vital::landmark_<T>& lm,
                  const vital::similarity_<T>& xform)
{
  lm.set_loc( xform * lm.get_loc() );
  lm.set_scale( lm.get_scale() * xform.scale() );
  lm.set_covar( transform(lm.get_covar(), xform) );
}


/// Transform the landmark map by applying a similarity transformation in place
void transform_inplace(vital::landmark_map& landmarks,
                       const vital::similarity_d& xform)
{
  vital::landmark_map::map_landmark_t lm_map = landmarks.landmarks();
  transform_inplace(lm_map, xform);
}


/// Transform the landmark map by applying a similarity transformation in place
void transform_inplace(vital::landmark_map::map_landmark_t& landmarks,
                       const vital::similarity_d& xform)
{
  for (vital::landmark_map::map_landmark_t::value_type& p : landmarks)
  {
    if (vital::landmark_d* vlm = dynamic_cast<vital::landmark_d*>(p.second.get()))
    {
      transform_inplace(*vlm, xform);
    }
    else if (vital::landmark_f* vlm = dynamic_cast<vital::landmark_f*>(p.second.get()))
    {
      transform_inplace(*vlm, vital::similarity_f(xform));
    }
  }
}


/// Transform a 3D covariance matrix with a similarity transformation
template <typename T>
vital::covariance_<3,T> transform(const vital::covariance_<3,T>& covar,
                                  const vital::similarity_<T>& xform)
{
  // TODO trasform covariance parameters directly
  // instead of converting to matrix form and back
  Eigen::Matrix<T,3,3> C(covar.matrix());
  Eigen::Matrix<T,3,3> sR(xform.rotation().matrix());
  sR /= xform.scale();
  C = sR * C * sR.transpose();
  return vital::covariance_<3,T>(C);
}


/// construct a transformed camera by applying a similarity transformation
vital::camera_perspective_sptr transform(vital::camera_perspective_sptr cam,
                                         const vital::similarity_d& xform)
{
  cam = std::dynamic_pointer_cast<vital::camera_perspective>(cam->clone());
  if( vital::simple_camera_perspective* vcam =
      dynamic_cast<vital::simple_camera_perspective*>(cam.get()) )
  {
    transform_inplace(*vcam, xform);
  }
  else
  {
    vital::simple_camera_perspective* new_cam =
        new vital::simple_camera_perspective( xform * cam->center(),
                                  cam->rotation() * xform.rotation().inverse(),
                                  cam->intrinsics() );
    new_cam->set_center_covar( transform(cam->center_covar(), xform) );
    cam = vital::camera_perspective_sptr( new_cam );
  }
  return cam;
}


/// construct a transformed map of cameras by applying a similarity transformation
vital::camera_map_sptr transform(vital::camera_map_sptr cameras,
                                 const vital::similarity_d& xform)
{
  vital::camera_map::map_camera_t cam_map = cameras->cameras();
  for(vital::camera_map::map_camera_t::value_type& p : cam_map)
  {
    auto cam_ptr = std::dynamic_pointer_cast<vital::camera_perspective>(p.second);
    if (!cam_ptr)
    {
      p.second = nullptr;
      continue;
    }
    p.second = transform(cam_ptr, xform);
  }
  return vital::camera_map_sptr(new vital::simple_camera_map(cam_map));
}


/// construct a transformed map of cameras by applying a similarity transformation
vital::camera_perspective_map_sptr
transform(vital::camera_perspective_map_sptr cameras,
          const vital::similarity_d& xform)
{
  auto cam_map = cameras->T_cameras();
  for (auto& p : cam_map)
  {
    p.second = transform(p.second, xform);
  }
  return std::make_shared<vital::camera_perspective_map>(cam_map);
}


/// construct a transformed landmark by applying a similarity transformation
vital::landmark_sptr transform(vital::landmark_sptr lm,
                               const vital::similarity_d& xform)
{
  if (!lm)
  {
    return vital::landmark_sptr();
  }
  lm = lm->clone();
  if( vital::landmark_d* vlm = dynamic_cast<vital::landmark_d*>(lm.get()) )
  {
    transform_inplace(*vlm, xform);
  }
  else if( vital::landmark_f* vlm = dynamic_cast<vital::landmark_f*>(lm.get()) )
  {
    transform_inplace(*vlm, vital::similarity_f(xform));
  }
  else
  {
    auto new_lm = std::make_shared<vital::landmark_d>( *lm );
    new_lm->set_loc( xform * lm->loc() );
    new_lm->set_scale( lm->scale() * xform.scale() );
    new_lm->set_covar( transform(lm->covar(), xform) );
    lm = new_lm;
  }
  return lm;
}


/// construct a transformed map of landmarks by applying a similarity transformation
vital::landmark_map_sptr transform(vital::landmark_map_sptr landmarks,
                                   const vital::similarity_d& xform)
{
  vital::landmark_map::map_landmark_t lm_map = landmarks->landmarks();
  for(vital::landmark_map::map_landmark_t::value_type& p : lm_map)
  {
    p.second = transform(p.second, xform);
  }
  return vital::landmark_map_sptr(new vital::simple_landmark_map(lm_map));
}


/// translate landmarks in place by the provided offset vector
void translate_inplace(vital::landmark_map& landmarks,
                       vital::vector_3d const& offset)
{
  for (auto lm : landmarks.landmarks())
  {
    auto lmd = std::dynamic_pointer_cast<vital::landmark_d>(lm.second);
    if (lmd)
    {
      lmd->set_loc(lm.second->loc() + offset);
    }
    else
    {
      auto lmf = std::dynamic_pointer_cast<vital::landmark_f>(lm.second);
      if (lmf)
      {
        lmf->set_loc((lm.second->loc() + offset).cast<float>());
      }
    }
  }
}


/// translate cameras in place by the provided offset vector
void translate_inplace(vital::simple_camera_perspective_map& cameras,
                       vital::vector_3d const& offset)
{
  for (auto cam : cameras.T_cameras())
  {
    auto cam_ptr = cam.second;
    if (cam_ptr)
    {
      cam_ptr->set_center(cam_ptr->center() + offset);
    }
  }
}


/// translate cameras in place by the provided offset vector
void translate_inplace(vital::camera_map& cameras,
                       vital::vector_3d const& offset)
{
  vital::simple_camera_perspective_map pcameras;
  pcameras.set_from_base_camera_map(cameras.cameras());
  kwiver::arrows::core::translate_inplace(pcameras, offset);
}


/// \cond DoxygenSuppress
#define INSTANTIATE_TRANSFORM(T) \
template KWIVER_ALGO_CORE_EXPORT vital::covariance_<3,T> \
transform(const vital::covariance_<3,T>& covar, \
          const vital::similarity_<T>& xform); \
template KWIVER_ALGO_CORE_EXPORT void \
transform_inplace(vital::landmark_<T>& cam, \
                  const vital::similarity_<T>& xform);

INSTANTIATE_TRANSFORM(double);
INSTANTIATE_TRANSFORM(float);

#undef INSTANTIATE_TRANSFORM
/// \endcond


} // end namespace core
} // end namespace arrows
} // end namespace kwiver
