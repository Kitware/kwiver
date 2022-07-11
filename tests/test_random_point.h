// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Functions for creating test points with added random Gaussian noise.

#ifndef KWIVER_TESTS_TEST_RANDOM_POINT_H_
#define KWIVER_TESTS_TEST_RANDOM_POINT_H_

#include <vital/types/vector.h>
#include <vital/vital_config.h>

#include <random>

namespace kwiver {

namespace testing {

/// Random number generator type
using rng_t = std::mt19937;

/// Normal distribution
using norm_dist_t = std::normal_distribution< double >;

/// Global random number generator instance
static rng_t rng;

// ------------------------------------------------------------------
// Limit occasional outliers in the normal distribution.
inline double
bounded_normal_noise( double stdev, double max_stdevs )
{
  auto result = 0.0;
  if( stdev <= 0.0 || max_stdevs <= 0.0 )
  {
    return result;
  }

  norm_dist_t norm( 0.0, stdev );
  do
  {
    result = norm( rng );
  } while( std::abs( result / stdev ) > max_stdevs );
  return result;
}

// ------------------------------------------------------------------
inline kwiver::vital::vector_3d
random_point3d( double stdev )
{
  auto const generate =
    [ stdev ](){ return bounded_normal_noise( stdev, 2.0 ); };
  return { generate(), generate(), generate() };
}

// ------------------------------------------------------------------
inline kwiver::vital::vector_2d
random_point2d( double stdev )
{
  auto const generate =
    [ stdev ](){ return bounded_normal_noise( stdev, 2.0 ); };
  return { generate(), generate() };
}

} // end namespace testing

} // end namespace kwiver

#endif
