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

  EXPECT_EQ( original->num_edges(), copy->num_edges() );

  kwiver::vital::mesh_vertex_array<3>& original_vertices = dynamic_cast
    <kwiver::vital::mesh_vertex_array<3>&>( original->vertices() );
  kwiver::vital::mesh_vertex_array<3>& copy_vertices = dynamic_cast
    <kwiver::vital::mesh_vertex_array<3>&>( copy->vertices() );

  EXPECT_EQ( original_vertices.size(), copy_vertices.size() );
  for(unsigned int i=0; i<original_vertices.size(); i++)
  {
    EXPECT_EQ( original_vertices[i].size(), copy_vertices[i].size() );
    for(unsigned int j=0; j<original_vertices[i].size(); j++)
    {
      EXPECT_EQ( original_vertices[i][j], copy_vertices[i][j] );
    }
  }

  kwiver::vital::mesh_face_array original_faces = *std::make_shared
    <kwiver::vital::mesh_face_array>(original->faces());
  kwiver::vital::mesh_face_array copy_faces = *std::make_shared
    <kwiver::vital::mesh_face_array>(copy->faces());

  EXPECT_EQ( original_faces.size(), copy_faces.size() );
  for(unsigned int i=0; i<original_faces.size(); i++)
  {
    EXPECT_EQ( original_faces[i].size(), copy_faces[i].size() );
    for(unsigned int j=0; j<original_faces[i].size(); j++)
    {
      EXPECT_EQ( original_faces[i][j], copy_faces[i][j] );
    }
  }

  kwiversys::SystemTools::RemoveADirectory( "temp" );
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_ply)
{
  kwiver::vital::mesh_sptr ply_mesh = kwiver::vital::read_ply(
    data_dir + "/cube_mesh.ply");
  kwiver::vital::mesh_sptr cube_mesh = kwiver::testing::cube_mesh( 1.0 );

  EXPECT_EQ( ply_mesh->num_edges(), cube_mesh->num_edges() );

  kwiver::vital::mesh_vertex_array<3>& ply_mesh_vertices = dynamic_cast
    <kwiver::vital::mesh_vertex_array<3>&>( ply_mesh->vertices() );
  kwiver::vital::mesh_vertex_array<3>& cube_mesh_vertices = dynamic_cast
    <kwiver::vital::mesh_vertex_array<3>&>( cube_mesh->vertices() );

  EXPECT_EQ( ply_mesh_vertices.size(), cube_mesh_vertices.size() );
  for(unsigned int i=0; i<ply_mesh_vertices.size(); i++)
  {
    EXPECT_EQ( ply_mesh_vertices[i].size(), cube_mesh_vertices[i].size() );
    for(unsigned int j=0; j<ply_mesh_vertices[i].size(); j++)
    {
      EXPECT_EQ( ply_mesh_vertices[i][j], cube_mesh_vertices[i][j] );
    }
  }

  kwiver::vital::mesh_face_array ply_mesh_faces = *std::make_shared
    <kwiver::vital::mesh_face_array>(ply_mesh->faces());
  kwiver::vital::mesh_face_array cube_mesh_faces = *std::make_shared
    <kwiver::vital::mesh_face_array>(cube_mesh->faces());

  EXPECT_EQ( ply_mesh_faces.size(), cube_mesh_faces.size() );
  for(unsigned int i=0; i<ply_mesh_faces.size(); i++)
  {
    EXPECT_EQ( ply_mesh_faces[i].size(), cube_mesh_faces[i].size() );
    for(unsigned int j=0; j<ply_mesh_faces[i].size(); j++)
    {
      EXPECT_EQ( ply_mesh_faces[i][j], cube_mesh_faces[i][j] );
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F(mesh_io, read_write_obj)
{
  kwiver::vital::mesh_sptr original = kwiver::testing::cube_mesh( 1.0 );

  std::string path = "temp/cube_mesh.obj";
  kwiver::vital::write_obj( path, *original );
  kwiver::vital::mesh_sptr copy = kwiver::vital::read_obj( path );

  EXPECT_EQ( original->num_edges(), copy->num_edges() );

  kwiver::vital::mesh_vertex_array<3>& original_vertices = dynamic_cast
    <kwiver::vital::mesh_vertex_array<3>&>( original->vertices() );
  kwiver::vital::mesh_vertex_array<3>& copy_vertices = dynamic_cast
    <kwiver::vital::mesh_vertex_array<3>&>( copy->vertices() );

  EXPECT_EQ( original_vertices.size(), copy_vertices.size() );
  for(unsigned int i=0; i<original_vertices.size(); i++)
  {
    EXPECT_EQ( original_vertices[i].size(), copy_vertices[i].size() );
    for(unsigned int j=0; j<original_vertices[i].size(); j++)
    {
      EXPECT_EQ( original_vertices[i][j], copy_vertices[i][j] );
    }
  }

  kwiver::vital::mesh_face_array original_faces = *std::make_shared
    <kwiver::vital::mesh_face_array>(original->faces());
  kwiver::vital::mesh_face_array copy_faces = *std::make_shared
    <kwiver::vital::mesh_face_array>(copy->faces());

  EXPECT_EQ( original_faces.size(), copy_faces.size() );
  for(unsigned int i=0; i<original_faces.size(); i++)
  {
    EXPECT_EQ( original_faces[i].size(), copy_faces[i].size() );
    for(unsigned int j=0; j<original_faces[i].size(); j++)
    {
      EXPECT_EQ( original_faces[i][j], copy_faces[i][j] );
    }
  }

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
