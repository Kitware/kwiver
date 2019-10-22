/*ckwg +30
 * Copyright 2019 by Kitware, Inc.
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
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/**
 * \file
 * \brief Header for \link kwiver::vital::track_interval type
 */

#ifndef VITAL_TRACK_INTERVAL_H_
#define VITAL_TRACK_INTERVAL_H_

#include <vital/types/timestamp.h>

#include <vital/vital_types.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
struct track_interval
{
  track_id_t track;
  timestamp start;
  timestamp stop;
};

// ----------------------------------------------------------------------------
/// output stream operator for a track_interval
inline
std::ostream&
operator<<( std::ostream& s, track_interval const& ti )
{
  s << ti.track << '[' << ti.start << ", " << ti.stop << ']';
  return s;
}

// ----------------------------------------------------------------------------
/// equality operator for a track_interval
inline
bool
operator==( track_interval const& lhs, track_interval const& rhs )
{
  return
    ( lhs.track == rhs.track ) &&
    ( lhs.start == rhs.start ) &&
    ( lhs.stop == rhs.stop );
}

} // namespace vital

} // namespace kwiver

#endif
