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

/**
 * @Brief Local Cartesian Conversion Math Utility 
 *
 * Base on the NGA GeoTrans library
 * https://earth-info.nga.mil/GandG/update/index.php?action=home
 *
 * This class is a cleaned up version of the LocalCartesian and Geocentric classes provided in GeoTrans
 * This allows a user to define a local cartesian coordinate system with any origin (expressed in WGS84)
 */
class local_cartesian::geotrans
{
public:
  geotrans(geo_point const& origin, double orientation)
  {
    // Using WGS84 ellipsoid 
    semiMajorAxis = 6378137.0;
    inv_f = 298.257223563;
    flattening = 1 / inv_f;

    Geocent_e2 = 2 * flattening - flattening * flattening;
    Geocent_ep2 = (1 / (1 - Geocent_e2)) - 1;
    set_origin(origin, orientation);
  }

  /**
   * @brief Set origin of the cartesian system as a geo_point
   *
   * Set local origin parameters as inputs and sets the corresponding state variables.
   * NOTE : If the origin changes, this method needs to be called again to recompute
   *        variables needed in the conversion math.
   *
   * @param [in] geo_point       : Geograpical origin of the cartesian system
   * @param [in] orientation     : Orientation angle of the local cartesian coordinate system,
   *                               in radians along the Z axis, normal to the earth surface
   */
  void set_origin(geo_point const& origin, double orientation)
  {
    if (origin.is_empty())
    {
      throw std::runtime_error("Origin geo_point is empty");
    }

    double N0;
    double val;

    auto loc = origin.location(kwiver::vital::SRID::lat_lon_WGS84);

    LocalCart_Origin_Lat = loc[1] * DEG_TO_RAD;
    LocalCart_Origin_Long = loc[0] * DEG_TO_RAD;
    if (LocalCart_Origin_Long > PI)
      LocalCart_Origin_Long -= TWO_PI;
    LocalCart_Origin_Height = loc[2];
    if (orientation > PI)
    {
      orientation -= TWO_PI;
    }
    LocalCart_Orientation = orientation;

    Sin_LocalCart_Origin_Lat = sin(LocalCart_Origin_Lat);
    Cos_LocalCart_Origin_Lat = cos(LocalCart_Origin_Lat);
    Sin_LocalCart_Origin_Lon = sin(LocalCart_Origin_Long);
    Cos_LocalCart_Origin_Lon = cos(LocalCart_Origin_Long);
    Sin_LocalCart_Orientation = sin(LocalCart_Orientation);
    Cos_LocalCart_Orientation = cos(LocalCart_Orientation);

    Sin_Lat_Sin_Orient = Sin_LocalCart_Origin_Lat * Sin_LocalCart_Orientation;
    Sin_Lat_Cos_Orient = Sin_LocalCart_Origin_Lat * Cos_LocalCart_Orientation;

    N0 = semiMajorAxis / sqrt(1 - Geocent_e2 * Sin_LocalCart_Origin_Lat * Sin_LocalCart_Origin_Lat);

    val = (N0 + LocalCart_Origin_Height) * Cos_LocalCart_Origin_Lat;
    u0 = val * Cos_LocalCart_Origin_Lon;
    v0 = val * Sin_LocalCart_Origin_Lon;
    w0 = ((N0 * (1 - Geocent_e2)) + LocalCart_Origin_Height) * Sin_LocalCart_Origin_Lat;
  }

  /**
   * @brief Converts geodetic coordinates to local cartesian coordinates
   *
   * The function convertFromGeodetic converts geodetic coordinates
   * (latitude, longitude, and height) to local cartesian coordinates (X, Y, Z),
   * according to the WGS84 ellipsoid and local origin parameters.
   *
   * @param [in]     geodetic_coordinate : WGS84 longitude/latitude in degrees and hight in meters
   * @param [in,out] cartesian_coordinate : The location in the local coordinate system, in meters
   */
  void convert_from_geodetic(vector_3d const& geodetic_coordinate, vector_3d& cartesian_coordinate) const
  {
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

  /**
   * @brief Converts local cartesian coordinates to geodetic coordinates
   *
   * The function convertToGeodetic converts local cartesian
   * coordinates (X, Y, Z) to geodetic coordinates (latitude, longitude,
   * and height), according to the WGS84 ellipsoid and local origin parameters.
   *
   * @param [in]     cartesian_coordinate : The location in the local coordinate system, in meters
   * @param [in,out] geodetic_coordinate  : WGS84 longitude/latitude in degrees and hight, in meters
   */
  void convert_to_geodetic(vector_3d const& cartesian_coordinate, vector_3d& geodetic_coordinate) const
  {
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

  /**
   * @brief Converts geocentric coordinates to cartesian coordinates
   *
   * The function convertFromGeocentric converts geocentric
   * coordinates according to the WGS84 ellipsoid and local origin parameters.
   *
   * @param [in]     geocentric_coordinate : The geocentric location, in meters
   * @param [in,out] cartesian_coordinate  : Calculated local cartesian coordinate, in meters
   */
  void convert_from_geocentric(vector_3d const& geocentric_coordinate, vector_3d& cartesian_coordinate) const
  {
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

  /**
   * @brief Converts cartesian coordinates to geocentric coordinates
   *
   * The function Convert_Local_Cartesian_To_Geocentric converts local cartesian
   * coordinates (x, y, z) to geocentric coordinates (X, Y, Z) according to the
   * current ellipsoid and local origin parameters.
   *
   * @param [in,out] cartesian_coordinate  : Local cartesian coordinate, in meters
   * @param [in]     geocentric_coordinate : The geocentric location, in meters
   */
  void convert_to_geocentric(vector_3d const& cartesian_coordinate, vector_3d& geocentric_coordinate) const
  {
    double U, V, W;

    double X = cartesian_coordinate.x();
    double Y = cartesian_coordinate.y();
    double Z = cartesian_coordinate.z();

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

    geocentric_coordinate << U, V, W;
  }
private:
  // Ellipsoid Parameters
// All are set to a WGS 84 ellipsoid
  double semiMajorAxis;
  double flattening;
  double inv_f;
  double Geocent_e2;
  double Geocent_ep2;

  double es2;                       /* Eccentricity (0.08181919084262188000) squared */
  double u0;                        /* Geocentric origin coordinates in */
  double v0;                        /* terms of Local Cartesian origin  */
  double w0;                        /* parameters                       */

  /* Local Cartesian Projection Parameters */
  double LocalCart_Origin_Lat;      /* Latitude of origin in radians     */
  double LocalCart_Origin_Long;     /* Longitude of origin in radians    */
  double LocalCart_Origin_Height;   /* Height of origin in meters        */
  double LocalCart_Orientation;     /* Orientation of Y axis in radians  */

  double Sin_LocalCart_Origin_Lat;  /* sin(LocalCart_Origin_Lat)         */
  double Cos_LocalCart_Origin_Lat;  /* cos(LocalCart_Origin_Lat)         */
  double Sin_LocalCart_Origin_Lon;  /* sin(LocalCart_Origin_Lon)         */
  double Cos_LocalCart_Origin_Lon;  /* cos(LocalCart_Origin_Lon)         */
  double Sin_LocalCart_Orientation; /* sin(LocalCart_Orientation)        */
  double Cos_LocalCart_Orientation; /* cos(LocalCart_Orientation)        */

  double Sin_Lat_Sin_Orient; /* sin(LocalCart_Origin_Lat) * sin(LocalCart_Orientation) */
  double Sin_Lat_Cos_Orient; /* sin(LocalCart_Origin_Lat) * cos(LocalCart_Orientation) */
  double Cos_Lat_Cos_Orient; /* cos(LocalCart_Origin_Lat) * cos(LocalCart_Orientation) */
  double Cos_Lat_Sin_Orient; /* cos(LocalCart_Origin_Lat) * sin(LocalCart_Orientation) */
};

local_cartesian::local_cartesian(geo_point const& origin, double orientation)
{
  origin_ = origin;
  orientation_ = orientation;
  geotrans_ = new geotrans(origin, orientation);
}

void local_cartesian::set_origin(geo_point const& origin, double orientation)
{
  origin_ = origin;
  orientation_ = orientation;
  geotrans_->set_origin(origin, orientation);
}

geo_point local_cartesian::get_origin() const
{
  return origin_;
}

double local_cartesian::get_orientation() const
{
  return orientation_;
}

void local_cartesian::convert_from_cartesian(vector_3d const& cartesian_coordinate, geo_point& location) const
{
  vector_3d geodetic;
  geotrans_->convert_from_geodetic(cartesian_coordinate, geodetic);
  location.set_location(geodetic, kwiver::vital::SRID::lat_lon_WGS84);
}

void local_cartesian::convert_to_cartesian(geo_point const& location, vector_3d& cartesian_coordinate) const
{
  geotrans_->convert_from_geodetic(location.location(kwiver::vital::SRID::lat_lon_WGS84), cartesian_coordinate);
}

} } // end namespace
