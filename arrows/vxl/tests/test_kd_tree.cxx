// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief test getting nearest neighbor with kd_tree
 */

#include <gtest/gtest.h>

#include <arrows/vxl/kd_tree.h>

#include <vital/algo/algorithm_factory.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/vital_config.h>

using namespace kwiver::vital;
using namespace kwiver::arrows::vxl;

// ----------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class kd_tree_test : public ::testing::Test
{
public:

  void
  SetUp()
  {
    points =
    { { -0.01717344,  0.63277306,  1.16735385 },
      { -0.12789247, -0.67814285, -0.53719132 },
      { 0.70140656,  1.8946183,  0.02268335 },
      { 1.17411077,  1.42231096,  0.6825078 },
      { -0.51654012,  0.024495, -0.21052011 },
      { 0.14719433, -0.06326257,  1.30487225 },
      { 0.38861575,  1.35210946,  0.90146365 },
      { 0.32465994, -0.94838736, -0.21327035 },
      { 0.17283864,  1.8833175, -0.24280185 },
      { -0.53618828,  1.14438589, -0.754441  },
      { -0.08824698, -0.68632001, -0.05610394 },
      { 1.39858515, -0.38175853,  0.11024733 },
      { -0.57303382,  0.8646172,  1.09681107 },
      { 1.37170567, -1.62716976,  0.13160887 },
      { 1.73804298,  0.64188309,  0.21032014 } };

    test_points =
    { { -0.33473656,  1.52684247, -0.86753264 },
      { -0.55981981, -1.09909536,  0.59573499 },
      { 0.46238199,  0.46761188,  0.07077688 },
      { 1.55802851, -0.65494246,  2.50201591 } };

    expected_nearest_indices =
    { { 9, 8, 2 }, { 10, 7, 1 }, { 4, 0, 6 }, { 5, 11, 0 } };

    expected_nearest_distances =
    { { 0.44681713, 0.88033834, 1.41468588 },
      { 0.90424467, 1.20810056, 1.28346573 },
      { 1.11075157, 1.20819398, 1.21565536 },
      { 1.94259906, 2.41259374, 2.43326843 } };

    expected_radius_indices =
    { { 9, 8 }, { 7, 10, 1 }, { 10, 11, 4, 0, 6, 14 }, {} };
  }

  // Set of points to create search tree for
  std::vector< point_3d > points;
  // Test points
  std::vector< point_3d > test_points;
  // Expected indices of closest points
  std::vector< std::vector< int > > expected_nearest_indices;
  // Expected distances to closest points
  std::vector< std::vector< double > > expected_nearest_distances;
  // Search radius
  double radius = 1.3;
  // Expected indices of point within radius
  std::vector< std::vector< int > > expected_radius_indices;
};

// ----------------------------------------------------------------------------
TEST ( kd_tree, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, algo::nearest_neighbors::create( "vxl_kd_tree" ) );
}

// ----------------------------------------------------------------------------
TEST_F ( kd_tree_test, find_nearest )
{
  kwiver::arrows::vxl::kd_tree search_tree;

  search_tree.build( points );

  int K = 3;

  std::vector< int > nearest_single_indices;
  std::vector< double > nearest_single_distances;

  search_tree.find_nearest_point( test_points[ 0 ], K, nearest_single_indices,
                                  nearest_single_distances );

  EXPECT_EQ( nearest_single_indices, expected_nearest_indices[ 0 ] );
  for( size_t j = 0; j < K; ++j )
  {
    EXPECT_NEAR( nearest_single_distances[ j ],
                 expected_nearest_distances[ 0 ][ j ], 1e-5 );
  }

  std::vector< std::vector< int > > nearest_indices;
  std::vector< std::vector< double > > nearest_distances;

  search_tree.find_nearest_points( test_points, K, nearest_indices,
                                   nearest_distances );

  EXPECT_EQ( nearest_indices.size(), expected_nearest_indices.size() );
  EXPECT_EQ( nearest_distances.size(), expected_nearest_distances.size() );

  for( size_t i = 0; i < nearest_indices.size(); ++i )
  {
    EXPECT_EQ( nearest_indices[ i ], expected_nearest_indices[ i ] );
    for( size_t j = 0; j < K; ++j )
    {
      EXPECT_NEAR( nearest_distances[ i ][ j ],
                   expected_nearest_distances[ i ][ j ], 1e-5 );
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( kd_tree_test, find_within_radius )
{
  kwiver::arrows::vxl::kd_tree search_tree;

  search_tree.build( points );

  std::vector< int > nearest_indices;

  for( size_t i = 0; i < test_points.size(); ++i )
  {
    search_tree.find_within_radius( test_points[ i ], radius,
                                    nearest_indices );
    EXPECT_EQ( nearest_indices, expected_radius_indices[ i ] );
  }
}
