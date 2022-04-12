// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core mesh functionality

#include <tests/test_gtest.h>
#include <tests/test_scene.h>
#include <vital/exceptions.h>

#include <vital/io/mesh_io.h>

#include <gtest/gtest.h>

kwiver::vital::path_t g_data_dir;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(mesh, group_names)
{
  kwiver::vital::mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );
  std::shared_ptr<kwiver::vital::mesh_face_array> faces =
    std::make_shared<kwiver::vital::mesh_face_array>(cube_mesh->faces());

  for(unsigned int i=0; i<faces->size(); i++){
    EXPECT_EQ( faces->group_name(i), "" );
  }

  EXPECT_EQ( faces->make_group( "testing name" ), faces->size() );
  EXPECT_EQ( faces->make_group( "second testing name" ), 0 );

  for(unsigned int i=0; i<faces->size(); i++){
    EXPECT_EQ( faces->group_name(i), "testing name" );
  }

  EXPECT_EQ( faces->group_name( faces->size()+1 ), "" );

  std::set<unsigned int> faces_set = faces->group_face_set( "testing name" );
  for(unsigned int i=0; i<faces->size(); i++){
    EXPECT_NE( faces_set.find( i ), faces_set.end() );
  }

  faces->push_back( std::vector<unsigned int>{1, 2, 3, 4} );
  faces->push_back( std::vector<unsigned int>{2, 3, 4, 5} );
  faces->push_back( std::vector<unsigned int>{3, 4, 5, 6} );
  faces->push_back( std::vector<unsigned int>{4, 5, 6, 7} );

  EXPECT_EQ( faces->make_group( "third testing name" ), 4 );
  EXPECT_EQ( faces->group_face_set( "third testing name" ).size(), 4 );

  faces->push_back( std::vector<unsigned int>{5, 6, 6, 7} );
  faces->push_back( std::vector<unsigned int>{6, 6, 7, 8} );

  EXPECT_EQ( faces->make_group( "third testing name" ), 2 );
  EXPECT_EQ( faces->group_face_set( "third testing name" ).size(), 6 );
  EXPECT_EQ( faces->group_face_set( "testing name" ).size(), faces->size()-6 );
}

// ----------------------------------------------------------------------------
TEST(mesh, append)
{
  kwiver::vital::mesh_sptr first_mesh = kwiver::testing::cube_mesh( 1.0 );
  first_mesh->compute_face_normals();
  std::shared_ptr<kwiver::vital::mesh_face_array> first_faces =
    std::make_shared<kwiver::vital::mesh_face_array>(first_mesh->faces());
  unsigned int first_size = first_faces->size();
  EXPECT_EQ( first_faces->make_group( "first name" ), first_size );

  kwiver::vital::mesh_sptr second_mesh = kwiver::testing::grid_mesh( 2, 3 );
  second_mesh->compute_face_normals();
  std::shared_ptr<kwiver::vital::mesh_face_array> second_faces =
    std::make_shared<kwiver::vital::mesh_face_array>(second_mesh->faces());
  unsigned int second_size = second_faces->size();
  EXPECT_EQ( second_faces->make_group( "second name" ), second_size );

  kwiver::vital::mesh_sptr third_mesh = kwiver::testing::cube_mesh( 1.0 );
  std::shared_ptr<kwiver::vital::mesh_face_array> third_faces =
    std::make_shared<kwiver::vital::mesh_face_array>(third_mesh->faces());
  unsigned int third_size = third_faces->size();

  first_faces->append( *second_faces );
  EXPECT_EQ( first_faces->make_group( "second name" ), 0 );
  EXPECT_EQ( first_faces->size(), first_size + second_size );
  EXPECT_EQ( first_faces->group_face_set( "second name" ).size(), second_size );
  EXPECT_TRUE( first_faces->has_normals() );
  first_faces->append( *third_faces );
  EXPECT_EQ( first_faces->make_group( "third name" ), third_size );
  EXPECT_EQ( first_faces->group_face_set( "third name" ).size(), third_size );
  EXPECT_EQ( first_faces->size(), first_size + second_size + third_size );
  EXPECT_FALSE( first_faces->has_normals() );
}

// ----------------------------------------------------------------------------
TEST(mesh, append_with_shift)
{
  std::vector<std::vector<unsigned int>> first_list = { { 0, 1, 2 } };
  std::vector<std::vector<unsigned int>> second_list = { { 0, 1, 2, 3, 4 },
                                                         { 5, 6, 7, 8, 9 } };
  unsigned int shift = 10;

  kwiver::vital::mesh_face_array first_faces( first_list );
  kwiver::vital::mesh_face_array second_faces( second_list );

  first_faces.append( second_faces, shift );

  for(unsigned int i=0; i<first_list.size(); i++)
  {
    for(unsigned int j=0; j<first_list[i].size(); j++)
    {
      EXPECT_EQ( first_list[i][j], first_faces[i][j] );
    }
  }

  unsigned int offset = first_list.size();
  for(unsigned int i=0; i<second_list.size(); i++)
  {
    for(unsigned int j=0; j<second_list[i].size(); j++)
    {
      EXPECT_EQ( second_list[i][j] + shift, first_faces[i + offset][j] );
    }
  }
}

// ----------------------------------------------------------------------------
TEST(mesh, half_edges)
{
  kwiver::vital::mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );

  std::vector<std::vector<unsigned int>> face_list = { {0, 1, 3, 2},
                                                       {4, 6, 7, 5},
                                                       {5, 7, 3, 1},
                                                       {6, 4, 0, 2},
                                                       {7, 6, 2, 3},
                                                       {1, 0, 4, 5} };
  unsigned int list_size = 0;
  for(unsigned int i=0; i<face_list.size(); i++)
  {
    list_size+=face_list[i].size();
  }

  kwiver::vital::mesh_half_edge_set constructed_edges( face_list );
  EXPECT_FALSE( cube_mesh->has_half_edges() );
  cube_mesh->build_edge_graph();
  EXPECT_TRUE( cube_mesh->has_half_edges() );
  kwiver::vital::mesh_half_edge_set copy_edges = cube_mesh->half_edges();

  EXPECT_EQ( constructed_edges.num_verts(), copy_edges.num_verts() );
  EXPECT_EQ( constructed_edges.num_faces(), copy_edges.num_faces() );

  EXPECT_EQ( copy_edges.size(), list_size );
  EXPECT_EQ( cube_mesh->num_faces(), face_list.size() );
}

// ----------------------------------------------------------------------------
TEST(mesh, copy_constructor)
{
  kwiver::vital::mesh_sptr original = kwiver::testing::cube_mesh( 1.0 );
  kwiver::vital::mesh_sptr copy =
    std::make_shared<kwiver::vital::mesh>( *original );

  EXPECT_EQ( *original, *copy );

  EXPECT_TRUE( original->is_init() );
  EXPECT_TRUE( copy->is_init() );
  *original = kwiver::vital::mesh();
  EXPECT_FALSE( original->is_init() );
  EXPECT_TRUE( copy->is_init() );
}

// ----------------------------------------------------------------------------
TEST(mesh, assignment_operator)
{
  kwiver::vital::mesh_sptr original = kwiver::testing::cube_mesh( 1.0 );
  kwiver::vital::mesh_sptr copy =
    std::make_shared<kwiver::vital::mesh>();
  *copy = kwiver::vital::mesh( *original );

  EXPECT_EQ( *original, *copy );

  EXPECT_TRUE( original->is_init() );
  EXPECT_TRUE( copy->is_init() );
  *original = kwiver::vital::mesh();
  EXPECT_FALSE( original->is_init() );
  EXPECT_TRUE( copy->is_init() );
}

// ----------------------------------------------------------------------------
TEST(mesh, compute_vertex_normals)
{
  kwiver::vital::mesh_sptr grid_mesh = kwiver::testing::grid_mesh( 2, 3 );

  EXPECT_FALSE( grid_mesh->vertices().has_normals() );
  EXPECT_FALSE( grid_mesh->has_half_edges() );
  grid_mesh->compute_vertex_normals();
  EXPECT_TRUE( grid_mesh->vertices().has_normals() );
  EXPECT_TRUE( grid_mesh->has_half_edges() );
}

// ----------------------------------------------------------------------------
TEST(mesh, compute_vertex_normals_from_faces)
{
  kwiver::vital::mesh_sptr grid_mesh = kwiver::testing::grid_mesh( 2, 3 );

  EXPECT_FALSE( grid_mesh->vertices().has_normals() );
  EXPECT_FALSE( grid_mesh->faces().has_normals() );
  EXPECT_FALSE( grid_mesh->has_half_edges() );
  grid_mesh->compute_vertex_normals_from_faces();
  EXPECT_TRUE( grid_mesh->vertices().has_normals() );
  EXPECT_TRUE( grid_mesh->faces().has_normals() );
  EXPECT_TRUE( grid_mesh->has_half_edges() );
}
