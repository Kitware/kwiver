// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of close_loops_multi_method

#include "close_loops_multi_method.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <vital/algo/algorithm.h>
#include <vital/exceptions/algorithm.h>

namespace kwiver {

namespace arrows {

namespace core {

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
static std::string
source_name( size_t n )
{
  return "method_" + std::to_string( n );
}

/// Private implementation class
class close_loops_multi_method::priv
{
public:
  priv( close_loops_multi_method& parent )
    : parent( parent )
  {}

  close_loops_multi_method& parent;

  /// The close loops methods to use.
  std::vector< vital::algo::close_loops_sptr >
  c_method()
  {
    return parent.c_method;
  }
};

// ----------------------------------------------------------------------------
void
close_loops_multi_method
::initialize()
{
  attach_logger( "arrows.core.close_loops_multi_method" );
}

/// Destructor
close_loops_multi_method
::~close_loops_multi_method() noexcept
{}

// ----------------------------------------------------------------------------
bool
close_loops_multi_method
::check_configuration( vital::config_block_sptr config ) const
{
  for( size_t i = 0; i < c_method.size(); i++ )
  {
    if( !check_nested_algo_configuration< vital::algo::close_loops >(
      source_name( i ),
      config ) )
    {
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
feature_track_set_sptr
close_loops_multi_method
::stitch(
  frame_id_t frame_number, feature_track_set_sptr input,
  image_container_sptr image, image_container_sptr mask ) const
{
  feature_track_set_sptr updated_set = input;

  for( size_t i = 0; i < c_method.size(); i++ )
  {
    updated_set = c_method[ i ]->stitch(
      frame_number, updated_set, image,
      mask );
  }

  return updated_set;
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
