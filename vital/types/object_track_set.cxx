/*ckwg +29
 * Copyright 2013-2017 by Kitware, Inc.
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
 * \brief Implementation of \link kwiver::vital::track_set track_set \endlink
 *        member functions
 */

#include "object_track_set.h"

#include <limits>


namespace kwiver {
namespace vital {

typedef std::unique_ptr< track_set_implementation > tsi_uptr;

/// Default Constructor
object_track_set
::object_track_set()
  : track_set( tsi_uptr( new simple_track_set_implementation ) )
{
}

/// Constructor specifying the implementation
object_track_set
::object_track_set( std::unique_ptr<track_set_implementation> impl )
  : track_set( std::move( impl ) )
{
}

/// Constructor from a vector of tracks
object_track_set
::object_track_set( std::vector< track_sptr > const& tracks )
  : track_set( tsi_uptr( new simple_track_set_implementation( tracks ) ) )
{
}

detected_object_sptr& object_track_state::detection()
{
  return detection_;
}
const detected_object_cptr object_track_state::detection() const
{
  return std::const_pointer_cast<const detected_object>(detection_);
}

point_2d_sptr& object_track_state::image_point()
{
  return image_point_;
}
const point_2d_cptr object_track_state::image_point() const
{ 
  return image_point_;
}

point_2d_sptr& object_track_state::track2D_point()
{
  return track2D_point_;
}
const point_2d_cptr object_track_state::track2D_point() const
{
  return track2D_point_;
}

point_3d_sptr& object_track_state::track3D_point()
{
  return track3D_point_;
}
const point_3d_cptr object_track_state::track3D_point() const
{
  return track3D_point_;
}

} } // end namespace vital
