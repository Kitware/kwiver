// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Header for lens distortion functions
 */

#ifndef KWIVER_ARROWS_MVG_LENS_DISTORTION_H_
#define KWIVER_ARROWS_MVG_LENS_DISTORTION_H_

#include <vital/vital_config.h>
#include <arrows/mvg/kwiver_algo_mvg_export.h>

#include <string>

namespace kwiver {
namespace arrows {
namespace mvg{

/// The various models for lens distortion supported in the config
enum LensDistortionType
{
  NO_DISTORTION,
  POLYNOMIAL_RADIAL_DISTORTION,
  POLYNOMIAL_RADIAL_TANGENTIAL_DISTORTION,
  RATIONAL_RADIAL_TANGENTIAL_DISTORTION
};

/// Provide a string representation for a LensDisortionType value
KWIVER_ALGO_MVG_EXPORT
const char*
LensDistortionTypeToString(LensDistortionType type);

/// Parse a LensDistortionType value from a string or return false
KWIVER_ALGO_MVG_EXPORT
bool
StringToLensDistortionType(std::string value, LensDistortionType* type);

/// Return the number of distortion parameters required for each type
KWIVER_ALGO_MVG_EXPORT
unsigned int
num_distortion_params(LensDistortionType type);

/// Class to hold to distortion function and traits
class distortion_poly_radial
{
public:
  // the number of distortion coefficients
  static const int num_coeffs = 2;

  /// Function to apply polynomial radial distortion
  /**
   * \param [in] dist_coeffs: radial distortion coefficients (2)
   * \param [in] source_xy: 2D point in normalized image coordinates
   * \param [out] distorted_xy: 2D point in distorted normalized image coordinates
   */
  template <typename T>
  static void apply(const T* dist_coeffs,
                    const T* source_xy,
                          T* distorted_xy)
  {
    const T& x = source_xy[0];
    const T& y = source_xy[1];

    // distortion parameters
    const T& k1 = dist_coeffs[0];
    const T& k2 = dist_coeffs[1];

    // apply radial distortion
    const T r2 = x*x + y*y;
    const T scale = T(1) + k1*r2 + k2*r2*r2;
    distorted_xy[0] = x*scale;
    distorted_xy[1] = y*scale;
  }
};

/// Class to hold to distortion function and traits
class distortion_poly_radial_tangential
{
public:
  // the number of distortion coefficients
  static const int num_coeffs = 5;

  /// Function to apply polynomial radial and tangential distortion
  /**
   * \param [in] dist_coeffs: radial (3) and tangential (2) distortion
   *                            coefficients
   * \param [in] source_xy: 2D point in normalized image coordinates
   * \param [out] distorted_xy: 2D point in distorted normalized image coordinates
   */
  template <typename T>
  static void apply(const T* dist_coeffs,
                    const T* source_xy,
                          T* distorted_xy)
  {
    const T& x = source_xy[0];
    const T& y = source_xy[1];

    // distortion parameters
    const T& k1 = dist_coeffs[0];
    const T& k2 = dist_coeffs[1];
    const T& p1 = dist_coeffs[2];
    const T& p2 = dist_coeffs[3];
    const T& k3 = dist_coeffs[4];

    // apply radial distortion
    const T x2 = x*x;
    const T y2 = y*y;
    const T xy = x*y;
    const T r2 = x2 + y2;
    const T r4 = r2*r2;
    const T scale = T(1) + k1*r2 + k2*r4 + k3*r2*r4;
    distorted_xy[0] = x*scale + T(2)*p1*xy + p2*(r2 + T(2)*x2);
    distorted_xy[1] = y*scale + T(2)*p2*xy + p1*(r2 + T(2)*y2);
  }
};

/// Class to hold to distortion function and traits
class distortion_ratpoly_radial_tangential
{
public:
  // the number of distortion coefficients
  static const int num_coeffs = 8;

  /// Function to apply rational polynomial radial and tangential distortion
  /**
   * \param [in] dist_coeffs: radial (6) and tangential (2) distortion
   *                          coefficients
   * \param [in] source_xy: 2D point in normalized image coordinates
   * \param [out] distorted_xy: 2D point in distorted normalized image coordinates
   */
  template <typename T>
  static void apply(const T* dist_coeffs,
                    const T* source_xy,
                          T* distorted_xy)
  {
    const T& x = source_xy[0];
    const T& y = source_xy[1];

    // distortion parameters
    const T& k1 = dist_coeffs[0];
    const T& k2 = dist_coeffs[1];
    const T& p1 = dist_coeffs[2];
    const T& p2 = dist_coeffs[3];
    const T& k3 = dist_coeffs[4];
    const T& k4 = dist_coeffs[5];
    const T& k5 = dist_coeffs[6];
    const T& k6 = dist_coeffs[7];

    // apply radial distortion
    const T x2 = x*x;
    const T y2 = y*y;
    const T xy = x*y;
    const T r2 = x2 + y2;
    const T r4 = r2*r2;
    const T r6 = r4*r2;
    const T scale = (T(1) + k1*r2 + k2*r4 + k3*r6) /
                    (T(1) + k4*r2 + k5*r4 + k6*r6);
    distorted_xy[0] = x*scale + T(2)*p1*xy + p2*(r2 + T(2)*x2);
    distorted_xy[1] = y*scale + T(2)*p2*xy + p1*(r2 + T(2)*y2);
  }
};

} // namespace mvg
} // namespace arrows
} // namespace kwiver

#endif
