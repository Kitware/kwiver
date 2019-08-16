/*ckwg +29
 * Copyright 2013-2019 by Kitware, Inc.
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
 * \brief Header file for \link kwiver::vital::object_track_set object_track_set
 *        \endlink and a concrete \link kwiver::vital::simple_object_track_set
 *        simple_object_track_set \endlink
 */

#ifndef VITAL_OBJECT_TRACK_SET_H_
#define VITAL_OBJECT_TRACK_SET_H_

#include <vital/types/timestamp.h>
#include <vital/types/track_set.h>
#include <vital/types/detected_object.h>
#include <vital/types/point.h>

#include <vital/vital_export.h>
#include <vital/vital_config.h>
#include <vital/vital_types.h>

#include <vital/range/transform.h>

#include <vector>
#include <memory>

namespace kwiver {
namespace vital {

// ============================================================================
/// A derived track_state for object tracks
class VITAL_EXPORT object_track_state : public track_state
{
public:
  object_track_state() = default;

  //@{
  /// Default constructor
  object_track_state( frame_id_t frame,
                      time_usec_t time,
                      detected_object_sptr const& d = nullptr )
    : track_state( frame )
    , time_(time)
    , detection_( d )
  {}

  object_track_state( frame_id_t frame,
                      time_usec_t time,
                      detected_object_sptr&& d )
    : track_state( frame )
    , time_(time)
    , detection_( std::move( d ) )
  {}
  //@}

  //@{
  /// Alternative constructor
  object_track_state( timestamp const& ts,
                      detected_object_sptr const& d = nullptr )
    : track_state( ts.get_frame() )
    , time_(ts.get_time_usec())
    , detection_( d )
  {}

  object_track_state( timestamp const& ts,
                      detected_object_sptr&& d )
    : track_state( ts.get_frame() )
    , time_(ts.get_time_usec())
    , detection_( std::move( d ) )
  {}
  //@}

  /// Copy constructor
  object_track_state( object_track_state const& other ) = default;

  /// Move constructor
  object_track_state( object_track_state&& other ) = default;

  /// Clone the track state (polymorphic copy constructor)
  track_state_sptr clone( clone_type ct = clone_type::DEEP ) const override
  {
    if ( ct == clone_type::DEEP )
    {
      auto new_detection =
        ( this->detection_ ? this->detection_->clone() : nullptr );
      return std::make_shared< object_track_state >(
        this->frame(), this->time(), std::move( new_detection ) );
    }
    else
    {
      return std::make_shared< object_track_state >( *this );
    }
  }

  void set_time( time_usec_t time )
  {
    time_ = time;
  }

  time_usec_t time() const
  {
    return time_;
  }

  detected_object_sptr& detection();
  const detected_object_cptr detection() const;

  point_2d_sptr& image_point();
  const point_2d_cptr image_point() const;

  point_3d_sptr& track_point();
  const point_3d_cptr track_point() const;

  static std::shared_ptr< object_track_state > downcast(
    track_state_sptr const& sp )
  {
    return std::dynamic_pointer_cast< object_track_state >( sp );
  }

  static constexpr auto downcast_transform = range::transform( downcast );

private:
  time_usec_t time_ = 0;
  detected_object_sptr detection_;
  point_2d_sptr  image_point_;
  point_3d_sptr  track_point_;
};


// ============================================================================
/// A collection of object tracks
class VITAL_EXPORT object_track_set : public track_set
{
public:
  /// Default Constructor
  /**
   * \note implementation defaults to simple_track_set_implementation
   */
  object_track_set();

  /// Constructor specifying the implementation
  object_track_set( std::unique_ptr< track_set_implementation > impl );

  /// Constructor from a vector of tracks
  /**
   * \note implementation defaults to simple_track_set_implementation
   */
  object_track_set( std::vector< track_sptr > const& tracks );

  /// Destructor
  virtual ~object_track_set() = default;
};

/// Shared pointer for object_track_set type
typedef std::shared_ptr< object_track_set > object_track_set_sptr;

/// Helper to iterate over the states of a track as object track states
/**
 * This object is an instance of a range transform adapter that can be applied
 * to a track_sptr in order to directly iterate over the underlying
 * object_track_state instances.
 *
 * \par Example:
 * \code
 * namespace kv = kwiver::vital;
 * namespace r = kwiver::vital::range;
 *
 * kv::track_sptr ot = get_the_object_track();
 * for ( auto s : ot | kv::as_object_track )
 *   std::cout << s->time() << std::endl;
 * \endcode
 *
 * \sa kwiver::vital::range::transform_view
 */
static constexpr auto as_object_track = object_track_state::downcast_transform;

} } // end namespace vital

#endif // VITAL_OBJECT_TRACK_SET_H_
