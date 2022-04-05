// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core landmark class

#include <vital/types/landmark.h>

#include <gtest/gtest.h>

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(landmark, base_print)
{
  // TODO include covariance once stream operators are defined
  // (just uncomment the last EXPECT_EQ)

  kwiver::vital::landmark_<double> original;
  kwiver::vital::landmark_<double> from_output;
  kwiver::vital::landmark* original_ptr = &original;

  // Non-default values for all landmark_ variables
  Eigen::Matrix< double, 3, 1 > loc( 1.2, 2.3, 3.4 );
  double scale = 4.5;
  Eigen::Matrix< double, 3, 1 > normal( 5.6, 6.7, 7.8 );
  kwiver::vital::rgb_color color( 16, 32, 64 );
  int observations = 321;
  double cos_obs_angle = 8.9;
  kwiver::vital::covariance_< 3, double > covar( 12.34 );

  original.set_loc( loc );
  original.set_scale( scale );
  original.set_normal( normal );
  original.set_color( color );
  original.set_observations( observations );
  original.set_cos_observation_angle( cos_obs_angle );
  original.set_covar( covar );

  std::stringstream output;
  output << *original_ptr;
  output >> from_output;

  EXPECT_EQ( original_ptr->loc(), from_output.get_loc() );
  EXPECT_EQ( original_ptr->scale(), from_output.get_scale() );
  EXPECT_EQ( original_ptr->normal(), from_output.get_normal() );
  EXPECT_EQ( original_ptr->color(), from_output.color() );
  EXPECT_EQ( original_ptr->observations(), from_output.observations() );
  EXPECT_EQ( original_ptr->cos_obs_angle(), from_output.get_cos_obs_angle() );
  // EXPECT_EQ( original_ptr->covar(), from_output.get_covar() );
}

// ----------------------------------------------------------------------------
TEST(landmark, template_print)
{
  // TODO include covariance once stream operators are defined
  // (just uncomment the last EXPECT_EQ)

  kwiver::vital::landmark_<double> original;
  kwiver::vital::landmark_<double> from_output;

  // Non-default values for all landmark_ variables
  Eigen::Matrix< double, 3, 1 > loc( 1.2, 2.3, 3.4 );
  double scale = 4.5;
  Eigen::Matrix< double, 3, 1 > normal( 5.6, 6.7, 7.8 );
  kwiver::vital::rgb_color color( 16, 32, 64 );
  int observations = 321;
  double cos_obs_angle = 8.9;
  kwiver::vital::covariance_< 3, double > covar( 12.34 );

  original.set_loc( loc );
  original.set_scale( scale );
  original.set_normal( normal );
  original.set_color( color );
  original.set_observations( observations );
  original.set_cos_observation_angle( cos_obs_angle );
  original.set_covar( covar );

  std::stringstream output;
  output << original;
  output >> from_output;

  EXPECT_EQ( original.get_loc(), from_output.get_loc() );
  EXPECT_EQ( original.get_scale(), from_output.get_scale() );
  EXPECT_EQ( original.get_normal(), from_output.get_normal() );
  EXPECT_EQ( original.color(), from_output.color() );
  EXPECT_EQ( original.observations(), from_output.observations() );
  EXPECT_EQ( original.get_cos_obs_angle(), from_output.get_cos_obs_angle() );
  // EXPECT_EQ( original.get_covar(), from_output.get_covar() );
}
