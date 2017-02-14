/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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
 * \brief vital::algo::initialize_cameras_landmarks interface implementation
 */

#include "initialize_cameras_landmarks.h"

#include <vital/algo/initialize_cameras_landmarks.h>
#include <vital/bindings/c/helpers/algorithm.h>
#include <vital/bindings/c/helpers/camera_map.h>
#include <vital/bindings/c/helpers/landmark_map.h>
#include <vital/bindings/c/helpers/track_set.h>


DEFINE_COMMON_ALGO_API( initialize_cameras_landmarks )


using namespace kwiver;


/// Initialize the camera and landmark parameters given a set of tracks
void
vital_algorithm_initialize_cameras_landmarks_initialize( vital_algorithm_t *algo,
                                                         vital_camera_map_t **cameras,
                                                         vital_landmark_map_t **landmarks,
                                                         vital_trackset_t *tracks,
                                                         vital_error_handle_t *eh )
{
  STANDARD_CATCH(
    "vital_initialize_cameras_landmarks_initialize", eh,

    auto a_sptr = vital_c::ALGORITHM_initialize_cameras_landmarks_SPTR_CACHE.get( algo );
    auto cameras_sptr = vital_c::CAM_MAP_SPTR_CACHE.get( *cameras );
    auto landmarks_sptr = vital_c::LANDMARK_MAP_SPTR_CACHE.get( *landmarks );
    auto tracks_sptr = vital_c::TRACK_SET_SPTR_CACHE.get( tracks );

    a_sptr->initialize( cameras_sptr, landmarks_sptr, tracks_sptr );

    // Check instance pointer for cameras and landmarks for being different from
    // input for caching.
    auto *cameras_after = reinterpret_cast< vital_camera_map_t* >( cameras_sptr.get() );
    if( *cameras != cameras_after )
    {
      vital_c::CAM_MAP_SPTR_CACHE.store( cameras_sptr );
      *cameras = cameras_after;
    }

    auto *landmarks_after = reinterpret_cast< vital_landmark_map_t* >( landmarks_sptr.get() );
    if( *landmarks != landmarks_after )
    {
      vital_c::LANDMARK_MAP_SPTR_CACHE.store( landmarks_sptr );
      *landmarks = landmarks_after;
    }

  );
}
