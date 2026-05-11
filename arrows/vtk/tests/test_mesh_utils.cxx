// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test VTK arrow mesh conversion utility functions.

#include <arrows/vtk/mesh_utils.h>
#include <test_gtest.h>
#include <vtkXMLPolyDataReader.h>

namespace kv = kwiver::vital;
namespace kav = kwiver::arrows::vtk;

kv::path_t g_data_dir;

kv::path_t vtp_file = "/mesh_files/triangles_and_quad.vtp";
kv::path_t lines = "/mesh_files/lines.vtp";
kv::path_t lines_and_quad = "/mesh_files/lines_and_quad.vtp";
kv::path_t triangles = "/mesh_files/triangles.vtp";
kv::path_t triandles_and_quad = "/mesh_files/triangles_and_quad.vtp";
kv::path_t triangle_strip = "/mesh_files/triangle_strip.vtp";

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class vtk_mesh_utils : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
template < typename T >
vtkSmartPointer< vtkPolyData >
vtk_read_mesh( std::string const& file_name )
{
  vtkSmartPointer< T > reader = T::New();
  reader->SetFileName( file_name.c_str() );
  reader->Update();

  vtkSmartPointer< vtkPolyData > mesh = reader->GetOutput();
  return mesh;
}

// ----------------------------------------------------------------------------
TEST_F ( vtk_mesh_utils, vtk_to_vital )
{
  kv::path_t mesh_path = data_dir + vtp_file;
  auto input_mesh = vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  auto mesh = kav::vtk_to_vital( input_mesh );
  EXPECT_EQ( mesh.num_verts(), input_mesh->GetNumberOfPoints() );
  EXPECT_EQ( mesh.num_faces(), input_mesh->GetNumberOfPolys() );
}

TEST_F ( vtk_mesh_utils, vital_to_vtk )
{
  vtkSmartPointer< vtkPolyData > input_mesh, output_mesh;

  kv::path_t mesh_path = data_dir + vtp_file;
  input_mesh = vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );

  auto mesh = kav::vtk_to_vital( input_mesh );
  output_mesh = kav::vital_to_vtk( mesh );
  EXPECT_EQ( mesh.num_verts(), output_mesh->GetNumberOfPoints() );
  EXPECT_EQ( mesh.num_faces(), output_mesh->GetNumberOfPolys() );
}

TEST_F ( vtk_mesh_utils, no_polygons )
{
  kv::path_t mesh_path = data_dir + lines;
  auto input_mesh = vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  auto mesh = kav::vtk_to_vital( input_mesh );
  // Expect no polygons
  EXPECT_EQ( mesh.num_verts(), 10 );
  EXPECT_EQ( mesh.num_faces(), 0 );
}

TEST_F ( vtk_mesh_utils, polygon_and_lines )
{
  kv::path_t mesh_path = data_dir + lines_and_quad;
  auto input_mesh = vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  auto mesh = kav::vtk_to_vital( input_mesh );
  // Only expect the polygon to be converted to a face.
  EXPECT_EQ( mesh.num_verts(), 10 );
  EXPECT_EQ( mesh.num_faces(), 1 );
}

TEST_F ( vtk_mesh_utils, triangular_mesh )
{
  // Load regular (triangular) mesh
  kv::path_t mesh_path = data_dir + triangles;
  auto input_mesh = vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  auto mesh = kav::vtk_to_vital( input_mesh );
  EXPECT_EQ( mesh.num_verts(), 10 );
  EXPECT_EQ( mesh.num_faces(), 6 );
  // Expect the mesh face array to be triangular (i.e. regularity() == 3)
  EXPECT_EQ( mesh.faces().regularity(), 3 );

  // Load irregular mesh
  mesh_path = data_dir + triandles_and_quad;
  input_mesh = vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  mesh = kav::vtk_to_vital( input_mesh );
  EXPECT_EQ( mesh.num_verts(), 10 );
  EXPECT_EQ( mesh.num_faces(), 5 );
  // Expect the mesh face array to be irregular (i.e. regularity() == 0)
  EXPECT_EQ( mesh.faces().regularity(), 0 );
}

TEST_F ( vtk_mesh_utils, triangle_strip )
{
  kv::path_t mesh_path = data_dir + triangle_strip;
  auto input_mesh = vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  auto mesh = kav::vtk_to_vital( input_mesh );
  // Expect triangle strip cell to decompose into 8 triangle polygons
  EXPECT_EQ( mesh.num_verts(), 10 );
  EXPECT_EQ( mesh.num_faces(), 8 );
}
