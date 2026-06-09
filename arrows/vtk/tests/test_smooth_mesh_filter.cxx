// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test VTK mesh smoothing algo.

#include <test_gtest.h>
#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <vtkSphereSource.h>
#include <vtkTriangleFilter.h>

#include <arrows/vtk/algo/smooth_mesh_filter.h>
#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>

namespace kv = kwiver::vital;
namespace kav = kwiver::arrows::vtk;

// ----------------------------------------------------------------------------
static vtkSmartPointer< vtkPolyData >
make_sphere_mesh()
{
  vtkNew< vtkSphereSource > sphere;
  sphere->SetThetaResolution( 50 );
  sphere->SetPhiResolution( 50 );
  sphere->Update();

  vtkNew< vtkTriangleFilter > tri;
  tri->SetInputConnection( sphere->GetOutputPort() );
  tri->Update();

  return tri->GetOutput();
}

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( vtk_smooth_mesh_filter, create )
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kv::create_algorithm< kv::algo::mesh_filter >( "vtk_smooth" ) );
}

// ----------------------------------------------------------------------------
TEST ( vtk_smooth_mesh_filter, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    kav::smooth_mesh_filter,
    "Use vtkSmoothPolyDataFilter to apply Laplacian smoothing to a mesh.",
    PARAM_DEFAULT(
      convergence, double,
      "Specify a convergence criterion for the iteration process. Smaller "
      "numbers result in more smoothing iterations.",
      0.0 ),
    PARAM_DEFAULT(
      number_of_iterations, int,
      "Specify the number of iterations for Laplacian smoothing.",
      20 ),
    PARAM_DEFAULT(
      relaxation_factor, double,
      "Specify the relaxation factor for Laplacian smoothing. In general, "
      "small relaxation factors and large numbers of iterations are more "
      "stable than larger relaxation factors and smaller numbers of "
      "iterations.",
      0.01 ),
    PARAM_DEFAULT(
      feature_edge_smoothing, bool,
      "Turn on/off smoothing along sharp interior edges.",
      false ),
    PARAM_DEFAULT(
      feature_angle, double,
      "Specify the feature angle for sharp edge identification.",
      45.0 ),
    PARAM_DEFAULT(
      edge_angle, double,
      "Specify the edge angle to control smoothing along edges (either "
      "interior or boundary).",
      15.0 ),
    PARAM_DEFAULT(
      boundary_smoothing, bool,
      "Turn on/off the smoothing of vertices on the boundary of the mesh.",
      true ),
    PARAM_DEFAULT(
      generate_error_scalars, bool,
      "Turn on/off the generation of scalar distance values.",
      false ),
    PARAM_DEFAULT(
      generate_error_vectors, bool,
      "Turn on/off the generation of error vectors.",
      false ),
    PARAM_DEFAULT(
      output_points_precision, int,
      "Set the desired precision for the output points type.",
      2 )
  );
}

// ----------------------------------------------------------------------------
TEST ( vtk_smooth_mesh_filter, filter )
{
  kv::mesh_container_sptr cont =
    std::make_shared< kav::mesh_container >( make_sphere_mesh() );

  auto smoother = std::make_shared< kav::smooth_mesh_filter >();
  auto vtk_out = smoother->filter( cont );

  EXPECT_EQ( vtk_out->num_verts(), cont->num_verts() );
  EXPECT_EQ( cont->num_faces(), vtk_out->num_faces() );

  EXPECT_THROW( smoother->filter( nullptr ), kv::invalid_value );
}
