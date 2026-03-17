// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 *
 * \brief Functions to create a set of features with attributes
 * for testing the filter_features implementations
 *
 * These functions are shared by various tests
 */

#ifndef KWIVER_TEST_TEST_FEATURES_H_
#define KWIVER_TEST_TEST_FEATURES_H_

#include <random>

#include <vital/types/feature.h>
#include <vital/types/feature_set.h>

using namespace kwiver::vital;

namespace kwiver {

namespace testing {

// Generate a set of generic features
// Function spreads feature values evenly over the number of features
// see <vital/types/feature.h> for parameter descriptions

template < typename T >
feature_set_sptr
make_n_features( size_t num_feat )
{
  std::vector< feature_sptr > feat;
  for( size_t i = 0; i < num_feat; ++i )
  {
    T v = static_cast< T >( i ) / num_feat;
    auto f = std::make_shared< feature_< T > >();
    T x = v * 5000, y = v * 5000 + 5;
    f->set_loc( Eigen::Matrix< T, 2, 1 >( x, y ) );
    f->set_scale( 1.0 + v );
    f->set_magnitude( 1 - v );
    f->set_angle( v * 3.14159f );
    f->set_color(
      rgb_color(
        static_cast< uint8_t >( i ),
        static_cast< uint8_t >( i + 5 ),
        static_cast< uint8_t >( i + 10 ) ) );
    f->set_covar( covariance_< 2, T >( v ) );
    feat.push_back( f );
  }

  return std::make_shared< simple_feature_set >( feat );
}

// Create a set of 10 features with known (unordered )
// scale and magnitude values for unit testing

template < typename T >
feature_set_sptr
make_10_features()
{
  size_t num_feat = 10;

  std::vector< double > scale = {
    1.0, 2.0, 1.8, 1.2, 1.1, 1.3, 1.7, 1.2, 1.1, 1.1 };

  std::vector< double > mag = {
    0.7, 0.1, 0.1, 0.2, 0.3, 0.5, 0.8, 0.5, 0.9, 0.1 };

  std::vector< feature_sptr > feat;
  for( size_t i = 0; i < num_feat; ++i )
  {
    T v = static_cast< T >( i ) / num_feat;
    auto f = std::make_shared< feature_< T > >();
    T x = v * 1000, y = v * 1000 + 5;
    f->set_loc( Eigen::Matrix< T, 2, 1 >( x, y ) );
    f->set_scale( scale[ i ] );
    f->set_magnitude( mag[ i ] );
    f->set_angle( v * 3.14159f );
    f->set_color(
      rgb_color(
        static_cast< uint8_t >( i ),
        static_cast< uint8_t >( i + 5 ),
        static_cast< uint8_t >( i + 10 ) ) );
    f->set_covar( covariance_< 2, T >( v ) );
    feat.push_back( f );
  }

  return std::make_shared< simple_feature_set >( feat );
}

// Create a set of 10 features with known (unordered )
// scale and magnitude values for unit testing
// set the locations for nonmax filtering

template < typename T >
feature_set_sptr
make_12_features()
{
  size_t num_feat = 12;

  std::vector< double > scale = {
    1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 4.0, 1.0  };

  std::vector< double > mag = {
    0.5, 1.0, 1.0, 0.5, 1.0, 0.2, 1.0, 0.2, 1.0, 1.0, 1.0, 1.0 };

  std::vector< size_t > coord = {
    100, 110, 300, 310, 320, 505, 510, 515, 700, 710, 720, 800 };

  std::vector< feature_sptr > feat;
  for( size_t i = 0; i < num_feat; ++i )
  {
    T v = static_cast< T >( i ) / num_feat;
    auto f = std::make_shared< feature_< T > >();
    f->set_loc( Eigen::Matrix< T, 2, 1 >( coord[ i ], coord[ i ] ) );
    f->set_scale( scale[ i ] );
    f->set_magnitude( mag[ i ] );
    f->set_angle( v * 3.14159f );
    f->set_color(
      rgb_color(
        static_cast< uint8_t >( i ),
        static_cast< uint8_t >( i + 5 ),
        static_cast< uint8_t >( i + 10 ) ) );
    f->set_covar( covariance_< 2, T >( v ) );
    feat.push_back( f );
  }

  return std::make_shared< simple_feature_set >( feat );
}

} // end namespace testing

} // end namespace kwiver

#endif
