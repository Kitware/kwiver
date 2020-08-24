/*ckwg +29
 * Copyright 2015-2020 by Kitware, Inc.
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

#ifndef KWIVER_ARROWS_CORE_FILTER_FEATURES_MAGNITUDE_H_
#define KWIVER_ARROWS_CORE_FILTER_FEATURES_MAGNITUDE_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/filter_features.h>

/**
 * \file
 * \brief Header defining the filter_features_magnitude algorithm
 */

namespace kwiver {
namespace arrows {
namespace core {

/// \brief Algorithm that filters features based on feature magnitude
class KWIVER_ALGO_CORE_EXPORT filter_features_magnitude
  : public vital::algo::filter_features
{
public:
  PLUGIN_INFO( "magnitude",
               "Filter features using a threshold"
               " on the magnitude of the detector response function." )

  /// Constructor
  filter_features_magnitude();

  /// Destructor
  virtual ~filter_features_magnitude();

  /// Get this algorithm's \link vital::config_block configuration block \endlink
  virtual vital::config_block_sptr get_configuration() const;
  /// Set this algorithm's properties via a config block
  virtual void set_configuration(vital::config_block_sptr config);
  /// Check that the algorithm's configuration config_block is valid
  virtual bool check_configuration(vital::config_block_sptr config) const;

protected:

  /// filter a feature set
  /**
   * \param [in] feature set to filter
   * \param [out] indices of the kept features to the original feature set
   * \returns a filtered version of the feature set
   */
  virtual vital::feature_set_sptr
  filter(vital::feature_set_sptr input, std::vector<unsigned int> &indices) const;
  using filter_features::filter;

private:
  /// private implementation class
  class priv;
  const std::unique_ptr<priv> d_;
};

} // end namespace core
} // end namespace arrows
} // end namespace kwiver

#endif
