// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * @file
 * @brief Shared type declarations for the VITAL module.
 *
 * This file contains the canonical type names for KWIVER-VITAL types.
 */

#ifndef KWIVER_CORE_TYPES_H
#define KWIVER_CORE_TYPES_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace kwiver {
namespace vital {

/// The type to be used for general strings
typedef std::string string_t;

/// The type to be used for file and directory paths
typedef std::string path_t;
typedef std::vector< path_t > path_list_t;

// a short name for unsigned char
typedef unsigned char byte;

enum class clone_type
{
  SHALLOW,
  DEEP,
};

} } // end namespace

#endif // KWIVER_CORE_TYPES_H
