/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief This file contains the implementation of a local geographical offset coordinate system.
 */

#include <vital/types/local_cartesian.h>
#include <vital/types/geodesy.h>

#include <iomanip>
#include <stdexcept>

namespace kwiver {
namespace vital {

const double PI = 3.14159265358979323e0;
const double PI_OVER_2 = (PI / 2.0e0);
const double TWO_PI = (2.0 * PI);
const double DEG_TO_RAD = (PI / 180.0);
const double RAD_TO_DEG = (180.0 / PI);
const double AD_C = 1.0026000;            /* Toms region 1 constant */
const double COS_67P5 = 0.38268343236508977;  /* cosine of 67.5 degrees */

local_cartesian::local_cartesian()
{
  // Using WGS84 ellipsoid 
  semiMajorAxis = 6378137.0;
  inv_f = 298.257223563;
  flattening = 1 / inv_f;

  Geocent_e2 = 2 * flattening - flattening * flattening;
  Geocent_ep2 = (1 / (1 - Geocent_e2)) - 1;
}

void local_cartesian::set_origin(geo_point const& origin, double orientation)
{
  double N0;
  double val;

  auto loc = origin.location(kwiver::vital::SRID::lat_lon_WGS84);

  LocalCart_Origin_Lat = loc[1]*DEG_TO_RAD;
  LocalCart_Origin_Long = loc[0]*DEG_TO_RAD;
  if (LocalCart_Origin_Long > PI)
    LocalCart_Origin_Long -= TWO_PI;
  LocalCart_Origin_Height = loc[2];
  if (orientation > PI)
  {
    orientation -= TWO_PI;
  }
  LocalCart_Orientation = orientation;
  es2 = 2 * flattening - flattening * flattening;

  Sin_LocalCart_Origin_Lat = sin(LocalCart_Origin_Lat);
  Cos_LocalCart_Origin_Lat = cos(LocalCart_Origin_Lat);
  Sin_LocalCart_Origin_Lon = sin(LocalCart_Origin_Long);
  Cos_LocalCart_Origin_Lon = cos(LocalCart_Origin_Long);
  Sin_LocalCart_Orientation = sin(LocalCart_Orientation);
  Cos_LocalCart_Orientation = cos(LocalCart_Orientation);

  Sin_Lat_Sin_Orient = Sin_LocalCart_Origin_Lat * Sin_LocalCart_Orientation;
  Sin_Lat_Cos_Orient = Sin_LocalCart_Origin_Lat * Cos_LocalCart_Orientation;

  N0 = semiMajorAxis / sqrt(1 - es2 * Sin_LocalCart_Origin_Lat * Sin_LocalCart_Origin_Lat);

  val = (N0 + LocalCart_Origin_Height) * Cos_LocalCart_Origin_Lat;
  u0 = val * Cos_LocalCart_Origin_Lon;
  v0 = val * Sin_LocalCart_Origin_Lon;
  w0 = ((N0 * (1 - es2)) + LocalCart_Origin_Height) * Sin_LocalCart_Origin_Lat;
}

void local_cartesian::convert_from_cartesian(vector_3d const& cartesian_coordinate, geo_point& location) const
{
  vector_3d geodetic;
  convert_from_geodetic(cartesian_coordinate, geodetic);
  location.set_location(geodetic, kwiver::vital::SRID::lat_lon_WGS84);
}

void local_cartesian::convert_to_cartesian(geo_point const& location, vector_3d& cartesian_coordinate) const
{
  convert_from_geodetic(location.location(kwiver::vital::SRID::lat_lon_WGS84), cartesian_coordinate);
}

void local_cartesian::convert_from_geodetic(vector_3d const& geodetic_coordinate, vector_3d& cartesian_coordinate) const
{
  /*
   * The function convertFromGeodetic converts geodetic coordinates
   * (latitude, longitude, and height) to local cartesian coordinates (X, Y, Z),
   * according to the WGS84 ellipsoid and local origin parameters.
   *
   *    geodetic_coordinate[0]  : Geodetic longitude, in degrees                       (input)
   *    geodetic_coordinate[1]  : Geodetic latitude, in degrees                        (input)
   *    geodetic_coordinate[2]  : Geodetic height, in meters                           (input)
   *    cartesian_coordinate[0] : Calculated local cartesian X coordinate, in meters   (output)
   *    cartesian_coordinate[1] : Calculated local cartesian Y coordinate, in meters   (output)
   *    cartesian_coordinate[2] : Calculated local cartesian Z coordinate, in meters   (output)
   */

  double longitude = geodetic_coordinate.x() * DEG_TO_RAD;
  double latitude = geodetic_coordinate.y() * DEG_TO_RAD;
  double height = geodetic_coordinate.z();

  double Rn;            /*  Earth radius at location  */
  double Sin_Lat;       /*  sin(Latitude)  */
  double Sin2_Lat;      /*  Square of sin(Latitude)  */
  double Cos_Lat;       /*  cos(Latitude)  */

  if (longitude > PI)
  {
    longitude -= (2 * PI);
  }
  Sin_Lat = sin(latitude);
  Cos_Lat = cos(latitude);
  Sin2_Lat = Sin_Lat * Sin_Lat;
  Rn = semiMajorAxis / (sqrt(1.0e0 - Geocent_e2 * Sin2_Lat));
  double X = (Rn + height) * Cos_Lat * cos(longitude);
  double Y = (Rn + height) * Cos_Lat * sin(longitude);
  double Z = ((Rn * (1 - Geocent_e2)) + height) * Sin_Lat;

  vector_3d geocentric_coordinate;
  geocentric_coordinate << X, Y, Z;

  convert_from_geocentric(geocentric_coordinate, cartesian_coordinate);
}

void local_cartesian::convert_to_geodetic(vector_3d const& cartesian_coordinate, vector_3d& geodetic_coordinate) const
{
  /*
   * The function convertToGeodetic converts local cartesian
   * coordinates (X, Y, Z) to geodetic coordinates (latitude, longitude,
   * and height), according to the WGS84 ellipsoid and local origin parameters.
   *
   *    cartesian_coordinate[0] : Local cartesian X coordinate, in meters    (input)
   *    cartesian_coordinate[1] : Local cartesian Y coordinate, in meters    (input)
   *    cartesian_coordinate[2] : Local cartesian Z coordinate, in meters    (input)
   *    geodetic_coordinate[0]  : Calculated longitude value, in degrees     (output)
   *    geodetic_coordinate[1]  : Calculated latitude value, in degrees      (output)
   *    geodetic_coordinate[2]  : Calculated height value, in meters         (output)
   */
  vector_3d geocentric_coordinate;
  convert_to_geocentric(cartesian_coordinate, geocentric_coordinate);

  double X = geocentric_coordinate.x();
  double Y = geocentric_coordinate.y();
  double Z = geocentric_coordinate.z();
  double latitude, longitude, height;

  double W;        /* distance from Z axis */
  double W2;       /* square of distance from Z axis */
  double T0;       /* initial estimate of vertical component */
  double T1;       /* corrected estimate of vertical component */
  double S0;       /* initial estimate of horizontal component */
  double S1;       /* corrected estimate of horizontal component */
  double Sin_B0;   /* sin(B0), B0 is estimate of Bowring aux variable */
  double Sin3_B0;  /* cube of sin(B0) */
  double Cos_B0;   /* cos(B0) */
  double Sin_p1;   /* sin(phi1), phi1 is estimated latitude */
  double Cos_p1;   /* cos(phi1) */
  double Rn;       /* Earth radius at location */
  double Sum;      /* numerator of cos(phi1) */
  bool At_Pole;     /* indicates location is in polar region */
  double Geocent_b = semiMajorAxis * (1 - flattening); /* Semi-minor axis of ellipsoid, in meters */

  At_Pole = false;
  if (X != 0.0)
  {
    longitude = atan2(Y, X);
  }
  else
  {
    if (Y > 0)
    {
      longitude = PI_OVER_2;
    }
    else if (Y < 0)
    {
      longitude = -PI_OVER_2;
    }
    else
    {
      At_Pole = true;
      longitude = 0.0;
      if (Z > 0.0)
      {  /* north pole */
        latitude = PI_OVER_2;
      }
      else if (Z < 0.0)
      {  /* south pole */
        latitude = -PI_OVER_2;
      }
      else
      {  /* center of earth */
        latitude = PI_OVER_2;
        height = -Geocent_b;
        geodetic_coordinate << (longitude*RAD_TO_DEG), (latitude*RAD_TO_DEG), height;
        return;
      }
    }
  }
  W2 = X * X + Y * Y;
  W = sqrt(W2);
  T0 = Z * AD_C;
  S0 = sqrt(T0 * T0 + W2);
  Sin_B0 = T0 / S0;
  Cos_B0 = W / S0;
  Sin3_B0 = Sin_B0 * Sin_B0 * Sin_B0;
  T1 = Z + Geocent_b * Geocent_ep2 * Sin3_B0;
  Sum = W - semiMajorAxis * Geocent_e2 * Cos_B0 * Cos_B0 * Cos_B0;
  S1 = sqrt(T1*T1 + Sum * Sum);
  Sin_p1 = T1 / S1;
  Cos_p1 = Sum / S1;
  Rn = semiMajorAxis / sqrt(1.0 - Geocent_e2 * Sin_p1 * Sin_p1);
  if (Cos_p1 >= COS_67P5)
  {
    height = W / Cos_p1 - Rn;
  }
  else if (Cos_p1 <= -COS_67P5)
  {
    height = W / -Cos_p1 - Rn;
  }
  else
  {
    height = Z / Sin_p1 + Rn * (Geocent_e2 - 1.0);
  }
  if (At_Pole == false)
  {
    latitude = atan(Sin_p1 / Cos_p1);
  }

  if (longitude > PI)
  {
    longitude = (longitude -= TWO_PI);
  }
  if (longitude < -PI)
  {
    longitude = (longitude += TWO_PI);
  }
  geodetic_coordinate << (longitude*RAD_TO_DEG), (latitude*RAD_TO_DEG), height;
}

void local_cartesian::convert_from_geocentric(vector_3d const& geocentric_coordinate, vector_3d& cartesian_coordinate) const
{
  /*
   * The function convertFromGeocentric converts geocentric
   * coordinates according to the WGS84 ellipsoid and local origin parameters.
   *
   *    geocentric_coordinate[0] : Geocentric latitude, in meters                       (input)
   *    geocentric_coordinate[1] : Geocentric longitude, in meters                      (input)
   *    geocentric_coordinate[2] : Geocentric height, in meters                         (input)
   *    cartesian_coordinate[0]  : Calculated local cartesian X coordinate, in meters   (output)
   *    cartesian_coordinate[1]  : Calculated local cartesian Y coordinate, in meters   (output)
   *    cartesian_coordinate[2]  : Calculated local cartesian Z coordinate, in meters   (output)
   */

  double X, Y, Z;
  double u_MINUS_u0, v_MINUS_v0, w_MINUS_w0;

  double U = geocentric_coordinate.x();
  double V = geocentric_coordinate.y();
  double W = geocentric_coordinate.z();

  u_MINUS_u0 = U - u0;
  v_MINUS_v0 = V - v0;
  w_MINUS_w0 = W - w0;

  if (LocalCart_Orientation == 0.0)
  {
    double cos_lon_u_MINUS_u0 = Cos_LocalCart_Origin_Lon * u_MINUS_u0;
    double sin_lon_v_MINUS_v0 = Sin_LocalCart_Origin_Lon * v_MINUS_v0;

    X = -Sin_LocalCart_Origin_Lon * u_MINUS_u0 + Cos_LocalCart_Origin_Lon * v_MINUS_v0;
    Y = -Sin_LocalCart_Origin_Lat * cos_lon_u_MINUS_u0 + -Sin_LocalCart_Origin_Lat * sin_lon_v_MINUS_v0 + Cos_LocalCart_Origin_Lat * w_MINUS_w0;
    Z = Cos_LocalCart_Origin_Lat * cos_lon_u_MINUS_u0 + Cos_LocalCart_Origin_Lat * sin_lon_v_MINUS_v0 + Sin_LocalCart_Origin_Lat * w_MINUS_w0;
  }
  else
  {
    double cos_lat_w_MINUS_w0 = Cos_LocalCart_Origin_Lat * w_MINUS_w0;

    X = (-Cos_LocalCart_Orientation * Sin_LocalCart_Origin_Lon + Sin_Lat_Sin_Orient * Cos_LocalCart_Origin_Lon) * u_MINUS_u0 +
      (Cos_LocalCart_Orientation * Cos_LocalCart_Origin_Lon + Sin_Lat_Sin_Orient * Sin_LocalCart_Origin_Lon) * v_MINUS_v0 +
      (-Sin_LocalCart_Orientation * cos_lat_w_MINUS_w0);

    Y = (-Sin_LocalCart_Orientation * Sin_LocalCart_Origin_Lon - Sin_Lat_Cos_Orient * Cos_LocalCart_Origin_Lon) * u_MINUS_u0 +
      (Sin_LocalCart_Orientation * Cos_LocalCart_Origin_Lon - Sin_Lat_Cos_Orient * Sin_LocalCart_Origin_Lon) * v_MINUS_v0 +
      (Cos_LocalCart_Orientation * cos_lat_w_MINUS_w0);

    Z = (Cos_LocalCart_Origin_Lat * Cos_LocalCart_Origin_Lon) * u_MINUS_u0 +
      (Cos_LocalCart_Origin_Lat * Sin_LocalCart_Origin_Lon) * v_MINUS_v0 +
      Sin_LocalCart_Origin_Lat * w_MINUS_w0;
  }

  cartesian_coordinate << X, Y, Z;
}

void local_cartesian::convert_to_geocentric(vector_3d const& cartesian_coordinates, vector_3d& geocentric_coordinates) const
{
  /*
   * The function Convert_Local_Cartesian_To_Geocentric converts local cartesian
   * coordinates (x, y, z) to geocentric coordinates (X, Y, Z) according to the
   * current ellipsoid and local origin parameters.
   *
   *    cartesian_coordinates[0]  : Local cartesian X coordinate, in meters    (input)
   *    cartesian_coordinates[1]  : Local cartesian Y coordinate, in meters    (input)
   *    cartesian_coordinates[2]  : Local cartesian Z coordinate, in meters    (input)
   *    geocentric_coordinates[0] : Calculated U value, in meters              (output)
   *    geocentric_coordinates[1] : Calculated v value, in meters              (output)
   *    geocentric_coordinates[2] : Calculated w value, in meters              (output)
   */

  double U, V, W;

  double X = cartesian_coordinates.x();
  double Y = cartesian_coordinates.y();
  double Z = cartesian_coordinates.z();

  if (LocalCart_Orientation == 0.0)
  {
    double sin_lat_y = Sin_LocalCart_Origin_Lat * Y;
    double cos_lat_z = Cos_LocalCart_Origin_Lat * Z;

    U = -Sin_LocalCart_Origin_Lon * X - sin_lat_y * Cos_LocalCart_Origin_Lon + cos_lat_z * Cos_LocalCart_Origin_Lon + u0;
    V = Cos_LocalCart_Origin_Lon * X - sin_lat_y * Sin_LocalCart_Origin_Lon + cos_lat_z * Sin_LocalCart_Origin_Lon + v0;
    W = Cos_LocalCart_Origin_Lat * Y + Sin_LocalCart_Origin_Lat * Z + w0;
  }
  else
  {
    double rotated_x, rotated_y;
    double rotated_y_sin_lat, z_cos_lat;

    rotated_x = Cos_LocalCart_Orientation * X + Sin_LocalCart_Orientation * Y;
    rotated_y = -Sin_LocalCart_Orientation * X + Cos_LocalCart_Orientation * Y;

    rotated_y_sin_lat = rotated_y * Sin_LocalCart_Origin_Lat;
    z_cos_lat = Z * Cos_LocalCart_Origin_Lat;

    U = -Sin_LocalCart_Origin_Lon * rotated_x - Cos_LocalCart_Origin_Lon * rotated_y_sin_lat + Cos_LocalCart_Origin_Lon * z_cos_lat + u0;
    V = Cos_LocalCart_Origin_Lon * rotated_x - Sin_LocalCart_Origin_Lon * rotated_y_sin_lat + Sin_LocalCart_Origin_Lon * z_cos_lat + v0;
    W = Cos_LocalCart_Origin_Lat * rotated_y + Sin_LocalCart_Origin_Lat * Z + w0;
  }

  geocentric_coordinates << U, V, W;
}


} } // end namespace
