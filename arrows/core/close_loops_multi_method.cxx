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

// Return IDs of all methods labels.
std::vector< std::string >
method_names( unsigned count )
{
  std::vector< std::string > output;

  for( unsigned i = 0; i < count; i++ )
  {
    std::stringstream str;
    str << "method" << (i+1);
    output.push_back( str.str() );
  }

  return output;
}

close_loops_multi_method
::close_loops_multi_method()
: count_( 1 ),
  methods_( 1 )
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
close_loops_multi_method
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  // Internal parameters
  config->set_value( "count", count_, "Number of close loops methods we want to use." );

  // Sub-algorithm implementation name + sub_config block
  std::vector< std::string > method_ids = method_names( count_ );

  for( unsigned i = 0; i < method_ids.size(); i++ )
  {
    close_loops::get_nested_algo_configuration( method_ids[i], config, methods_[i] );
  }

  return config;
}

// ----------------------------------------------------------------------------
void
close_loops_multi_method
::set_configuration( vital::config_block_sptr in_config )
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  // Parse count parameter
  count_ = config->get_value<unsigned>( "count" );
  methods_.resize( count_ );

  // Parse methods
  std::vector<std::string> method_ids = method_names( count_ );

  for( unsigned i = 0; i < method_ids.size(); i++ )
  {
    close_loops::set_nested_algo_configuration( method_ids[i], config, methods_[i] );
  }
}

// ----------------------------------------------------------------------------
bool
close_loops_multi_method
::check_configuration( vital::config_block_sptr config ) const
{
  std::vector<std::string> method_ids = method_names( config->get_value<unsigned>( "count" ) );

  for( unsigned i = 0; i < method_ids.size(); i++ )
  {
    if( !close_loops::check_nested_algo_configuration( method_ids[i], config ) )
    {
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
feature_track_set_sptr
close_loops_multi_method
::stitch( frame_id_t frame_number, feature_track_set_sptr input,
          image_container_sptr image, image_container_sptr mask ) const
{
  feature_track_set_sptr updated_set = input;

  for( unsigned i = 0; i < methods_.size(); i++ )
  {
    updated_set = methods_[i]->stitch( frame_number, updated_set, image, mask );
  }

  return updated_set;
}

} // end namespace core
} // end namespace arrows
} // end namespace kwiver
