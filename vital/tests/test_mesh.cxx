// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core mesh functionality

#include <kwiversys/SystemTools.hxx>
#include <fstream>
#include <tests/test_gtest.h>
#include <vital/exceptions.h>

#include <vital/io/mesh_io.h>

#include <gtest/gtest.h>

kwiver::vital::path_t g_data_dir;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  GET_ARG(1, g_data_dir);
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class mesh : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(mesh, group_names)
{
  kwiver::vital::mesh_sptr mesh_ply2 = kwiver::vital::read_mesh(
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply2");
  std::shared_ptr<kwiver::vital::mesh_face_array> faces =
    std::make_shared<kwiver::vital::mesh_face_array>(mesh_ply2->faces());

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
}

// ----------------------------------------------------------------------------
TEST_F(mesh, append)
{
  unsigned int first_size = 10;
  unsigned int second_size = 5;
  unsigned int third_size = 15;

  kwiver::vital::mesh_face_array first_faces( first_size );
  first_faces.make_group( "first name" );
  std::vector<kwiver::vital::vector_3d> first_normals;
  for(unsigned int i=0; i<first_size; i++){
    first_normals.push_back( kwiver::vital::vector_3d{ 1, 2, 3 } );
  }
  first_faces.set_normals( first_normals );

  kwiver::vital::mesh_face_array second_faces( second_size );
  second_faces.make_group( "second name" );
  std::vector<kwiver::vital::vector_3d> second_normals;
  for(unsigned int i=0; i<second_size; i++){
    second_normals.push_back( kwiver::vital::vector_3d{ 4, 5, 6 } );
  }
  second_faces.set_normals( second_normals );

  kwiver::vital::mesh_face_array third_faces( third_size );
  third_faces.make_group( "third name" );

  first_faces.append( second_faces );
  EXPECT_EQ( first_faces.size(), first_size + second_size );
  EXPECT_TRUE( first_faces.has_normals() );
  first_faces.append( third_faces );
  EXPECT_EQ( first_faces.size(), first_size + second_size + third_size );
  EXPECT_FALSE( first_faces.has_normals() );
}

// ----------------------------------------------------------------------------
TEST_F(mesh, append_with_shift)
{
  // unsigned int first_size = 10;
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
TEST_F(mesh, half_edges)
{
  std::vector<std::vector<unsigned int>> list = { { 9, 8, 7, 6, 5 },
                                                  { 4, 3, 2, 1, 0 } };
  unsigned int list_size = 0;
  for(unsigned int i=0; i<list.size(); i++)
  {
    for(unsigned int j=0; j<list[i].size(); j++)
    {
      list_size++;
    }
  }

  kwiver::vital::mesh_half_edge_set edges( list );
  EXPECT_EQ( edges.num_verts(), list_size );
  EXPECT_EQ( edges.num_faces(), list.size() );
}

// ----------------------------------------------------------------------------
TEST_F(mesh, copy_constructor)
{
  std::string original_path =
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.obj";
  std::string copy_path = "temp/aphill_240_1fps_crf32_sm_fused_mesh2.obj";

  kwiver::vital::mesh_sptr original =
    kwiver::vital::read_mesh( original_path );
  kwiver::vital::mesh copy( *original );


  EXPECT_TRUE( original->is_init() );
  EXPECT_TRUE( copy.is_init() );
  *original = kwiver::vital::mesh();
  EXPECT_FALSE( original->is_init() );
  EXPECT_TRUE( copy.is_init() );

  kwiver::vital::write_obj( copy_path, copy );
  std::ifstream original_stream( original_path.c_str() );
  std::ifstream copy_stream( copy_path.c_str() );

  EXPECT_EQ( original_stream.gcount(), copy_stream.gcount() );

  std::string original_str = "";
  std::string copy_str = "";
  while( original_stream >> original_str && copy_stream >> copy_str ){
    EXPECT_EQ( original_str, copy_str );
  }

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh, assignment_operator)
{
  std::string original_path =
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.obj";
  std::string copy_path = "temp/aphill_240_1fps_crf32_sm_fused_mesh2.obj";

  kwiver::vital::mesh_sptr original =
    kwiver::vital::read_mesh( original_path );
  kwiver::vital::mesh copy;

  EXPECT_TRUE( original->is_init() );
  EXPECT_FALSE( copy.is_init() );
  copy = *original;
  EXPECT_TRUE( original->is_init() );
  EXPECT_TRUE( copy.is_init() );
  *original = kwiver::vital::mesh();
  EXPECT_FALSE( original->is_init() );
  EXPECT_TRUE( copy.is_init() );

  kwiver::vital::write_obj( copy_path, copy );
  std::ifstream original_stream( original_path.c_str() );
  std::ifstream copy_stream( copy_path.c_str() );

  EXPECT_EQ( original_stream.gcount(), copy_stream.gcount() );

  std::string original_str = "";
  std::string copy_str = "";
  while( original_stream >> original_str && copy_stream >> copy_str ){
    EXPECT_EQ( original_str, copy_str );
  }

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh, build_edge_graph)
{
  kwiver::vital::mesh_sptr mesh_ply = kwiver::vital::read_ply(
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply");

  EXPECT_FALSE( mesh_ply->has_half_edges() );
  mesh_ply->build_edge_graph();
  EXPECT_TRUE( mesh_ply->has_half_edges() );
}

// ----------------------------------------------------------------------------
TEST_F(mesh, compute_vertex_normals)
{
  kwiver::vital::mesh_sptr mesh_ply2 = kwiver::vital::read_mesh(
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply2");

  EXPECT_FALSE( mesh_ply2->vertices().has_normals() );
  EXPECT_FALSE( mesh_ply2->has_half_edges() );
  mesh_ply2->compute_vertex_normals();
  EXPECT_TRUE( mesh_ply2->vertices().has_normals() );
  EXPECT_TRUE( mesh_ply2->has_half_edges() );
}

// ----------------------------------------------------------------------------
TEST_F(mesh, compute_vertex_normals_from_faces)
{
  kwiver::vital::mesh_sptr mesh_ply2 = kwiver::vital::read_mesh(
    data_dir + "/pipeline_data/aphill_240_1fps_crf32_sm_fused_mesh.ply2");

  EXPECT_FALSE( mesh_ply2->vertices().has_normals() );
  EXPECT_FALSE( mesh_ply2->faces().has_normals() );
  EXPECT_FALSE( mesh_ply2->has_half_edges() );
  mesh_ply2->compute_vertex_normals_from_faces();
  EXPECT_TRUE( mesh_ply2->vertices().has_normals() );
  EXPECT_TRUE( mesh_ply2->faces().has_normals() );
  EXPECT_TRUE( mesh_ply2->has_half_edges() );
}
