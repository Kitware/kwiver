/*ckwg +29
 * Copyright 2013-2014 by Kitware, Inc.
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
 * \brief Header for \link kwiver::vital::track track \endlink objects
 */

#ifndef VITAL_TRACK_H_
#define VITAL_TRACK_H_


#include <vital/vital_export.h>
#include <vital/vital_config.h>
#include <vital/vital_types.h>

#include <vector>
#include <set>
#include <memory>

namespace kwiver {
namespace vital {


/// Empty base class for data associated with a track state
class VITAL_EXPORT track_state_data
{
protected:
  virtual ~track_state_data() VITAL_DEFAULT_DTOR
};
typedef std::shared_ptr<track_state_data> track_state_data_sptr;


/// Empty base class for data associated with a whole track
class VITAL_EXPORT track_data
{
protected:
  virtual ~track_data() VITAL_DEFAULT_DTOR
};
typedef std::shared_ptr<track_data> track_data_sptr;


/// A representation of a track.
/**
 * A track is a sequence of corresponding identifiers associated with each
 * other across time (i.e. frames indicies).  Each track consists of a
 * sequence of track states each with a frame id and optional data field.
 * Frame ids are in monotonically increasing order but need not be sequential.
 * The same track structure can be used to represent feature tracks for
 * image registration or moving object tracks.
 */
class VITAL_EXPORT track
{
public:
  /// A structure to hold the state of a track on a given frame
  struct track_state
  {
    /// Constructor
    track_state( frame_id_t            frame,
                 track_state_data_sptr d=nullptr )
      : frame_id( frame ),
        data( d )
    { }

    /// The frame identifier (i.e. frame number)
    frame_id_t frame_id;
    /// The optional data structure associated with this state
    track_state_data_sptr data;
  };

  /// convenience type for the const iterator of the track state vector
  typedef std::vector< track_state >::const_iterator history_const_itr;

  /// Default Constructor
  track(track_data_sptr d=nullptr);

  /// Copy Constructor
  track( const track& other );

  ~track() VITAL_DEFAULT_DTOR

  /// Construct a track from a single track state
  explicit track( const track_state& ts, track_data_sptr d=nullptr );

  /// Access the track identification number
  track_id_t id() const { return id_; }

  /// Access the track data
  track_data_sptr data() const { return data_; }

  /// Set the track identification number
  void set_id( track_id_t id ) { id_ = id; }

  /// Set the track data
  void set_data( track_data_sptr d ) { data_ = d; }

  /// Access the first frame number covered by this track
  frame_id_t first_frame() const;

  /// Access the last frame number covered by this track
  frame_id_t last_frame() const;

  /// Append a track state.
  /**
   * The added track state must have a frame_id greater than
   * the last frame in the history.
   *
   * \returns true if successful, false not correctly ordered
   * \param state track state to add to this track.
   */
  bool append( const track_state& state );

  /// Append the history contents of another track.
  /**
   * The first state of the input track must contain a frame number
   * greater than the last state of this track.
   *
   * \returns true if successful, false not correctly ordered
   */
  bool append( const track& to_append );

  /// Insert a track state.
  /**
   * The added track state must have a frame_id that is not already
   * present in the track history.
   *
   * \returns true if successful, false if already a state on this frame
   * \param state track state to add to this track.
   */
  bool insert( const track_state& state );

  /// Access a const iterator to the start of the history
  history_const_itr begin() const { return history_.begin(); }

  /// Access a const iterator to the end of the history
  history_const_itr end() const { return history_.end(); }

  /// Find the track state iterator matching \a frame
  /**
   *  \param [in] frame the frame number to access
   *  \return an iterator at the frame if found, or end() if not
   */
  history_const_itr find( frame_id_t frame ) const;

  /// Return the set of all frame IDs covered by this track
  std::set< frame_id_t > all_frame_ids() const;

  /// Return the number of states in the track.
  size_t size() const { return history_.size(); }

  /// Return whether or not this track has any states.
  bool empty() const { return history_.empty(); }


protected:
  /// The ordered array of track states
  std::vector< track_state > history_;
  /// The unique track identification number
  track_id_t id_;
  /// The optional data structure associated with this track
  track_data_sptr data_;
};


/// Shared pointer for general track type
typedef std::shared_ptr< track > track_sptr;

} } // end namespace vital

#endif // VITAL_TRACK_H_
