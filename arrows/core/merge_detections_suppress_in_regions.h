// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_MERGE_DETECTIONS_SUPPRESS_IN_REGIONS_H_
#define KWIVER_ARROWS_MERGE_DETECTIONS_SUPPRESS_IN_REGIONS_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/merge_detections.h>

namespace kwiver {
namespace arrows {
namespace core {

// -----------------------------------------------------------------------------
/**
 * \class merge_detections_suppress_in_regions
 *
 * \brief Prunes detections overlapping with regions identified by class string
 */
class KWIVER_ALGO_CORE_EXPORT merge_detections_suppress_in_regions
  : public vital::algorithm_impl< merge_detections_suppress_in_regions,
    vital::algo::merge_detections >
{

public:
  PLUGIN_INFO( "suppress_in_regions",
    "Suppresses detections within regions indicated by a certain fixed category "
    "of detections. Can either remove the detections or reduce their probability." )

  merge_detections_suppress_in_regions();
  virtual ~merge_detections_suppress_in_regions();

  /// Get this algorithm's \link kwiver::vital::config_block configuration block \endlink
  virtual vital::config_block_sptr get_configuration() const;
  /// Set this algorithm's properties via a config block
  virtual void set_configuration( vital::config_block_sptr config );
  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Refine all input object detections
  /**
   * This method suppresses detections given by ID
   *
   * \param sets Input detection sets
   * \returns vector of refined detections
   */
  virtual kwiver::vital::detected_object_set_sptr
  merge( std::vector< kwiver::vital::detected_object_set_sptr > const& sets ) const;

private:

  /// private implementation class
  class priv;
  const std::unique_ptr< priv > d;

}; // end class merge_detections_suppress_in_regions

}}} // end namespace

#endif /* KWIVER_ARROWS_MERGE_DETECTIONS_SUPPRESS_IN_REGIONS_H_ */
