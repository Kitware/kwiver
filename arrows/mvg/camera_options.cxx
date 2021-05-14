// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation of MVG camera options.
 */

#include "camera_options.h"
#include "lens_distortion.h"

using namespace kwiver::vital;

#define MVG_ENUM_HELPERS( NS, mvg_type ) \
  namespace kwiver { \
  namespace vital { \
\
  template <> \
  config_block_value_t \
  config_block_set_value_cast( NS::mvg_type const & value ) \
  { \
    return NS::mvg_type##ToString( value ); \
  } \
\
  template <> \
  NS::mvg_type \
  config_block_get_value_cast( config_block_value_t const & value ) \
  { \
    NS::mvg_type cet; \
    if( !NS::StringTo##mvg_type( value, &cet ) ) \
    VITAL_THROW( bad_config_block_cast, value ); \
    return cet; \
  } \
\
  } \
\
  namespace arrows { \
  namespace mvg { \
\
  template <> \
  std::string \
  mvg_options< NS::mvg_type >() \
  { \
    typedef NS::mvg_type T; \
    std::string options_str = "\nMust be one of the following options:"; \
    std::string opt; \
    for( unsigned i = 0; i < 20; ++i ) \
    { \
      opt = NS::mvg_type##ToString( static_cast< T >( i ) ); \
      if( opt == "UNKNOWN" ) break; \
      options_str += "\n  - " + opt; \
    } \
    return options_str; \
  } \
\
  } \
  } \
  }

MVG_ENUM_HELPERS( kwiver::arrows::mvg, LensDistortionType )

#undef CERES_ENUM_HELPERS

namespace kwiver {

namespace arrows {

namespace mvg {

#define CASESTR( x ) case x: return #x
#define STRENUM( x ) if( value == #x ) { *type = x; return true; }

/// Convert a string to upper case
static void
UpperCase( std::string* input )
{
  std::transform( input->begin(), input->end(), input->begin(), ::toupper );
}

/// Provide a string representation for a LensDisortionType value
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

/// Parse a LensDistortionType value from a string or return false
bool
StringToLensDistortionType( std::string value, LensDistortionType* type )
{
  UpperCase( &value );
  STRENUM( NO_DISTORTION );
  STRENUM( POLYNOMIAL_RADIAL_DISTORTION );
  STRENUM( POLYNOMIAL_RADIAL_TANGENTIAL_DISTORTION );
  STRENUM( RATIONAL_RADIAL_TANGENTIAL_DISTORTION );
  return false;
}

/// Return the number of distortion parameters required for each type
unsigned int
num_distortion_params( LensDistortionType type )
{
  switch( type )
  {
    case POLYNOMIAL_RADIAL_DISTORTION:
      return distortion_poly_radial::num_coeffs;
    case POLYNOMIAL_RADIAL_TANGENTIAL_DISTORTION:
      return distortion_poly_radial_tangential::num_coeffs;
    case RATIONAL_RADIAL_TANGENTIAL_DISTORTION:
      return distortion_ratpoly_radial_tangential::num_coeffs;
    default:
      return 0;
  }
}

/// Constructor
camera_options
::camera_options()
  : optimize_focal_length( true ),
    optimize_aspect_ratio( false ),
    optimize_principal_point( false ),
    optimize_skew( false ),
    lens_distortion_type( NO_DISTORTION ),
    optimize_dist_k1( true ),
    optimize_dist_k2( false ),
    optimize_dist_k3( false ),
    optimize_dist_p1_p2( false ),
    optimize_dist_k4_k5_k6( false ),
    minimum_hfov( 0.0 )
{
}

/// Copy Constructor
camera_options
::camera_options( const camera_options& other )
  : optimize_focal_length( other.optimize_focal_length ),
    optimize_aspect_ratio( other.optimize_aspect_ratio ),
    optimize_principal_point( other.optimize_principal_point ),
    optimize_skew( other.optimize_skew ),
    lens_distortion_type( other.lens_distortion_type ),
    optimize_dist_k1( other.optimize_dist_k1 ),
    optimize_dist_k2( other.optimize_dist_k2 ),
    optimize_dist_k3( other.optimize_dist_k3 ),
    optimize_dist_p1_p2( other.optimize_dist_p1_p2 ),
    optimize_dist_k4_k5_k6( other.optimize_dist_k4_k5_k6 ),
    minimum_hfov( other.minimum_hfov )
{
}

/// populate the config block with options
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

/// set the member variables from the config block
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

/// Return true if any options to optimize intrinsic parameters are set
bool
camera_options
::optimize_intrinsics() const
{
  if( optimize_focal_length ||
      optimize_aspect_ratio ||
      optimize_principal_point ||
      optimize_skew )
  {
    return true;
  }
  switch( lens_distortion_type )
  {
    case POLYNOMIAL_RADIAL_DISTORTION:
      return optimize_dist_k1 ||
             optimize_dist_k2 ||
             optimize_dist_k3;
    case POLYNOMIAL_RADIAL_TANGENTIAL_DISTORTION:
      return optimize_dist_k1 ||
             optimize_dist_k2 ||
             optimize_dist_k3 ||
             optimize_dist_p1_p2;
    case RATIONAL_RADIAL_TANGENTIAL_DISTORTION:
      return optimize_dist_k1 ||
             optimize_dist_k2 ||
             optimize_dist_k3 ||
             optimize_dist_p1_p2 ||
             optimize_dist_k4_k5_k6;
    default:
      break;
  }
  return false;
}

/// enumerate the intrinsics held constant
std::vector< int >
camera_options
::enumerate_constant_intrinsics() const
{
  std::vector< int > constant_intrinsics;

  // number of lens distortion parameters in the selected model
  const unsigned int ndp = num_distortion_params( this->lens_distortion_type );

  if( !this->optimize_focal_length )
  {
    constant_intrinsics.push_back( 0 );
  }
  if( !this->optimize_principal_point )
  {
    constant_intrinsics.push_back( 1 );
    constant_intrinsics.push_back( 2 );
  }
  if( !this->optimize_aspect_ratio )
  {
    constant_intrinsics.push_back( 3 );
  }
  if( !this->optimize_skew )
  {
    constant_intrinsics.push_back( 4 );
  }
  if( !this->optimize_dist_k1 && ndp > 0 )
  {
    constant_intrinsics.push_back( 5 );
  }
  if( !this->optimize_dist_k2 && ndp > 1 )
  {
    constant_intrinsics.push_back( 6 );
  }
  if( !this->optimize_dist_p1_p2 && ndp > 3 )
  {
    constant_intrinsics.push_back( 7 );
    constant_intrinsics.push_back( 8 );
  }
  if( !this->optimize_dist_k3 && ndp > 4 )
  {
    constant_intrinsics.push_back( 9 );
  }
  if( !this->optimize_dist_k4_k5_k6 && ndp > 7 )
  {
    constant_intrinsics.push_back( 10 );
    constant_intrinsics.push_back( 11 );
    constant_intrinsics.push_back( 12 );
  }
  return constant_intrinsics;
}

/// Update a camera object to use extrinsic parameters from an array.
void
camera_options
::update_camera_extrinsics(
  std::shared_ptr< simple_camera_perspective > camera,
  double const* params ) const
{
  camera->set_rotation( rotation_d( vector_3d( Eigen::Map< const vector_3d >(
                                                 params ) ) ) );
  camera->set_center( Eigen::Map< const vector_3d >( &params[ 3 ] ) );
}

/// extract the paramters from camera intrinsics into the parameter array
void
camera_options
::extract_camera_intrinsics( const camera_intrinsics_sptr K,
                             double* params ) const
{
  params[ 0 ] = K->focal_length();
  params[ 1 ] = K->principal_point().x();
  params[ 2 ] = K->principal_point().y();
  params[ 3 ] = K->aspect_ratio();
  params[ 4 ] = K->skew();

  const std::vector< double > d = K->dist_coeffs();
  // copy the intersection of parameters provided in K
  // and those that are supported by the requested model type
  const unsigned int num_dp =
    std::min( num_distortion_params( this->lens_distortion_type ),
              static_cast< unsigned int >( d.size() ) );
  if( num_dp > 0 )
  {
    std::copy( d.begin(), d.begin() + num_dp, &params[ 5 ] );
  }
}

/// update the camera intrinsics from a parameter array
void
camera_options
::update_camera_intrinsics( std::shared_ptr< simple_camera_intrinsics > K,
                            const double* params ) const
{
  K->set_focal_length( params[ 0 ] );

  vector_2d pp( ( Eigen::Map< const vector_2d >( &params[ 1 ] ) ) );
  K->set_principal_point( pp );
  K->set_aspect_ratio( params[ 3 ] );
  K->set_skew( params[ 4 ] );

  // distortion parameters
  const unsigned int ndp = num_distortion_params( this->lens_distortion_type );
  if( ndp > 0 )
  {
    Eigen::VectorXd dc( ndp );
    std::copy( &params[ 5 ], &params[ 5 ] + ndp, dc.data() );
    K->set_dist_coeffs( dc );
  }
}

/// update the camera objects using the extracted camera parameters
void
camera_options
::update_camera_parameters( camera_map::map_camera_t& cameras,
                            cam_param_map_t const& ext_params,
                            std::vector< std::vector< double > > const& int_params,
                            cam_intrinsic_id_map_t const& int_map ) const
{
  std::vector< camera_intrinsics_sptr > updated_intr;
  if( this->optimize_intrinsics() )
  {
    // Update the camera intrinics with optimized values
    for( const std::vector< double >& cip : int_params )
    {
      auto K = std::make_shared< simple_camera_intrinsics >();
      this->update_camera_intrinsics( K, &cip[ 0 ] );
      updated_intr.push_back( camera_intrinsics_sptr( K ) );
    }
  }

  // Update the cameras with the optimized values
  typedef std::map< frame_id_t, std::vector< double > > cam_param_map_t;
  for( const cam_param_map_t::value_type& cp : ext_params )
  {
    auto orig_cam =
      std::dynamic_pointer_cast< camera_perspective >( cameras[ cp.first ] );
    auto simp_cam = std::dynamic_pointer_cast< simple_camera_perspective >(
      orig_cam );
    if( !simp_cam )
    {
      simp_cam = std::make_shared< simple_camera_perspective >();
      cameras[ cp.first ] = simp_cam;
    }

    camera_intrinsics_sptr K = orig_cam->intrinsics();
    if( this->optimize_intrinsics() )
    {
      // look-up updated intrinsics
      auto map_itr = int_map.find( cp.first );
      unsigned int intr_idx = map_itr->second;
      K = updated_intr[ intr_idx ];
    }
    this->update_camera_extrinsics( simp_cam, &cp.second[ 0 ] );
    simp_cam->set_intrinsics( K );
  }
}

} // namespace mvg

} // namespace arrows

} // namespace kwiver
