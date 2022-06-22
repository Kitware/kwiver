// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_CONFIG_FORMATTER_H
#define KWIVER_CONFIG_FORMATTER_H

#include <vital/config/config_block.h>
#include <vital/config/vital_config_export.h>

#include <ostream>
#include <string>

namespace kwiver::vital {

/**
 * @brief Generates formatted versions of a config block.
 *
 * This class encapsulates several different formatting options for
 * a config block.
 *
 * TODO: This likely should be an "algorithm" and not be located in the config
 *       module since there is no inbuilt use of this -- It seems to only be
 *       used in down-stream libraries/tools.
 */
class VITAL_CONFIG_EXPORT config_block_formatter
{
public:
  config_block_formatter( const config_block_sptr config );
  ~config_block_formatter() = default;

  /**
   * @brief Format config block in simple text format.
   *
   * @param str Stream to format on.
   */
  void print( std::ostream& str );

  /**
   * @brief Set line prefix for printing.
   *
   * @param pfx The prefix string.
   */
  void set_prefix( const std::string& pfx );

  /**
   * @brief Set option to generate source location.
   *
   * @param opt TRUE will generate the source location, FALSE will not.
   */
  void generate_source_loc( bool opt );

private:
  void format_block( std::ostream& str,
                     const config_block_sptr config,
                     const std::string& prefix );

  config_block_sptr m_config;
  std::string m_prefix;
  bool m_gen_source_loc;
};

} // namespace kwiver::vital

#endif /* KWIVER_CONFIG_FORMATTER_H */
