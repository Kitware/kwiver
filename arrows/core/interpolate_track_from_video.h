/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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

#ifndef KWIVER_ARROWS_INTERPOLATE_TRACK_FROM_VIDEO_H
#define KWIVER_ARROWS_INTERPOLATE_TRACK_FROM_VIDEO_H

#include <arrows/core/kwiver_algo_core_export.h>
#include <vital/algo/interpolate_track.h>

namespace kwiver {
namespace arrows {
namespace core {

///
/**
 *
 *
 */
class KWIVER_ALGO_CORE_EXPORT interpolate_track_from_video
  : public vital::algorithm_impl<interpolate_track_from_video, vital::algo::interpolate_track>
{
public:
  interpolate_track_from_video();
  virtual ~interpolate_track_from_video() = default;

  virtual vital::config_block_sptr get_configuration() const;
  virtual void set_configuration(vital::config_block_sptr config);
  virtual bool check_configuration(vital::config_block_sptr config) const;

  //+ this may have to use object_track_set's
  virtual object_track& interpolate( const object_track& init_states );

private:
  // private implementation class
  class priv;
  const std::unique_ptr<priv> d;
};

} } } // end namespace

#endif /* KWIVER_ARROWS_INTERPOLATE_TRACK_FROM_VIDEO_H */
