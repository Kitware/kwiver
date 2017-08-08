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
 * \brief Frame to Frame Homography definition
 */

#ifndef VITAL_HOMOGRAPHY_F2F_H
#define VITAL_HOMOGRAPHY_F2F_H

#include <vital/types/homography.h>
#include <vital/types/timestamp.h>

namespace kwiver {
namespace vital {


class VITAL_EXPORT homography_f2f
{
public:
  /// Construct an identity homography for the given frame
  /**
   * \param ts
   */
  homography_f2f( const timestamp& ts );
  explicit homography_f2f( frame_id_t frame_id );

  /// Construct a frame to frame homography using a matrix
  /**
   * \param h
   * \param from_id
   * \param to_id
   * \tparam T Data type for the underlying homography transformation
   */
  template < typename T >
  homography_f2f( Eigen::Matrix< T, 3, 3 > const& h,
                  const timestamp& from_id,
                  const timestamp& to_id )
    : h_( homography_sptr( new homography_< T > ( h ) ) ),
    from_id_( from_id ),
    to_id_( to_id )
  { }


  /// Construct a frame to frame homography given an existing transform
  /**
   * The given homography sptr is cloned into this object so we retain a unique
   * copy.
   *
   * \param h
   * \param from_id
   * \param to_id
   */
  homography_f2f( homography_sptr const& h,
                  const timestamp&       from_id,
                  const timestamp&       to_id );

  /// Copy constructor
  homography_f2f( homography_f2f const& h );

  /// Destructor
  virtual ~homography_f2f() VITAL_DEFAULT_DTOR

  /// Get the sptr of the contained homography transformation
  virtual homography_sptr homography() const;

  /// Frame identifier that the homography maps from.
  virtual const timestamp& from_id() const;

  /// Frame identifier that the homography maps to.
  virtual const timestamp& to_id() const;

  /// Return a new inverse \p homography_f2f instance
  /**
   * \return New \p homography_f2f instance whose transformation is inverted as
   *         well as has flipped from and to ID references.
   */
  virtual homography_f2f inverse() const;

  /// Custom homography_f2f multiplication operator for \p homography_f2f
  /**
   * \throws invalid_matrix_operation
   *    When \p this.from_id() != \p rhs.to_id() as transformed from and to IDs
   *    are undefined otherwise.
   *
   * \param rhs Right-hand-side operand homography.
   * \return New homography object whose transform is the result of
   *         \p this * \p rhs.
   */
  virtual homography_f2f operator*( homography_f2f const& rhs );


protected:
  /// Homography transformation sptr.
  homography_sptr h_;

  /// From frame identifier.
  timestamp from_id_;

  /// To frame identifier.
  timestamp to_id_;
};


/// Shared pointer for \p homography_f2f
typedef std::shared_ptr< homography_f2f > homography_f2f_sptr;


/// \p homography_f2f output stream operator
VITAL_EXPORT std::ostream& operator<<( std::ostream& s, homography_f2f const& h );


} } // end vital namespace

#endif // VITAL_HOMOGRAPHY_F2F_H
