/*ckwg +29
 * Copyright 2017-2020 by Kitware, Inc.
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

#ifndef KWIVER_ARROWS_COMPUTE_ASSOCIATION_MATRIX_FROM_FEATURES_H_
#define KWIVER_ARROWS_COMPUTE_ASSOCIATION_MATRIX_FROM_FEATURES_H_

#include <vital/vital_config.h>
#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/compute_association_matrix.h>

namespace kwiver {
namespace arrows {
namespace core {

/// Compute an association matrix given detections and tracks
class KWIVER_ALGO_CORE_EXPORT compute_association_matrix_from_features
  : public vital::algo::compute_association_matrix
{
public:
  PLUGIN_INFO( "from_features",
               "Populate association matrix in tracking from detector features." )

  /// Default Constructor
  compute_association_matrix_from_features();

  /// Destructor
  virtual ~compute_association_matrix_from_features() noexcept;

  /// Get this algorithm's \link vital::config_block configuration block \endlink
  /**
   * \returns \c config_block containing the configuration for this algorithm
   *          and any nested components.
   */
  virtual vital::config_block_sptr get_configuration() const;

  /// Set this algorithm's properties via a config block
  /**
   * \throws no_such_configuration_value_exception
   *    Thrown if an expected configuration value is not present.
   * \throws algorithm_configuration_exception
   *    Thrown when the algorithm is given an invalid \c config_block or is'
   *    otherwise unable to configure itself.
   *
   * \param config  The \c config_block instance containing the configuration
   *                parameters for this algorithm
   */
  virtual void set_configuration(vital::config_block_sptr config);

  /// Check that the algorithm's currently configuration is valid
  /**
   * This checks solely within the provided \c config_block and not against
   * the current state of the instance. This isn't static for inheritence
   * reasons.
   *
   * \param config  The config block to check configuration of.
   *
   * \returns true if the configuration check passed and false if it didn't.
   */
  virtual bool check_configuration(vital::config_block_sptr config) const;

  /// Compute an association matrix given detections and tracks
  /**
   * \param ts frame ID
   * \param image contains the input image for the current frame
   * \param tracks active track set from the last frame
   * \param detections input detected object sets from the current frame
   * \param matrix output matrix
   * \param considered output detections used in matrix
   * \return returns whether a matrix was successfully computed
   */
  virtual bool
  compute(kwiver::vital::timestamp ts,
          kwiver::vital::image_container_sptr image,
          kwiver::vital::object_track_set_sptr tracks,
          kwiver::vital::detected_object_set_sptr detections,
          kwiver::vital::matrix_d& matrix,
          kwiver::vital::detected_object_set_sptr& considered) const;

private:
  /// private implementation class
  class priv;
  const std::unique_ptr<priv> d_;
};


} // end namespace core
} // end namespace arrows
} // end namespace kwiver


#endif
