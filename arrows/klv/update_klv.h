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
  update_klv();
  virtual ~update_klv();

  PLUGIN_INFO( "update_klv",
               "Edits klv packets based on vital metadata values." )

  vital::config_block_sptr get_configuration() const override;
  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  size_t send(
    vital::metadata_vector const& input_metadata,
    vital::image_container_scptr const& input_image ) override;
  vital::metadata_vector receive() override;
  size_t flush() override;

  size_t available_frames() const override;
  size_t unavailable_frames() const override;

private:
  class impl;
  std::unique_ptr< impl > d;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
