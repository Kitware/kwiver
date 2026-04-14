// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/core/algo/uv_unwrap_mesh.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/mesh.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <iomanip>
#include <iostream>

using namespace kwiver::vital;
using namespace kwiver::arrows::core;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

class uv_unwrap_mesh_test : public ::testing::Test
{
public:
  void
  SetUp()
  {
    // cube mesh of size 1.0
    std::vector< vector_3d > verts = {
      { -0.500000, -0.500000, -0.500000 },
      { -0.500000, -0.500000, 0.500000 },
      { -0.500000, 0.500000, -0.500000 },
      { -0.500000, 0.500000, 0.500000 },
      { 0.500000, -0.500000, -0.500000 },
      { 0.500000, -0.500000, 0.500000 },
      { 0.500000, 0.500000, -0.500000 },
      { 0.500000, 0.500000, 0.500000 } };
    std::vector< mesh_regular_face< 3 > > faces;
    faces.push_back( mesh_regular_face< 3 >( { 0, 1, 2 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 3, 2, 1 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 4, 6, 5 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 7, 5, 6 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 0, 4, 1 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 5, 1, 4 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 2, 3, 6 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 7, 6, 3 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 0, 2, 4 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 6, 4, 2 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 1, 5, 3 } ) );
    faces.push_back( mesh_regular_face< 3 >( { 7, 3, 5 } ) );

    std::unique_ptr< mesh_vertex_array_base > vertices_array_ptr(
      new mesh_vertex_array< 3 >( verts ) );
    std::unique_ptr< mesh_face_array_base > faces_array_ptr(
      new mesh_regular_face_array< 3 >( faces ) );
    mesh =
      std::make_shared< kwiver::vital::mesh >(
        std::move( vertices_array_ptr ),
        std::move( faces_array_ptr ) );
  }

  mesh_sptr mesh;
};

// ----------------------------------------------------------------------------
TEST ( uv_unwrap_mesh, create )
{
  using namespace kwiver::vital;

  plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, create_algorithm< algo::uv_unwrap_mesh >( "core" ) );
}

// ----------------------------------------------------------------------------
TEST_F ( uv_unwrap_mesh_test, check_texture_coordinates )
{
  uv_unwrap_mesh mesh_unwrap;
  config_block_sptr algo_config = mesh_unwrap.get_configuration();
  algo_config->set_value< double >( "spacing", 0.005 );
  mesh_unwrap.set_configuration( algo_config );

  mesh_unwrap.unwrap( mesh );

  // check that texture coordinates are between 0 and 1
  const std::vector< vector_2d >& tcoords = mesh->tex_coords();
  for( auto tc : tcoords )
  {
    EXPECT_GE( tc[ 0 ], 0.0 );
    EXPECT_GE( tc[ 1 ], 0.0 );
    EXPECT_LE( tc[ 0 ], 1.0 );
    EXPECT_LE( tc[ 1 ], 1.0 );
  }
}

// ----------------------------------------------------------------------------
// Run this test to print the exact UV coordinates produced by the current
// implementation (spacing=0.005, ascending sort, no compact packing).
// Copy the output into check_texture_coordinates_exact below.
TEST_F ( uv_unwrap_mesh_test, print_texture_coordinates )
{
  uv_unwrap_mesh mesh_unwrap;
  config_block_sptr algo_config = mesh_unwrap.get_configuration();
  algo_config->set_value< double >( "spacing", 0.005 );
  mesh_unwrap.set_configuration( algo_config );

  mesh_unwrap.unwrap( mesh );

  const std::vector< vector_2d >& tcoords = mesh->tex_coords();
  std::cout << std::fixed << std::setprecision( 15 );
  std::cout << "// " << tcoords.size() << " texture coordinates "
            << "(" << tcoords.size() / 3 << " faces x 3)\n";
  for( std::size_t i = 0; i < tcoords.size(); ++i )
  {
    std::cout << "  { " << tcoords[ i ][ 0 ]
              << ", " << tcoords[ i ][ 1 ] << " },  // tc[" << i << "]\n";
  }
}

// ----------------------------------------------------------------------------
// Regression test: exact UV coordinates for the unit cube, spacing=0.005,
// ascending sort, no compact packing.
// Values captured from print_texture_coordinates above.
// Update this table if the algorithm is intentionally changed.
TEST_F ( uv_unwrap_mesh_test, check_texture_coordinates_exact )
{
  uv_unwrap_mesh mesh_unwrap;
  config_block_sptr algo_config = mesh_unwrap.get_configuration();
  algo_config->set_value< double >( "spacing", 0.005 );
  mesh_unwrap.set_configuration( algo_config );

  mesh_unwrap.unwrap( mesh );

  const std::vector< vector_2d >& tcoords = mesh->tex_coords();

  // Expected values captured from print_texture_coordinates.
  // Reflects: spacing=0.005, ascending area sort, no compact packing.
  const std::vector< vector_2d > expected = {
    { 0.165906090208019, 0.165906090208019 },  // tc[0]
    { 0.004563458751885, 0.004563458751885 },  // tc[1]
    { 0.327248721664154, 0.004563458751885 },  // tc[2]
    { 0.493154811872173, 0.165906090208019 },  // tc[3]
    { 0.331812180416038, 0.004563458751885 },  // tc[4]
    { 0.654497443328308, 0.004563458751885 },  // tc[5]
    { 0.165906090208019, 0.331812180416038 },  // tc[6]
    { 0.004563458751885, 0.170469548959904 },  // tc[7]
    { 0.327248721664154, 0.170469548959904 },  // tc[8]
    { 0.493154811872173, 0.331812180416038 },  // tc[9]
    { 0.331812180416038, 0.170469548959904 },  // tc[10]
    { 0.654497443328308, 0.170469548959904 },  // tc[11]
    { 0.165906090208019, 0.497718270624058 },  // tc[12]
    { 0.004563458751885, 0.336375639167923 },  // tc[13]
    { 0.327248721664154, 0.336375639167923 },  // tc[14]
    { 0.493154811872173, 0.497718270624058 },  // tc[15]
    { 0.331812180416038, 0.336375639167923 },  // tc[16]
    { 0.654497443328308, 0.336375639167923 },  // tc[17]
    { 0.165906090208019, 0.663624360832077 },  // tc[18]
    { 0.004563458751885, 0.502281729375942 },  // tc[19]
    { 0.327248721664154, 0.502281729375942 },  // tc[20]
    { 0.493154811872173, 0.663624360832077 },  // tc[21]
    { 0.331812180416038, 0.502281729375942 },  // tc[22]
    { 0.654497443328308, 0.502281729375942 },  // tc[23]
    { 0.165906090208019, 0.829530451040096 },  // tc[24]
    { 0.004563458751885, 0.668187819583962 },  // tc[25]
    { 0.327248721664154, 0.668187819583962 },  // tc[26]
    { 0.493154811872173, 0.829530451040096 },  // tc[27]
    { 0.331812180416038, 0.668187819583962 },  // tc[28]
    { 0.654497443328308, 0.668187819583962 },  // tc[29]
    { 0.165906090208019, 0.995436541248116 },  // tc[30]
    { 0.004563458751885, 0.834093909791981 },  // tc[31]
    { 0.327248721664154, 0.834093909791981 },  // tc[32]
    { 0.493154811872173, 0.995436541248116 },  // tc[33]
    { 0.331812180416038, 0.834093909791981 },  // tc[34]
    { 0.654497443328308, 0.834093909791981 },  // tc[35]
  };

  if( expected.empty() )
  {
    std::cout << "[  SKIPPED ] Expected values not yet filled in; "
                 "run print_texture_coordinates first.\n";
    return;
  }

  ASSERT_EQ( expected.size(), tcoords.size() );

  const double tol = 1e-10;
  for( std::size_t i = 0; i < tcoords.size(); ++i )
  {
    EXPECT_NEAR( tcoords[ i ][ 0 ], expected[ i ][ 0 ], tol )
      << "tc[" << i << "][0] mismatch";
    EXPECT_NEAR( tcoords[ i ][ 1 ], expected[ i ][ 1 ], tol )
      << "tc[" << i << "][1] mismatch";
  }
}
