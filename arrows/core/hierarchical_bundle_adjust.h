/*ckwg +29
 * Copyright 2014-2017 by Kitware, Inc.
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
 * \brief Header defining the hierarchical_bundle_adjust algorithm
 */

#ifndef KWIVER_ARROWS_CORE_HIERARCHICAL_BUNDLE_ADJUST_H_
#define KWIVER_ARROWS_CORE_HIERARCHICAL_BUNDLE_ADJUST_H_

#include <vital/vital_config.h>
#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/bundle_adjust.h>
#include <vital/config/config_block.h>

#include <memory>


namespace kwiver {
namespace arrows {
namespace core {


class KWIVER_ALGO_CORE_EXPORT hierarchical_bundle_adjust
  : public vital::algorithm_impl<hierarchical_bundle_adjust, vital::algo::bundle_adjust>
{
public:

  /// Constructor
  hierarchical_bundle_adjust();
  /// Destructor
  virtual ~hierarchical_bundle_adjust() VITAL_NOTHROW;

  /// Get this algorithm's \link kwiver::vital::config_block configuration block \endlink
  virtual vital::config_block_sptr get_configuration() const;
  /// Set this algorithm's properties via a config block
  virtual void set_configuration(vital::config_block_sptr config);
  /// Check that the algorithm's configuration vital::config_block is valid
  virtual bool check_configuration(vital::config_block_sptr config) const;

  /// Optimize the camera and landmark parameters given a set of tracks
  virtual void optimize(vital::camera_map_sptr & cameras,
                        vital::landmark_map_sptr & landmarks,
                        vital::feature_track_set_sptr tracks,
                        vital::video_metadata_map_sptr metadata = nullptr) const;

private:
  // private implementation class
  class priv;
  std::unique_ptr<priv> d_;
};


/// Type definition for shared pointer for hierarchical_bundle_adjust algorithm
typedef std::shared_ptr<hierarchical_bundle_adjust> hierarchical_bundle_adjust_sptr;

} // end namespace core
} // end namespace arrows
} // end namespace kwiver

#endif // KWIVER_ARROWS_CORE_HIERARCHICAL_BUNDLE_ADJUST_H_
