// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/geocalc/geo_conv.h>

#include <arrows/geocalc/constants.h>

#include <vital/math_constants.h>

#include <algorithm>
#include <stdexcept>

namespace kwiver {

namespace arrows {

namespace geocalc {

namespace {

// ----------------------------------------------------------------------------
// https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_geodetic_to_ECEF_coordinates
template < class E >
vital::vector_3d
geodetic_to_ecef( vital::vector_3d const& geodetic )
{
  auto const sin_lon = std::sin( geodetic[ 0 ] * vital::deg_to_rad );
  auto const cos_lon = std::cos( geodetic[ 0 ] * vital::deg_to_rad );
  auto const sin_lat = std::sin( geodetic[ 1 ] * vital::deg_to_rad );
  auto const cos_lat = std::cos( geodetic[ 1 ] * vital::deg_to_rad );
  auto const hae = geodetic[ 2 ];
  auto const inside_sqrt = 1.0 - E::e2 * sin_lat * sin_lat;
  if( inside_sqrt <= 0.0 )
  {
    return { 0.0, 0.0, ( sin_lat > 0.0 ? 1.0 : -1.0 ) * ( E::b + hae ) };
  }

  auto const prime_vertical_radius = E::a / std::sqrt( inside_sqrt );
  auto const x = ( prime_vertical_radius + hae ) * cos_lat * cos_lon;
  auto const y = ( prime_vertical_radius + hae ) * cos_lat * sin_lon;
  auto const z = ( ( 1.0 - E::e2 ) * prime_vertical_radius + hae ) * sin_lat;
  return { x, y, z };
}

// ----------------------------------------------------------------------------
// https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_geodetic_coordinates
template < class E >
vital::vector_3d
ecef_to_geodetic_iterative( vital::vector_3d const& ecef )
{
  constexpr size_t max_iterations = 10;

  auto const x = ecef[ 0 ];
  auto const y = ecef[ 1 ];
  auto const z = ecef[ 2 ];
  auto const x2 = x * x;
  auto const y2 = y * y;
  auto const z2 = z * z;
  auto const p = std::sqrt( x2 + y2 );
  auto const l = E::b - std::sqrt( x2 + y2 + z2 );
  auto const a2_prime = std::pow( E::a - l, 2.0 );
  auto const b2_prime = std::pow( E::b - l, 2.0 );
  auto const e2_prime = ( a2_prime - b2_prime ) / a2_prime;

  // Iteratively refine estimate of k, the effective "aspect ratio" of the
  // ellipsoid at the given altitude
  auto k = 1.0 / ( 1.0 - e2_prime );
  for( size_t i = 0; i < max_iterations; ++i )
  {
    auto const c =
      std::pow(
        x2 + y2 + ( 1.0 - E::e2 ) * z2 * k * k, 1.5 ) /
      ( E::a * E::e2 );
    auto const new_k =
      1.0 + ( x2 + y2 + ( 1.0 - E::e2 ) * z2 * k * k * k ) /
      ( c - ( x2 + y2 ) );
    auto const adjustment = new_k - k;
    k = new_k;
    if( std::abs( adjustment ) < 1e-15 )
    {
      break;
    }
  }

  // Geodetic values can be calculated easily from k
  auto const lon = std::atan2( y, x );
  auto const lat = z ? std::atan( z / p * k ) : 0.0;
  auto const hae =
    std::sqrt( x2 + y2 + z2 * k * k ) * ( 1.0 / k - 1.0 + E::e2 ) / E::e2;

  return { lon* vital::rad_to_deg, lat* vital::rad_to_deg, hae };
}

// ----------------------------------------------------------------------------
// https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_geodetic_coordinates
template < class E >
vital::vector_3d
ecef_to_geodetic( vital::vector_3d const& ecef )
{
  auto const x = ecef[ 0 ];
  auto const y = ecef[ 1 ];
  auto const z = ecef[ 2 ];

  if( !x && !y )
  {
    // Along polar axis
    if( z > 0.0 )
    {
      // North pole
      return { 0.0, 90.0, z - E::b };
    }
    else if( z < 0.0 )
    {
      // South pole
      return { 0.0, -90.0, -z - E::b };
    }
    else // z == 0.0
    {
      // Earth center
      return { 0.0, 0.0, -E::a };
    }
  }

  auto const x2 = x * x;
  auto const y2 = y * y;
  auto const z2 = z * z;

  if( std::sqrt( x2 + y2 + z2 ) < E::a * 0.02 )
  {
    // Fall back to iterative solution when near center of Earth
    return ecef_to_geodetic_iterative< E >( ecef );
  }

  // This is a distilled version of the iterative method equivalent to a single
  // iteration. It's accurate when far away from the center of the Earth
  // (i.e. on the surface, where we care about)
  auto const p = std::sqrt( x2 + y2 );
  auto const F = 54.0 * E::b2 * z2;
  auto const G = p * p + ( 1.0 - E::e2 ) * z2 - E::e2 * E::a2_minus_b2;
  auto const c = E::e2 * E::e2 * F * p * p / ( G * G * G );
  auto const s = std::pow( 1.0 + c + std::sqrt( c * c + 2.0 * c ), 1.0 / 3.0 );
  auto const k = s + 1.0 + 1.0 / s;
  auto const P = F / ( 3.0 * k * k * G * G );
  auto const Q = std::sqrt( 1.0 + 2.0 * E::e2 * E::e2 * P );
  auto const inside_sqrt =
    0.5 * E::a2 * ( 1.0 + 1.0 / Q ) -
    P * ( 1.0 - E::e2 ) * z2 / ( Q * ( 1.0 + Q ) ) -
    0.5 * P * p * p;
  auto r0 = -P * E::e2 * p / ( 1.0 + Q );
  if( inside_sqrt > 0.0 )
  {
    // This is the only square root that could be negative due to rounding error
    r0 += std::sqrt( inside_sqrt );
  }

  auto const per0 = p - E::e2 * r0;
  auto const U = std::sqrt( per0 * per0 + z2 );
  auto const V = std::sqrt( per0 * per0 + ( 1.0 - E::e2 ) * z2 );
  auto const z0 = E::b2 * z / ( E::a * V );

  auto const lat = std::atan( ( z + E::eprime2 * z0 ) / p );
  auto const lon = std::atan2( y, x );
  auto const hae = U * ( 1.0 - E::b2 / ( E::a * V ) );

  return { lon* vital::rad_to_deg, lat* vital::rad_to_deg, hae };
}

using namespace vital::SRID;
using conversion_map_t =
  std::map<
    std::pair< int, int >, // from, to
    vital::vector_3d ( * )( vital::vector_3d const& ) >;

// ----------------------------------------------------------------------------
conversion_map_t const&
get_conversion_map()
{
  static conversion_map_t map = {
    { { lat_lon_WGS84, ECEF_WGS84 }, &geodetic_to_ecef< ellipsoid_wgs84 > },
    { { ECEF_WGS84, lat_lon_WGS84 }, &ecef_to_geodetic< ellipsoid_wgs84 > }, };
  return map;
}

// ----------------------------------------------------------------------------
// Ranking of CRS's for use as intermediate representations
int
get_intermediate_score( int crs )
{
  switch( crs )
  {
    case ECEF_WGS84: return 2;
    case lat_lon_WGS84: return 1;
    default: return 0;
  }
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
geo_conversion&
geo_conversion
::instance()
{
  static geo_conversion _instance;
  return _instance;
}

// ----------------------------------------------------------------------------
char const*
geo_conversion
::id() const
{
  return "geocalc";
}

// ----------------------------------------------------------------------------
vital::geo_crs_description_t
geo_conversion
::describe( [[maybe_unused]] int crs )
{
  throw std::runtime_error( "Not implemented" );
}

// ----------------------------------------------------------------------------
vital::vector_2d
geo_conversion::operator()( vital::vector_2d const& point, int from, int to )
{
  vital::vector_3d point3{ point[ 0 ], point[ 1 ], 0.0 };
  return ( *this )( point3, from, to ).head< 2 >();
}

// ----------------------------------------------------------------------------
vital::vector_3d
geo_conversion::operator()( vital::vector_3d const& point, int from, int to )
{
  if( !point.allFinite() )
  {
    auto const nan = std::numeric_limits< double >::quiet_NaN();
    return vital::vector_3d{ nan, nan, nan };
  }

  auto const& conversion_map = get_conversion_map();

  // Check for a direct conversion
  if( auto const it = conversion_map.find( { from, to } );
      it != conversion_map.end() )
  {
    return it->second( point );
  }

  // Search for an intermediate representation for which there is a conversion
  struct conversion_path
  {
    conversion_map_t::mapped_type conversion1;
    conversion_map_t::mapped_type conversion2;
    int score;
  };

  std::vector< conversion_path > paths;
  for( auto it = conversion_map.lower_bound( { from, INT_MIN } );
       it != conversion_map.end() && it->first.first == from; ++it )
  {
    auto const intermediate = it->first.second;
    if( auto const jt = conversion_map.find( { intermediate, to } );
        jt != conversion_map.end() )
    {
      auto const score = get_intermediate_score( intermediate );
      paths.push_back( { it->second, jt->second, score } );
    }
  }

  if( paths.empty() )
  {
    throw std::runtime_error( "Not implemented" );
  }

  // Find best intermediate path and execute it
  auto const cmp =
    []( conversion_path const& lhs, conversion_path const& rhs ){
      return lhs.score < rhs.score;
    };
  auto const best_path =
    std::max_element( paths.begin(), paths.end(), cmp );
  return best_path->conversion2( best_path->conversion1( point ) );
}

} // namespace geocalc

} // namespace arrows

} // namespace kwiver
