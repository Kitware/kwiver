// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of apply_child_klv filter.

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/algo/metadata_filter.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Applies KLV amend and segment sets.
class KWIVER_ALGO_KLV_EXPORT apply_child_klv
  : public vital::algo::metadata_filter
{
public:
  apply_child_klv();
  virtual ~apply_child_klv();

  PLUGIN_INFO( "apply_child_klv",
               "Produces resultant klv sets from source klv with ST1607 amend "
               "or segment sets." )

  vital::config_block_sptr get_configuration() const override;
  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  vital::metadata_vector filter(
    vital::metadata_vector const& input_metadata,
    vital::image_container_scptr const& input_image ) override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
