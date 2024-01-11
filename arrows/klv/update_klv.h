// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of update_klv metadata filter.

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/algo/buffered_metadata_filter.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Attempts to encode vital metadata fields into KLV.
///
/// This is very much not comprehensive or lossless. Direct manipulation of KLV
/// should be preferred for precision.
///
/// \warning
///   Only feed this filter a single video, in frame order. Past metadata fed
///   to it is used in the algorithm.
class KWIVER_ALGO_KLV_EXPORT update_klv
  : public vital::algo::buffered_metadata_filter
{
public:
  virtual ~update_klv();
  
  PLUGGABLE_IMPL(
    // name 
    update_klv,
    // description
    "Edits klv packets based on vital metadata values.",
    //parameters
    PARAM_DEFAULT(st1108_frequency, size_t,
          "How often (in frames) to encode a ST1108 packet.",
          1),
    PARAM_DEFAULT(st1108_inter, std::string,
           "How to deal with a group of multiple frames when st1108_frequency > 1. "

           "'sample' will create a packet with the metric values of the first frame "
           "of the group and associate it with the first frame only, leaving the rest "
           "of the frames in the group with no associated values. "

           "'sample_smear' will create a packet with the metric values of the first "
           "frame of the group and associate it with all frames in the group. "

           "'mean' will create a packet with the averages of the group's metric "
           "values and associate it with all frames in the group.",
           // default value
            "sample")
    );

  bool check_configuration( vital::config_block_sptr config ) const override;

  size_t send(
    vital::metadata_vector const& input_metadata,
    vital::image_container_scptr const& input_image ) override;
  vital::metadata_vector receive() override;
  size_t flush() override;

  size_t available_frames() const override;
  size_t unavailable_frames() const override;

private:
  void initialize() override;
  class impl;
  KWIVER_UNIQUE_PTR(impl,d);
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
