// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_VITAL_UNIQUE_MAP_PTR_H
#define KWIVER_VITAL_UNIQUE_MAP_PTR_H

#include <memory>

namespace kwiver {

namespace vital {

// The design for various collections requires that the elements in the
// collection are owned by the collection and are only returned by value.
// Typically, one uses std::unique_ptr to accomplish this. Unfortunately, not
// all STL implementations support maps with unique_ptr elements. This type
// alias exists to help work around this limitation.
#ifdef VITAL_STD_MAP_NO_UNIQUE_PTR
template< typename T >
using unique_map_ptr = std::shared_ptr< T >;
#else
template< typename T >
using unique_map_ptr = std::unique_ptr< T >;
#endif

// ----------------------------------------------------------------------------
template< typename T, typename... Args >
inline unique_map_ptr< T > make_map_unique( Args&&... args )
{
#ifdef VITAL_STD_MAP_NO_UNIQUE_PTR
  return std::make_shared< T >( std::forward< Args >( args )... );
#else
  return unique_map_ptr< T >{ new T( std::forward< Args >( args )... ) };
#endif
}

} // namespace vital

} // namespace kwiver

#endif
