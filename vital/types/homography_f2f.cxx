/*ckwg +29
 * Copyright 2015-2017 by Kitware, Inc.
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
 * \brief Frame to Frame Homography implementation
 */

#include "homography_f2f.h"

#include <vital/exceptions/math.h>
#include <vital/vital_types.h>

namespace kwiver {
namespace vital {


// ------------------------------------------------------------------
// Construct an identity homography for the given frame
homography_f2f
::homography_f2f( const timestamp& frame_id )
  : h_( homography_sptr( new homography_<double>() ) ),
    from_id_( frame_id ),
    to_id_( frame_id )
{
}


homography_f2f
::homography_f2f( frame_id_t frame_id )
  : h_( homography_sptr( new homography_<double>() ) ),
    from_id_( kwiver::vital::timestamp( 0, frame_id) ),
    to_id_( kwiver::vital::timestamp( 0, frame_id) )
{
}


// ------------------------------------------------------------------
// Construct a frame to frame homography given an existing transform
homography_f2f
::homography_f2f( homography_sptr const &h,
                  const timestamp& from_id,
                  const timestamp& to_id )
  : h_( h->clone() ),
    from_id_( from_id ),
    to_id_( to_id )
{
}


// ------------------------------------------------------------------
// Copy constructor
homography_f2f
::homography_f2f( homography_f2f const &h )
  : h_( h.h_->clone() ),
    from_id_( h.from_id_ ),
    to_id_( h.to_id_ )
{
}


// ------------------------------------------------------------------
// Get the homography transformation
homography_sptr
homography_f2f
::homography() const
{
  return this->h_;
}


// ------------------------------------------------------------------
// Frame identifier that the homography maps from.
const timestamp&
homography_f2f
::from_id() const
{
  return this->from_id_;
}


// ------------------------------------------------------------------
// Frame identifier that the homography maps to.
const timestamp&
homography_f2f
::to_id() const
{
  return this->to_id_;
}


// ------------------------------------------------------------------
// Return a new inverse \p homography_f2f instance
homography_f2f
homography_f2f
::inverse() const
{
  return homography_f2f( this->h_->inverse(), this->to_id_, this->from_id_ );
}


// ------------------------------------------------------------------
// Custom homography_f2f multiplication operator for \p homography_f2f
homography_f2f
homography_f2f
::operator*( homography_f2f const &rhs )
{
  if( this->from_id() != rhs.to_id() )
  {
    throw invalid_matrix_operation( "Homography frame identifiers do not match up" );
  }

  Eigen::Matrix<double,3,3> new_h = this->h_->matrix() * rhs.h_->matrix();
  return homography_f2f( new_h, rhs.from_id(), this->to_id() );
}


// ------------------------------------------------------------------
// \p homography_f2f output stream operator
std::ostream&
operator<<( std::ostream &s, homography_f2f const &h )
{
  s << h.from_id() << " -> " << h.to_id() << "\n"
    << *h.homography();
  return s;
}


} } // end vital namespace
