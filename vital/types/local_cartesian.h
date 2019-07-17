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
 * \brief This file contains the interface to a local geographic offset coordinate system.
 */

#ifndef KWIVER_VITAL_LOCAL_CARTESIAN_H_
#define KWIVER_VITAL_LOCAL_CARTESIAN_H_

#include <vital/types/point.h>
#include <vital/types/geo_point.h>

namespace kwiver {
namespace vital {

// ----------------------------------------------------------------------------
/** Local Cartesian Conversion Utility.
 *
 * Base on the NGA GeoTrans library
 * https://earth-info.nga.mil/GandG/update/index.php?action=home
 *
 * This class is a cleaned up version of the LocalCartesian and Geocentric classes provided in GeoTrans
 * This allows a user to define a local cartesian coordinate system with any origin (expressed in WGS84)
 */
class VITAL_EXPORT local_cartesian
{
public:

  local_cartesian();
  virtual ~local_cartesian() = default;

  /**
   * Set local origin parameters as inputs and sets the corresponding state variables.
   * NOTE : If the origin changes, this method needs to be called again to recompute
   *        variables needed in the conversion math.
   *
   *    geo_point                : Geograpical origin of the cartesian system        (input)
   *    orientation              : Orientation angle of the local cartesian coordinate system,
   *                               in radians                                        (input)
   */
  void set_origin(geo_point const& origin, double orientation=0);

  /**
   * \brief Get the WGS84 coordinates of the cartesian coordinates.
   *
   * \returns The WGS84 lon/lat/alt of the cartesian coordinates.
   */
  void convert_from_cartesian(vector_3d const& cartesian_coordinate, geo_point& location) const;

  /**
   * \brief Set the cartesian coordinates based on 2 geo_points
   */
  void convert_to_cartesian(geo_point const& location, vector_3d& cartesian_coordinate) const;

  ////////////////////////////////////////////////////////////////////////////////////
  // METHODS BELOW NOTICE                                                           //
  // The methods below are used in the convertion math for the above methods        //
  // They are not intended for general public use, but they are left public         //
  // if you find a need for using world cartesian coordinates (non local to origin) //
  ////////////////////////////////////////////////////////////////////////////////////

  /**
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
  void convert_from_geodetic(vector_3d const& geodetic_coordinate, vector_3d& cartesian_coordinate) const;

  /**
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
  void convert_to_geodetic(vector_3d const& cartesian_coordinate, vector_3d& geodetic_coordinate) const;

  /**
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
  void convert_from_geocentric(vector_3d const& geocentric_coordinate, vector_3d& cartesian_coordinate) const;

  /**
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
  void convert_to_geocentric(vector_3d const& cartesian_coordinate, vector_3d& geocentric_coordinate) const;

private:

  // Ellipsoid Parameters.
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

} } // end namespace

#endif /* KWIVER_VITAL_LOCAL_CARTESIAN_H_ */
