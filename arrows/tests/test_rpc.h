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
 * \brief Sets up known landmarks for rpc related tests
 */

#ifndef KWIVER_ARROWS_TESTS_TEST_RPC_H_
#define KWIVER_ARROWS_TESTS_TEST_RPC_H_

#include <vital/types/camera_rpc.h>
#include <vital/types/landmark_map.h>
#include <vital/types/vector.h>

namespace kwiver {
namespace testing {

// construct a map of landmarks at the corners of a cube centered at c
// with a side length of s
vital::landmark_map_sptr
rpc_landmarks()
{
  vital::landmark_map::map_landmark_t landmark_map;
  std::vector< vital::vector_3d > lm_pos;
  lm_pos.push_back( vital::vector_3d( -117.237465, 32.881208, 110.0 ) );
  lm_pos.push_back( vital::vector_3d( -117.235309, 32.879108, 110.0 ) );
  lm_pos.push_back( vital::vector_3d( -117.239404, 32.877824, 110.0 ) );
  lm_pos.push_back( vital::vector_3d( -117.236088, 32.877091, 110.0 ) );
  lm_pos.push_back( vital::vector_3d( -117.240455, 32.876183, 110.0 ) );

  for ( size_t i = 0; i < lm_pos.size(); ++i )
  {
    auto landmark_ptr = std::make_shared< vital::landmark_< double > >( lm_pos[i] );
    landmark_map.insert(
      std::pair< vital::landmark_id_t, vital::landmark_sptr >(i, landmark_ptr ) );
  }

  return std::make_shared< vital::simple_landmark_map >( landmark_map );
}

// add Gaussian noise to RPC camera coefficients
kwiver::vital::camera_map_sptr
noisy_rpc_cameras( kwiver::vital::camera_map_sptr cameras,
                   double stdev = 1.0,
                   int order = 1,
                   bool image_norm = false,
                   bool world_norm = false )
{
  using namespace kwiver::vital;

  camera_map::map_camera_t cam_map;
  for( camera_map::map_camera_t::value_type const& p : cameras->cameras() )
  {
    auto cam_ptr = std::dynamic_pointer_cast<vital::camera_rpc>(p.second);
    auto c = std::dynamic_pointer_cast<vital::camera_rpc>(cam_ptr->clone());

    unsigned int n;
    switch ( order )
    {
      case -1:
        n = 0;
        break;
      case 0:
        n = 1;
        break;
      case 1:
        n = 4;
        break;
      case 2:
        n = 10;
        break;
      default:
        n = 20;
        break;
    }

    simple_camera_rpc& cam = dynamic_cast<simple_camera_rpc&>(*c);

    auto new_coeffs = cam.rpc_coeffs();

    for ( size_t i = 0; i < n; ++i )
    {
      new_coeffs.block< 4, 1 >( 0, i ) += random_matrix<double, 4, 1>(stdev);
    }

    cam.set_rpc_coeffs( new_coeffs );

    if ( image_norm )
    {
      cam.set_image_scale( cam.image_scale() + random_point2d(stdev) );
      cam.set_image_offset( cam.image_offset() + random_point2d(stdev) );
    }

    if ( world_norm )
    {
      cam.set_world_scale( cam.world_scale() + random_point3d(stdev) );
      cam.set_world_offset( cam.world_offset() + random_point3d(stdev) );
    }

    cam_map[p.first] = c;
  }
  return camera_map_sptr( new simple_camera_map( cam_map ) );
}

}
}

#endif // KWIVER_ARROWS_TESTS_TEST_RPC_H_