// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef _KWIVER_ARROWS_CREATE_DETECTION_GRID_H_
#define _KWIVER_ARROWS_CREATE_DETECTION_GRID_H_

#include <vital/vital_config.h>
#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/image_object_detector.h>

namespace kwiver {
namespace arrows {
namespace core {

/// Initialize object tracks via simple single frame thresholding
class KWIVER_ALGO_CORE_EXPORT create_detection_grid
  : public vital::algo::image_object_detector
{
public:
  PLUGIN_INFO( "create_detection_grid",
               "Create a grid of detections across the input image." )

  /// Default Constructor
  create_detection_grid();

  /// Destructor
  virtual ~create_detection_grid() noexcept;

  /// Get this algorithm's \link vital::config_block configuration block \endlink
  ///
  /// \returns \c config_block containing the configuration for this algorithm
  ///          and any nested components.
  virtual vital::config_block_sptr get_configuration() const;

  /// Set this algorithm's properties via a config block
  ///
  /// \throws no_such_configuration_value_exception
  ///    Thrown if an expected configuration value is not present.
  /// \throws algorithm_configuration_exception
  ///    Thrown when the algorithm is given an invalid \c config_block or is'
  ///    otherwise unable to configure itself.
  ///
  /// \param config  The \c config_block instance containing the configuration
  ///                parameters for this algorithm
  virtual void set_configuration(vital::config_block_sptr config);

  /// Check that the algorithm's currently configuration is valid
  ///
  /// This checks solely within the provided \c config_block and not against
  /// the current state of the instance. This isn't static for inheritence
  /// reasons.
  ///
  /// \param config  The config block to check configuration of.
  ///
  /// \returns true if the configuration check passed and false if it didn't.
  virtual bool check_configuration(vital::config_block_sptr config) const;

  /// Find all objects on the provided image
  ///
  /// This method analyzes the supplied image and along with any saved
  /// context, returns a vector of detected image objects.
  ///
  /// \param image_data the image pixels
  /// \returns vector of image objects found
  virtual vital::detected_object_set_sptr
      detect( vital::image_container_sptr image_data) const;

private:
  /// private implementation class
  class priv;
  const std::unique_ptr<priv> d_;
};

} // end namespace core
} // end namespace arrows
} // end namespace kwiver

#endif
