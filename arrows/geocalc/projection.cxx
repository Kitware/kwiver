// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/geocalc/projection.h>

#include <arrows/geocalc/constants.h>

#include <vital/types/geodesy.h>

namespace kwiver {

namespace arrows {

namespace geocalc {

namespace {

// ----------------------------------------------------------------------------
template < class E >
std::optional< vital::vector_3d >
raycast_ecef_to_ellipsoid_impl(
  vital::vector_3d const& point,
  vital::vector_3d const& vector_in,
  double altitude,
  bool throw_on_interior_point )
{
  if( !point.allFinite() )
  {
    throw std::runtime_error( "raycast_ecef_to_ellipsoid(): invalid point" );
  }

  if( vector_in.isZero() || !vector_in.allFinite() )
  {
    throw std::runtime_error( "raycast_ecef_to_ellipsoid(): invalid vector" );
  }

  auto const a = E::a + altitude;
  auto const b = E::b + altitude;

  if( !std::isfinite( altitude ) || a <= 0.0 || b <= 0.0 )
  {
    throw std::runtime_error( "raycast_ecef_to_ellipsoid(): invalid altitude" );
  }

  vital::vector_3d vector = vector_in.normalized();

  // Scale everything so the Earth is a unit sphere
  vital::vector_3d scale{ 1.0 / a, 1.0 / a, 1.0 / b };
  vital::vector_3d p = point.array() * scale.array();
  vital::vector_3d v = vector.array() * scale.array();

  // Components of quadratic equation at^2 + bt + c = 0
  auto const quad_a = v.dot( v );
  auto const quad_b = 2.0 * p.dot( v );
  auto const quad_c = p.dot( p ) - 1.0;

  auto const determinant = quad_b * quad_b - 4.0 * quad_a * quad_c;
  if( determinant < 0 )
  {
    // No intersection; looking skew to ellipsoid
    return std::nullopt;
  }

  auto const t0 = ( -quad_b - std::sqrt( determinant ) ) / ( 2.0 * quad_a );
  auto const t1 = ( -quad_b + std::sqrt( determinant ) ) / ( 2.0 * quad_a );

  if( t0 >= 0 )
  {
    // Successful raycast from outside the ellipsoid to the surface
    return point + vector * t0;
  }

  if( t1 >= 0 )
  {
    // Successful raycast from inside the ellipsoid to the surface
    if( throw_on_interior_point )
    {
      throw std::runtime_error(
        "raycast_ecef_to_ellipsoid(): point is inside ellipsoid" );
    }
    return point + vector * t1;
  }

  // No intersection; looking away from ellipsoid
  return std::nullopt;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
std::optional< vital::vector_3d >
raycast_ecef_to_ellipsoid(
  vital::vector_3d const& point,
  vital::vector_3d const& vector,
  int crs,
  double altitude,
  bool throw_on_interior_point )
{
  switch( crs )
  {
    case vital::SRID::ECEF_WGS84:
      return raycast_ecef_to_ellipsoid_impl< ellipsoid_wgs84 >(
        point, vector, altitude, throw_on_interior_point );
    default:
      throw std::runtime_error(
        "raycast_ecef_to_ellipsoid(): unsupported CRS" );
  }
}

} // namespace geocalc

} // namespace arrows

} // namespace kwiver
