// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test VTK arrow mesh container.

#include <test_gtest.h>
#include <vtkXMLPolyDataReader.h>

#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>

namespace kv = kwiver::vital;
namespace kav = kwiver::arrows::vtk;

kv::path_t g_data_dir;

static std::string vtp_file = "/mesh_files/triangles.vtp";

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class vtk_mesh_container : public ::testing::Test
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
TEST_F ( vtk_mesh_container, create )
{
  kv::path_t mesh_path = data_dir + vtp_file;
  vtkSmartPointer< vtkPolyData > input_mesh =
    vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  kv::mesh mesh = kav::vtk_to_vital( input_mesh );

  kv::mesh_container_sptr cont =
    std::make_shared< kav::mesh_container >( mesh );
  EXPECT_EQ( mesh.num_verts(), cont->num_verts() );
  EXPECT_EQ( mesh.num_faces(), cont->num_faces() );

  kv::mesh cont_mesh = cont->mesh();
  EXPECT_EQ( mesh.num_verts(), cont_mesh.num_verts() );
  EXPECT_EQ( mesh.num_faces(), cont_mesh.num_faces() );
}

TEST_F ( vtk_mesh_container, copy )
{
  kv::path_t mesh_path = data_dir + vtp_file;
  vtkSmartPointer< vtkPolyData > input_mesh =
    vtk_read_mesh< vtkXMLPolyDataReader >( mesh_path );
  kv::mesh mesh = kav::vtk_to_vital( input_mesh );

  kv::mesh_container_sptr vtk_cont =
    std::make_shared< kav::mesh_container >( mesh );

  kv::mesh_container_sptr simple_cont =
    std::make_shared< kv::simple_mesh_container >( vtk_cont->mesh() );

  // Converting constructor from generic mesh_container
  kav::mesh_container_sptr cont_copy_1 =
    std::make_shared< kav::mesh_container >( *simple_cont );

  // Converting constructor casting to vtk::mesh_container
  kav::mesh_container_sptr cont_copy_2 =
    std::make_shared< kav::mesh_container >( *vtk_cont );

  EXPECT_EQ( simple_cont->num_verts(), vtk_cont->num_verts() );
  EXPECT_EQ( simple_cont->num_faces(), vtk_cont->num_faces() );
  EXPECT_EQ( cont_copy_1->num_verts(), vtk_cont->num_verts() );
  EXPECT_EQ( cont_copy_1->num_faces(), vtk_cont->num_faces() );
  EXPECT_EQ( cont_copy_2->num_verts(), vtk_cont->num_verts() );
  EXPECT_EQ( cont_copy_2->num_faces(), vtk_cont->num_faces() );
}
