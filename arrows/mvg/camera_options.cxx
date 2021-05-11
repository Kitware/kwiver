// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of MVG camera options.

#include "camera_options.h"

#define MVG_ENUM_HELPERS( NS, mvg_type )                                  \
  namespace kwiver {                                                      \
  namespace vital {                                                       \
                                                                          \
  template <>                                                             \
  config_block_value_t                                                    \
  config_block_set_value_cast( NS::mvg_type const & value )               \
  {                                                                       \
    return NS::mvg_type##ToString( value );                               \
  }                                                                       \
                                                                          \
  template <>                                                             \
  NS::mvg_type                                                            \
  config_block_get_value_cast( config_block_value_t const & value )       \
  {                                                                       \
    NS::mvg_type cet;                                                     \
    if( !NS::StringTo##mvg_type( value, cet ) )                           \
    {                                                                     \
      VITAL_THROW( bad_config_block_cast, value );                        \
    }                                                                     \
    return cet;                                                           \
  }                                                                       \
                                                                          \
  }                                                                       \
                                                                          \
  namespace arrows {                                                      \
  namespace mvg {                                                         \
                                                                          \
  template <>                                                             \
  std::string                                                             \
  mvg_options< NS::mvg_type >()                                           \
  {                                                                       \
    typedef NS::mvg_type T;                                               \
    std::string options_str = "\nMust be one of the following options:";  \
    std::string opt;                                                      \
    for( unsigned i = 0; i < 20; ++i )                                    \
    {                                                                     \
      opt = NS::mvg_type##ToString( static_cast< T >( i ) );              \
      if( opt == "UNKNOWN" )                                              \
      {                                                                   \
        break;                                                            \
      }                                                                   \
      options_str += "\n  - " + opt;                                      \
    }                                                                     \
    return options_str;                                                   \
  }                                                                       \
                                                                          \
  }                                                                       \
  }                                                                       \
  }

MVG_ENUM_HELPERS( kwiver::arrows::mvg, LensDistortionType )

#undef MVG_ENUM_HELPERS

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace mvg {

#define CASESTR( x ) case x: return #x
#define STRENUM( x ) if( value == #x ) { type = x; return true; }

// Convert a string to the upper case.
static std::string&
UpperCase( std::string& str )
{
  std::transform( str.begin(), str.end(), str.begin(), ::toupper );
  return str;
}

// ----------------------------------------------------------------------------
const char*
LensDistortionTypeToString( LensDistortionType type )
{
  switch( type )
  {
    CASESTR( NO_DISTORTION );
    CASESTR( POLYNOMIAL_RADIAL_DISTORTION );
    CASESTR( POLYNOMIAL_RADIAL_TANGENTIAL_DISTORTION );
    CASESTR( RATIONAL_RADIAL_TANGENTIAL_DISTORTION );
    default:
      return "UNKNOWN";
  }
}

bool
StringToLensDistortionType( std::string value, LensDistortionType& type )
{
  UpperCase( value );
  STRENUM( NO_DISTORTION );
  STRENUM( POLYNOMIAL_RADIAL_DISTORTION );
  STRENUM( POLYNOMIAL_RADIAL_TANGENTIAL_DISTORTION );
  STRENUM( RATIONAL_RADIAL_TANGENTIAL_DISTORTION );
  return false;
}

// ----------------------------------------------------------------------------
void
camera_options
::get_configuration( config_block_sptr config ) const
{
  config->set_value( "optimize_focal_length", this->optimize_focal_length,
                     "Include focal length parameters in bundle adjustment." );
  config->set_value( "optimize_aspect_ratio", this->optimize_aspect_ratio,
                     "Include aspect ratio parameters in bundle adjustment." );
  config->set_value( "optimize_principal_point",
                     this->optimize_principal_point,
                     "Include principal point parameters in bundle adjustment." );
  config->set_value( "optimize_skew", this->optimize_skew,
                     "Include skew parameters in bundle adjustment." );
  config->set_value( "lens_distortion_type", this->lens_distortion_type,
                     "Lens distortion model to use." +
                     mvg_options< LensDistortionType >() );
  config->set_value( "optimize_dist_k1", this->optimize_dist_k1,
                     "Include radial lens distortion parameter k1 in "
                     "bundle adjustment." );
  config->set_value( "optimize_dist_k2", this->optimize_dist_k2,
                     "Include radial lens distortion parameter k2 in "
                     "bundle adjustment." );
  config->set_value( "optimize_dist_k3", this->optimize_dist_k3,
                     "Include radial lens distortion parameter k3 in "
                     "bundle adjustment." );
  config->set_value( "optimize_dist_p1_p2", this->optimize_dist_p1_p2,
                     "Include tangential lens distortion parameters "
                     "p1 and p2 in bundle adjustment." );
  config->set_value( "optimize_dist_k4_k5_k6", this->optimize_dist_k4_k5_k6,
                     "Include radial lens distortion parameters "
                     "k4, k5, and k6 in bundle adjustment." );
  config->set_value( "minimum_hfov", this->minimum_hfov,
                     "A soft lower bound on the minimum horizontal field of "
                     "view in degrees. This generates a soft upper bound on "
                     "focal length if set greater than zero. If the focal "
                     "length exceeds this limit it will incur a quadratic "
                     "penalty." );
}

void
camera_options
::set_configuration( config_block_sptr config )
{
#define GET_VALUE( vtype, vname ) \
  this->vname = config->get_value< vtype >(#vname, this->vname );

  GET_VALUE( bool, optimize_focal_length );
  GET_VALUE( bool, optimize_aspect_ratio );
  GET_VALUE( bool, optimize_principal_point );
  GET_VALUE( bool, optimize_skew );
  GET_VALUE( bool, optimize_dist_k1 );
  GET_VALUE( bool, optimize_dist_k2 );
  GET_VALUE( bool, optimize_dist_k3 );
  GET_VALUE( bool, optimize_dist_p1_p2 );
  GET_VALUE( bool, optimize_dist_k4_k5_k6 );
  GET_VALUE( LensDistortionType, lens_distortion_type );
  GET_VALUE( double, minimum_hfov );
#undef GET_VALUE
}

} // namespace mvg

} // namespace arrows

} // namespace kwiver
