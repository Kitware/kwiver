// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core mesh_io functionality

#include <fstream>

#include <kwiversys/SystemTools.hxx>
#include <tests/test_gtest.h>
#include <tests/test_scene.h>
#include <vital/exceptions.h>

#include <vital/io/mesh_io.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;

path_t g_data_dir;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  GET_ARG(1, g_data_dir);
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class mesh_io : public ::testing::Test
{
  TEST_ARG(data_dir);

  void compare_meshes(const mesh_sptr& first,
                      const mesh_sptr& second)
  {
    double threshhold = 0.000001;

    mesh_vertex_array<3>& first_vertices = dynamic_cast
      <mesh_vertex_array<3>&>( first->vertices() );
    mesh_vertex_array<3>& second_vertices = dynamic_cast
      <mesh_vertex_array<3>&>( second->vertices() );

    EXPECT_EQ( first_vertices.size(), second_vertices.size() );
    for(unsigned int i=0; i<first_vertices.size(); i++)
    {
      EXPECT_EQ( first_vertices[i].size(), second_vertices[i].size() );
      for(unsigned int j=0; j<first_vertices[i].size(); j++)
      {
        EXPECT_EQ( first_vertices[i][j], second_vertices[i][j] );
      }
    }

    mesh_face_array first_faces = *std::make_shared
      <mesh_face_array>(first->faces());
    mesh_face_array second_faces = *std::make_shared
      <mesh_face_array>(second->faces());

    EXPECT_EQ( first_faces.size(), second_faces.size() );
    for(unsigned int i=0; i<first_faces.size(); i++)
    {
      EXPECT_EQ( first_faces[i].size(), second_faces[i].size() );
      for(unsigned int j=0; j<first_faces[i].size(); j++)
      {
        EXPECT_EQ( first_faces[i][j], second_faces[i][j] );
      }
    }

    std::vector<vector_3d> first_normals = first_vertices.normals();
    std::vector<vector_3d> second_normals = second_vertices.normals();
    EXPECT_EQ( first_normals.size(), second_normals.size() );
    for(unsigned int i=0; i<first_normals.size(); i++)
    {
      EXPECT_NEAR( first_normals[i][0], second_normals[i][0], threshhold );
      EXPECT_NEAR( first_normals[i][1], second_normals[i][1], threshhold );
      EXPECT_NEAR( first_normals[i][2], second_normals[i][2], threshhold );
    }
  }
};

// ----------------------------------------------------------------------------
TEST_F(mesh_io, invalid_output_file)
{
  kwiversys::SystemTools::MakeDirectory( "temp" );

  mesh empty_mesh;
  EXPECT_THROW( write_ply2( "temp", empty_mesh ), file_write_exception );

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, invalid_input_file)
{
  EXPECT_THROW( read_ply2( data_dir + "/nonexistant" ),
    file_not_found_exception );
  EXPECT_THROW( read_ply2( data_dir + "/videos" ), file_not_found_exception );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_invalid_type)
{
  mesh_sptr empty_read_mesh = read_mesh(
    data_dir + "/aphill_pipeline_data/geo_origin.txt");

  EXPECT_EQ( empty_read_mesh, nullptr );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_write_ply2)
{
  mesh_sptr original = kwiver::testing::cube_mesh( 1.0 );

  std::string path = "temp/cube_mesh.ply2";
  write_ply2( path, *original );
  mesh_sptr copy = read_ply2( path );

  compare_meshes( original, copy );

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_ply)
{
  mesh_sptr ply_mesh = read_ply(
    data_dir + "/cube_mesh.ply");
  mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );

  compare_meshes( ply_mesh, cube_mesh );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_write_obj)
{
  mesh_sptr original = kwiver::testing::cube_mesh( 1.0 );
  original->compute_vertex_normals();

  std::string path = "temp/cube_mesh.obj";
  write_obj( path, *original );
  mesh_sptr copy = read_obj( path );

  compare_meshes( original, copy );

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, write_kml)
{
  mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );
  EXPECT_NO_THROW( write_kml( "temp/cube_mesh.kml", *cube_mesh ) );
  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, write_kml_collada_not_triangular)
{
  std::string output_file_name = "temp/cube_mesh.kml_collada";

  mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );
  EXPECT_NO_THROW( write_kml_collada( output_file_name, *cube_mesh ) );
  std::ifstream stream( output_file_name.c_str() );
  EXPECT_EQ( stream.peek(), std::ifstream::traits_type::eof() );

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, write_vrml_not_triangular)
{
  std::string output_file_name = "temp/cube_mesh.vrml";

  mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );
  EXPECT_NO_THROW( write_vrml( output_file_name, *cube_mesh ) );
  std::ifstream stream( output_file_name.c_str() );
  EXPECT_EQ( stream.peek(), std::ifstream::traits_type::eof() );

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}
