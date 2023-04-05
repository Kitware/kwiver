// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file This header defines the interface for a plugin that formats a
 * config block.
 */

#ifndef VITAL_CONFIG_FORMAT_CONFIG_BLOCK_H
#define VITAL_CONFIG_FORMAT_CONFIG_BLOCK_H

#include <vital/config/config_block.h>
#include <vital/plugin_management/pluggable.h>
#include <vital/plugin_management/pluggable_macro_magic.h>

#include <ostream>
#include <string>

namespace kwiver::vital {

// ----------------------------------------------------------------

/**
 * \brief Config block printer plugin interface.
 *
 * This class defines the abstract interface for all implementations
 * of the config block formatting plugin.
 *
 * TODO: This likely should be an "algorithm" and not be located in the config
 *       module since there is no inbuilt use of this -- It seems to only be
 *       used in down-stream libraries/tools.
 */
class format_config_block : public pluggable
{
public:
  PLUGGABLE_INTERFACE( format_config_block )

  virtual void print( std::ostream& str ) = 0;

  #define PARAM_SET() \
    PARAM( opt_prefix, std::string, "Optional prefix for output" ), \
    PARAM( config, config_block_sptr, "Reference to a config block"), \
    PARAM_DEFAULT( opt_gen_source_loc, bool, "Provide information about config file.", false)

  PLUGGABLE_VARIABLES( PARAM_SET() )
  // PLUGGABLE_CONSTRUCTOR( format_config_block, PARAM_SET() )
}; // end class format_config_block

using format_config_block_sptr = std::shared_ptr< format_config_block >;

} // namespace kwiver::vital

#endif /* VITAL_CONFIG_FORMAT_CONFIG_BLOCK_H */
