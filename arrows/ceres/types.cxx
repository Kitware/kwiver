// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of enum to/from string conversions

#include "lens_distortion.h"

#include <arrows/ceres/types.h>

#include <ceres/loss_function.h>

using namespace kwiver::vital;

#define CERES_ENUM_HELPERS( NS, ceres_type )                              \
  namespace kwiver {                                                      \
  namespace vital {                                                       \
                                                                          \
  template <>                                                             \
  config_block_value_t                                                    \
  config_block_set_value_cast( NS::ceres_type const & value )             \
  {                                                                       \
    return NS::ceres_type##ToString( value );                             \
  }                                                                       \
                                                                          \
  template <>                                                             \
  NS::ceres_type                                                          \
  config_block_get_value_cast( config_block_value_t const & value )       \
  {                                                                       \
    NS::ceres_type cet;                                                   \
    if( !NS::StringTo##ceres_type( value, &cet ) )                        \
    {                                                                     \
      VITAL_THROW( bad_config_block_cast, value );                        \
    }                                                                     \
    return cet;                                                           \
  }                                                                       \
                                                                          \
  }                                                                       \
                                                                          \
  namespace arrows {                                                      \
  namespace ceres {                                                       \
                                                                          \
  template <>                                                             \
  std::string                                                             \
  ceres_options< NS::ceres_type >()                                       \
  {                                                                       \
    typedef NS::ceres_type T;                                             \
    std::string options_str = "\nMust be one of the following options:";  \
    std::string opt;                                                      \
    for( unsigned i = 0; i < 20; ++i )                                    \
    {                                                                     \
      opt = NS::ceres_type##ToString( static_cast< T >( i ) );            \
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

CERES_ENUM_HELPERS( ::ceres, LinearSolverType )
CERES_ENUM_HELPERS( ::ceres, PreconditionerType )
CERES_ENUM_HELPERS( ::ceres, TrustRegionStrategyType )
CERES_ENUM_HELPERS( ::ceres, DoglegType )

CERES_ENUM_HELPERS( kwiver::arrows::ceres, LossFunctionType )
CERES_ENUM_HELPERS( kwiver::arrows::ceres, CameraIntrinsicShareType )

#undef CERES_ENUM_HELPERS

namespace kwiver {

namespace arrows {

namespace ceres {

#define CASESTR( x ) case x: return #x
#define STRENUM( x ) if( value == #x ) { *type = x; return true; }

/// Convert a string to upper case
static void
UpperCase( std::string* input )
{
  std::transform( input->begin(), input->end(), input->begin(), ::toupper );
}

/// Provide a string representation for a LossFunctionType value
const char*
LossFunctionTypeToString( LossFunctionType type )
{
  switch( type )
  {
    CASESTR( TRIVIAL_LOSS );
    CASESTR( HUBER_LOSS );
    CASESTR( SOFT_L_ONE_LOSS );
    CASESTR( CAUCHY_LOSS );
    CASESTR( ARCTAN_LOSS );
    CASESTR( TUKEY_LOSS );
    default:
      return "UNKNOWN";
  }
}

/// Parse a LossFunctionType value from a string or return false
bool
StringToLossFunctionType( std::string value, LossFunctionType* type )
{
  UpperCase( &value );
  STRENUM( TRIVIAL_LOSS );
  STRENUM( HUBER_LOSS );
  STRENUM( SOFT_L_ONE_LOSS );
  STRENUM( CAUCHY_LOSS );
  STRENUM( ARCTAN_LOSS );
  STRENUM( TUKEY_LOSS );
  return false;
}

/// Provide a string representation for a CameraIntrinsicShareType value
const char*
CameraIntrinsicShareTypeToString( CameraIntrinsicShareType type )
{
  switch( type )
  {
    CASESTR( AUTO_SHARE_INTRINSICS );
    CASESTR( FORCE_COMMON_INTRINSICS );
    CASESTR( FORCE_UNIQUE_INTRINSICS );
    default:
      return "UNKNOWN";
  }
}

/// Parse a CameraIntrinsicShareType value from a string or return false
bool
StringToCameraIntrinsicShareType( std::string value,
                                  CameraIntrinsicShareType* type )
{
  UpperCase( &value );
  STRENUM( AUTO_SHARE_INTRINSICS );
  STRENUM( FORCE_COMMON_INTRINSICS );
  STRENUM( FORCE_UNIQUE_INTRINSICS );
  return false;
}

#undef CASESTR
#undef STRENUM

/// Construct a LossFunction object from the specified enum type
::ceres::LossFunction*
LossFunctionFactory( LossFunctionType type, double s )
{
  switch( type )
  {
    case TRIVIAL_LOSS:
      return NULL;
    case HUBER_LOSS:
      return new ::ceres::HuberLoss( s );
    case SOFT_L_ONE_LOSS:
      return new ::ceres::SoftLOneLoss( s );
    case CAUCHY_LOSS:
      return new ::ceres::CauchyLoss( s );
    case ARCTAN_LOSS:
      return new ::ceres::ArctanLoss( s );
    case TUKEY_LOSS:
      return new ::ceres::TukeyLoss( s );
    default:
      return NULL;
  }
}

} // namespace ceres

} // namespace arrows

} // namespace kwiver
