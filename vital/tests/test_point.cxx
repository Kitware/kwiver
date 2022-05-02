// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core point class

#include <vital/types/point.h>

#include <gtest/gtest.h>

const std::string covariance_2D = " - covariance : [ 1, 0\n"
                                  "                  0, 1 ]\n";
const std::string expected_2D = "point2D\n"
                                " - value : [ 12.34, 23.45 ]\n" +
                                covariance_2D;
const std::string expected_3D = "point3D\n"
                                " - value : [ 12.34, 23.45, 34.56 ]\n"
                                " - covariance : [ 1, 0, 0\n"
                                "                  0, 1, 0\n"
                                "                  0, 0, 1 ]\n";
const std::string expected_4D = "point4D\n"
                                " - value : [ 12.34, 23.45, 34.56, 45.67 ]\n"
                                " - covariance : [ 1, 0, 0, 0\n"
                                "                  0, 1, 0, 0\n"
                                "                  0, 0, 1, 0\n"
                                "                  0, 0, 0, 1 ]\n";

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(point, print_2i)
{
  kwiver::vital::point_2i p2i( 1234, 2345 );
  std::stringstream output;
  output << p2i;

  std::string expected;
  expected = "point2D\n"
             " - value : [ 1234, 2345 ]\n" +
             covariance_2D;

  EXPECT_EQ( expected, output.str() );
}

// ----------------------------------------------------------------------------
TEST(point, print_2d)
{
  kwiver::vital::point_2d p2d( 12.34, 23.45 );
  std::stringstream output;
  output << p2d;

  EXPECT_EQ( expected_2D, output.str() );
}

// ----------------------------------------------------------------------------
TEST(point, print_2f)
{
  kwiver::vital::point_2f p2f( 12.34, 23.45 );
  std::stringstream output;
  output << p2f;

  EXPECT_EQ( expected_2D, output.str() );
}

// ----------------------------------------------------------------------------
TEST(point, print_3d)
{
  kwiver::vital::point_3d p3d( 12.34, 23.45, 34.56 );
  std::stringstream output;
  output << p3d;

  EXPECT_EQ( expected_3D, output.str() );
}

// ----------------------------------------------------------------------------
TEST(point, print_3f)
{
  kwiver::vital::point_3f p3f( 12.34, 23.45, 34.56 );
  std::stringstream output;
  output << p3f;

  EXPECT_EQ( expected_3D, output.str() );
}

// ----------------------------------------------------------------------------
TEST(point, print_4d)
{
  kwiver::vital::point_4d p4d( 12.34, 23.45, 34.56, 45.67 );
  std::stringstream output;
  output << p4d;

  EXPECT_EQ( expected_4D, output.str() );
}

// ----------------------------------------------------------------------------
TEST(point, print_4f)
{
  kwiver::vital::point_4f p4f( 12.34, 23.45, 34.56, 45.67 );
  std::stringstream output;
  output << p4f;

  EXPECT_EQ( expected_4D, output.str() );
}
