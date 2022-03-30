// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test core mesh_io functionality

#include <kwiversys/SystemTools.hxx>
#include <tests/test_gtest.h>
#include <tests/test_scene.h>
#include <vital/exceptions.h>

#include <vital/io/mesh_io.cxx>

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
class mesh_io : public ::testing::Test
{
  TEST_ARG(data_dir);

  void compare_meshes(const kwiver::vital::mesh_sptr& first,
                      const kwiver::vital::mesh_sptr& second)
  {
    EXPECT_EQ( first->num_edges(), second->num_edges() );

    kwiver::vital::mesh_vertex_array<3>& first_vertices = dynamic_cast
      <kwiver::vital::mesh_vertex_array<3>&>( first->vertices() );
    kwiver::vital::mesh_vertex_array<3>& second_vertices = dynamic_cast
      <kwiver::vital::mesh_vertex_array<3>&>( second->vertices() );

    EXPECT_EQ( first_vertices.size(), second_vertices.size() );
    for(unsigned int i=0; i<first_vertices.size(); i++)
    {
      EXPECT_EQ( first_vertices[i].size(), second_vertices[i].size() );
      for(unsigned int j=0; j<first_vertices[i].size(); j++)
      {
        EXPECT_EQ( first_vertices[i][j], second_vertices[i][j] );
      }
    }

    kwiver::vital::mesh_face_array first_faces = *std::make_shared
      <kwiver::vital::mesh_face_array>(first->faces());
    kwiver::vital::mesh_face_array second_faces = *std::make_shared
      <kwiver::vital::mesh_face_array>(second->faces());

    EXPECT_EQ( first_faces.size(), second_faces.size() );
    for(unsigned int i=0; i<first_faces.size(); i++)
    {
      EXPECT_EQ( first_faces[i].size(), second_faces[i].size() );
      for(unsigned int j=0; j<first_faces[i].size(); j++)
      {
        EXPECT_EQ( first_faces[i][j], second_faces[i][j] );
      }
    }

  }
};

// ----------------------------------------------------------------------------
TEST_F(mesh_io, invalid_output_file)
{
  EXPECT_NO_THROW( kwiver::vital::check_output_file( "temp/name" ) );
  EXPECT_THROW( kwiver::vital::check_output_file( "temp" ),
    kwiver::vital::file_write_exception );
  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, invalid_input_file)
{
  EXPECT_THROW( kwiver::vital::check_input_file( data_dir + "/nonexistant" ),
    kwiver::vital::file_not_found_exception );
  EXPECT_THROW( kwiver::vital::check_input_file( data_dir + "/videos" ),
    kwiver::vital::file_not_found_exception );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_invalid_type)
{
  kwiver::vital::mesh_sptr empty_read_mesh = kwiver::vital::read_mesh(
    data_dir + "/pipeline_data/geo_origin.txt");

  EXPECT_EQ( empty_read_mesh, nullptr );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_write_ply2)
{
  kwiver::vital::mesh_sptr original = kwiver::testing::cube_mesh( 1.0 );

  std::string path = "temp/cube_mesh.ply2";
  kwiver::vital::write_ply2( path, *original );
  kwiver::vital::mesh_sptr copy = kwiver::vital::read_ply2( path );

  compare_meshes( original, copy );

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_ply)
{
  kwiver::vital::mesh_sptr ply_mesh = kwiver::vital::read_ply(
    data_dir + "/cube_mesh.ply");
  kwiver::vital::mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );

  compare_meshes( ply_mesh, cube_mesh );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_write_obj)
{
  kwiver::vital::mesh_sptr original = kwiver::testing::cube_mesh( 1.0 );

  std::string path = "temp/cube_mesh.obj";
  kwiver::vital::write_obj( path, *original );
  kwiver::vital::mesh_sptr copy = kwiver::vital::read_obj( path );

  compare_meshes( original, copy );

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, write_kml)
{
  kwiver::vital::mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );
  EXPECT_NO_THROW( kwiver::vital::write_kml(
    "temp/cube_mesh.kml", *cube_mesh ) );
  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, write_kml_collada)
{
  kwiver::vital::mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );
  EXPECT_NO_THROW( kwiver::vital::write_kml_collada(
    "temp/cube_mesh.kml_collada", *cube_mesh ) );
  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, write_vrml)
{
  kwiver::vital::mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );
  EXPECT_NO_THROW( kwiver::vital::write_vrml(
    "temp/cube_mesh.vrml", *cube_mesh ) );
  kwiversys::SystemTools::RemoveADirectory( "temp" );
}
