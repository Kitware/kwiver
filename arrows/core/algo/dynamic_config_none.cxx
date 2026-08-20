// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining the implementation to dynamic_config_none

#include "dynamic_config_none.h"

#include <vital/vital_config.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
bool
dynamic_config_none
::check_configuration(
  [[maybe_unused]] kwiver::vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
kwiver::vital::config_block_sptr
dynamic_config_none
::get_dynamic_configuration()
{
  return kwiver::vital::config_block::empty_config();
}

} // namespace core

} // namespace arrows

}     // end namespace
