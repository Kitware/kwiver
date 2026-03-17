// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "resection_camera_options.h"

namespace kwiver {

namespace arrows {

namespace ocv {

std::ostream&
operator<<( std::ostream& s, resection_camera_options::vectorf const& v )
{
  for( size_t i = 0, n = v.size(); i < n; ++i )
  {
    if( i > 0 ) { s << ' '; }
    s << v[ i ];
  }
  return s;
}

std::istream&
operator>>( std::istream& s, resection_camera_options::vectorf& v )
{
  while( !s.eof() )
  {
    float a = 0;
    s >> a;
    v.push_back( a );
  }
  return s;
}

// ----------------------------------------------------------------------------
void
resection_camera_options
::get_configuration( vital::config_block_sptr config ) const
{
  arrows::mvg::camera_options::get_configuration( config );
  config->set_value(
    "reproj_accuracy", this->reproj_accuracy,
    "desired re-projection positive accuracy for inlier points" );
  config->set_value(
    "max_iterations", this->max_iterations,
    "maximum number of iterations to run optimization [1, INT_MAX]" );

  std::stringstream ss;
  ss << this->focal_scales;
  config->set_value(
    "focal_scales", ss.str(),
    "focal length scales to optimize f*scale over" );
}

// ----------------------------------------------------------------------------
void
resection_camera_options
::set_configuration( vital::config_block_sptr config )
{
  arrows::mvg::camera_options::set_configuration( config );

  this->reproj_accuracy = config->get_value< double >(
    "reproj_accuracy",
    this->reproj_accuracy );
  this->max_iterations = config->get_value< int >(
    "max_iterations",
    this->max_iterations );

  std::stringstream ss( config->get_value< std::string >(
    "focal_scales",
    "1" ) );
  this->focal_scales.clear();
  ss >> this->focal_scales;
}

} // namespace ocv

} // namespace arrows

} // namespace kwiver
