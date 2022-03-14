// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "mesh_coloration.h"

#include "vtkKwiverCamera.h"

#include <kwiversys/SystemTools.hxx>

#include <vital/range/iota.h>

#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkIntArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPointDataToCellData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataNormals.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkSequencePass.h"
#include "vtkSmartPointer.h"
#include "vtkRemovePolyData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkVector.h"
#include "vtkWindowToImageFilter.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkXMLPolyDataWriter.h"

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace kvr = kwiver::vital::range;

namespace // anonymous
{

using kwiver::vital::algo::video_input;

static char const* const BLOCK_VR = "video_reader";
static char const* const BLOCK_MR = "mask_reader";

// ----------------------------------------------------------------------------
/// Compute median of a vector
template < typename T >
static void
ComputeMedian( std::vector< T > vector, double& median )
{
  std::sort( vector.begin(), vector.end() );

  size_t middleIndex = vector.size() / 2;
  if( vector.size() % 2 == 0 )
  {
    median = ( vector[ middleIndex ] + vector[ middleIndex - 1 ] ) / 2;
  }
  else
  {
    median = vector[ middleIndex ];
  }
}

} // namespace <anonymous>

namespace kwiver {

namespace arrows {

namespace vtk {

// ----------------------------------------------------------------------------
mesh_coloration
::mesh_coloration()
{
  input_ = nullptr;
  output_ = nullptr;
  sampling_ = 1;
  frame_ = -1;
  all_frames_ = false;
  occlusion_threshold_ = 0.0;
  remove_occluded_ = true;
  remove_masked_ = true;
  remove_not_colored_ = false;
  video_reader_ = nullptr;
  mask_reader_ = nullptr;
  cameras_ = nullptr;
  logger_ = kwiver::vital::get_logger( "arrows.vtk.mesh_coloration" );
}

// ----------------------------------------------------------------------------
mesh_coloration
::mesh_coloration(
  kwiver::vital::config_block_sptr const& video_config,
  std::string const& video_path,
  kwiver::vital::config_block_sptr const& mask_config,
  std::string const& mask_path,
  kwiver::vital::camera_map_sptr const& cameras )
  : mesh_coloration()
{
  set_video( video_config, video_path );
  set_mask( mask_config, mask_path );
  set_cameras( cameras );
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::set_video(
  kwiver::vital::config_block_sptr const& video_config,
  std::string const& video_path )
{
  video_path_ = video_path;
  video_input::set_nested_algo_configuration(
    BLOCK_VR, video_config, video_reader_ );
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::set_mask( kwiver::vital::config_block_sptr const& mask_config,
            std::string const& mask_path )
{
  mask_path_ = mask_path;

  auto const has_mask = !mask_path_.empty();
  if( has_mask && !video_input::check_nested_algo_configuration(
        BLOCK_MR, mask_config ) )
  {
    LOG_ERROR( logger_,
               "An error was found in the mask reader configuration." );
    return;
  }
  if( has_mask )
  {
    video_input::set_nested_algo_configuration(
      BLOCK_MR, mask_config, mask_reader_ );
  }
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::set_cameras( kwiver::vital::camera_map_sptr const& cameras )
{
  cameras_ = cameras;
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::set_input( vtkSmartPointer< vtkPolyData > mesh )
{
  input_ = mesh;
}

// ----------------------------------------------------------------------------
vtkSmartPointer< vtkPolyData >
mesh_coloration
::get_input()
{
  return input_;
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::set_output( vtkSmartPointer< vtkPolyData > mesh )
{
  output_ = mesh;
}

// ----------------------------------------------------------------------------
vtkSmartPointer< vtkPolyData >
mesh_coloration
::get_output()
{
  return output_;
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::set_frame_sampling( int sample )
{
  if( sample > 0 )
  {
    sampling_ = sample;
  }
}

// ----------------------------------------------------------------------------
bool
mesh_coloration
::colorize()
{
  LOG_INFO( logger_, "Initialize camera and image list: frame " << frame_ );
  initialize_data_list( frame_ );

  auto const numFrames = data_list_.size();

  if( input_ == 0 || numFrames == 0 )
  {
    if( input_ == 0 )
    {
      LOG_ERROR( logger_, "Error when input has been set" );
    }
    else
    {
      LOG_INFO( logger_, "No camera for this frame" );
    }
    LOG_INFO( logger_, "Done: frame " << frame_ );
    return false;
  }
  vtkDataArray* normals = input_->GetPointData()->GetNormals();

  if( !normals )
  {
    LOG_INFO( logger_, "Generating normals ..." );

    vtkNew< vtkPolyDataNormals > compute_normals;
    compute_normals->SetInputDataObject( input_ );
    compute_normals->Update();
    input_ = compute_normals->GetOutput();
    normals = input_->GetPointData()->GetNormals();
  }
  vtkPoints* meshPointList = input_->GetPoints();

  if( meshPointList == 0 )
  {
    LOG_ERROR( logger_, "invalid mesh points" );
    LOG_INFO( logger_, "Done: frame " << frame_ );
    return false;
  }
  vtkIdType nbMeshPoint = meshPointList->GetNumberOfPoints();

  // per frame colors
  std::vector< vtkSmartPointer< vtkUnsignedCharArray > > perFrameColor;
  // average colors
  vtkNew< vtkUnsignedCharArray > meanValues;
  vtkNew< vtkUnsignedCharArray > medianValues;
  vtkNew< vtkIntArray > countValues;
  // Store each rgb value for each depth map
  std::vector< double > list0;
  std::vector< double > list1;
  std::vector< double > list2;
  struct DepthBuffer
  {
    DepthBuffer()
    {
      Range[ 0 ] = Range[ 1 ] = 0;
    }

    vtkSmartPointer< vtkFloatArray > Buffer;
    double Range[ 2 ];
  };

  std::vector< DepthBuffer > depthBuffer( numFrames );

  if( remove_occluded_ )
  {
    report_progress_changed( "Creating depth buffers", 0 );

    auto ren_win = create_depth_buffer_pipeline();

    if( !ren_win )
    {
      LOG_ERROR( logger_, "Fail to create the render window" );
      return false;
    }

    size_t i = 0;
    for( auto it = depthBuffer.begin(); it != depthBuffer.end(); ++it )
    {
      kwiver::vital::camera_perspective_sptr camera = data_list_[ i ].camera_;
      auto const& colorImage = data_list_[ i ].image_;
      auto const width = static_cast< int >( colorImage.width() );
      auto const height = static_cast< int >( colorImage.height() );
      DepthBuffer db;
      db.Buffer =
        render_depth_buffer( ren_win, camera, width, height, db.Range );
      *it = db;
      ++i;
    }
  }
  if( !all_frames_ )
  {
    // Contains rgb values
    meanValues->SetNumberOfComponents( 3 );
    meanValues->SetNumberOfTuples( nbMeshPoint );
    meanValues->FillComponent( 0, 0 );
    meanValues->FillComponent( 1, 0 );
    meanValues->FillComponent( 2, 0 );
    meanValues->SetName( "mean" );

    medianValues->SetNumberOfComponents( 3 );
    medianValues->SetNumberOfTuples( nbMeshPoint );
    medianValues->FillComponent( 0, 0 );
    medianValues->FillComponent( 1, 0 );
    medianValues->FillComponent( 2, 0 );
    medianValues->SetName( "median" );

    countValues->SetNumberOfComponents( 1 );
    countValues->SetNumberOfTuples( nbMeshPoint );
    countValues->FillComponent( 0, 0 );
    countValues->SetName( "count" );
  }
  else
  {
    perFrameColor.resize( numFrames );

    vtkNew< vtkIntArray > cameraIndex;
    cameraIndex->SetNumberOfComponents( 1 );
    cameraIndex->SetNumberOfTuples( static_cast< int >( numFrames ) );
    cameraIndex->SetName( "camera_index" );
    output_->GetFieldData()->AddArray( cameraIndex );

    size_t i = 0;
    for( auto it = perFrameColor.begin(); it != perFrameColor.end(); ++it )
    {
      ( *it ) = vtkSmartPointer< vtkUnsignedCharArray >::New();
      ( *it )->SetNumberOfComponents( 4 ); // RGBA, we use A=0 for invalid
                                           // pixels and A=255 otherwise
      ( *it )->SetNumberOfTuples( nbMeshPoint );

      unsigned char* p = ( *it )->GetPointer( 0 );
      std::fill( p, p + nbMeshPoint * 4, 0 );

      std::ostringstream ostr;
      kwiver::vital::frame_id_t frame = data_list_[ i ].frame_;
      cameraIndex->SetValue( static_cast< vtkIdType >( i ),
                             static_cast< int >( frame ) );
      ostr << "frame_" << std::setfill( '0' ) << std::setw( 4 ) << frame;
      ( *it )->SetName( ostr.str().c_str() );
      output_->GetPointData()->AddArray( *it );
      ++i;
    }
  }
  auto const progress_step = nbMeshPoint / 100;
  vtkNew<vtkIdTypeArray> removedPoints;
  removedPoints->SetNumberOfTuples(nbMeshPoint);
  vtkIdType removedPointsIndex = 0;

  for( auto const pointId : kvr::iota( nbMeshPoint ) )
  {
    if( pointId % progress_step == 0 )
    {
      report_progress_changed(
        "Coloring Mesh Points",
        static_cast< int >( ( 100 * pointId ) / nbMeshPoint ) );
    }
    if( !all_frames_ )
    {
      list0.reserve( numFrames );
      list1.reserve( numFrames );
      list2.reserve( numFrames );
    }

    // Get mesh position from id
    kwiver::vital::vector_3d position;
    meshPointList->GetPoint( pointId, position.data() );

    kwiver::vital::vector_3d pointNormal;
    normals->GetTuple( pointId, pointNormal.data() );
    size_t coloredCount = 0;
    for( auto const frameId : kvr::iota( numFrames ) )
    {
      kwiver::vital::camera_perspective_sptr camera =
        data_list_[ frameId ].camera_;
      // Check if the 3D point is in front of the camera
      double depth = camera->depth( position );

      if( depth <= 0.0 )
      {
        continue;
      }

      // test that we are viewing the front side of the mesh
      kwiver::vital::vector_3d cameraPointVec = position - camera->center();

      if( cameraPointVec.dot( pointNormal ) > 0.0 )
      {
        continue;
      }

      // project 3D point to pixel coordinates
      auto pixelPosition = camera->project( position );
      auto const& colorImage = data_list_[ frameId ].image_;
      auto const width = colorImage.width();
      auto const height = colorImage.height();

      if( pixelPosition[ 0 ] < 0.0 ||
          pixelPosition[ 1 ] < 0.0 ||
          pixelPosition[ 0 ] >= static_cast< double >( width ) ||
          pixelPosition[ 1 ] >= static_cast< double >( height ) )
      {
        continue;
      }

      bool has_mask = true;
      auto const& maskImage = data_list_[ frameId ].mask_image_;
      auto const maskWidth = static_cast< double >( maskImage.width() );
      auto const maskHeight = static_cast< double >( maskImage.height() );

      if( pixelPosition[ 0 ] < 0.0 ||
          pixelPosition[ 1 ] < 0.0 ||
          pixelPosition[ 0 ] >= maskWidth ||
          pixelPosition[ 1 ] >= maskHeight )
      {
        has_mask = false;
      }

      try
      {
        auto const x = static_cast< size_t >( pixelPosition[ 0 ] );
        auto const y = static_cast< size_t >( pixelPosition[ 1 ] );
        kwiver::vital::rgb_color rgb = colorImage.at( x, y );
        bool showPoint = true;

        if( has_mask )
        {
          showPoint = ( maskImage.at( x, y ).r > 0 );
        }

        double depthBufferValue = 0;
        if( remove_occluded_ )
        {
          double* range = depthBuffer[ frameId ].Range;
          float depthBufferValueNorm =
            depthBuffer[ frameId ].Buffer->GetValue(
              static_cast< vtkIdType >( x + width * ( height - y - 1 ) ) );
          depthBufferValue =
            2 * range[1] * range[0] / (range[1] + range[0] - (2 * depthBufferValueNorm - 1) * (range[1] - range[0]));
        }
        if( ( !remove_occluded_ ||
              depthBufferValue + occlusion_threshold_ > depth ) &&
            ( !remove_masked_ || showPoint ) )
        {
          if( !all_frames_ )
          {
            list0.push_back( rgb.r );
            list1.push_back( rgb.g );
            list2.push_back( rgb.b );
          }
          else
          {
            unsigned char rgba[] = { rgb.r, rgb.g, rgb.b, 255 };
            perFrameColor[ frameId ]->SetTypedTuple( pointId, rgba );
          }
        }
      }
      catch ( std::out_of_range const& )
      {
        continue;
      }
      ++coloredCount;
    }
    if (coloredCount == 0)
    {
      removedPoints->SetValue(removedPointsIndex++, pointId);
    }

    if( !all_frames_ )
    {
      // If we get elements
      if( list0.size() != 0 )
      {
        double const sum0 = std::accumulate( list0.begin(), list0.end(), 0 );
        double const sum1 = std::accumulate( list1.begin(), list1.end(), 0 );
        double const sum2 = std::accumulate( list2.begin(), list2.end(), 0 );
        double const nb_val = static_cast< double >( list0.size() );
        meanValues->SetTuple3(
          pointId, sum0 / nb_val, sum1 / nb_val, sum2 / nb_val );

        double median0, median1, median2;
        ComputeMedian< double >( list0, median0 );
        ComputeMedian< double >( list1, median1 );
        ComputeMedian< double >( list2, median2 );
        medianValues->SetTuple3( pointId, median0, median1, median2 );
        countValues->SetTuple1( pointId, nb_val );
      }

      list0.clear();
      list1.clear();
      list2.clear();
    }
  }

  removedPoints->SetNumberOfTuples(removedPointsIndex);
  if( !all_frames_ )
  {
    output_->GetPointData()->AddArray( meanValues );
    output_->GetPointData()->AddArray( medianValues );
    output_->GetPointData()->AddArray( countValues );
  }

  if (remove_not_colored_ && removedPoints->GetNumberOfTuples() > 1)
  {
    // remove points and cells not colored
    vtkNew<vtkRemovePolyData> removeNotColored;
    removeNotColored->SetInputData(output_);
    removeNotColored->SetPointIds(removedPoints);
    removeNotColored->Update();
    output_ = vtkPolyData::SafeDownCast(removeNotColored->GetOutput());
  }

  report_progress_changed( "Done", 100 );
  return true;
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::push_data(
  kwiver::vital::camera_map::map_camera_t::value_type cam_itr,
  kwiver::vital::timestamp& ts, bool has_mask )
{
  auto cam_ptr =
    std::dynamic_pointer_cast< kwiver::vital::camera_perspective >(
      cam_itr.second );
  if( cam_ptr && video_reader_->seek_frame( ts, cam_itr.first ) &&
      ( !has_mask || mask_reader_->seek_frame( ts, cam_itr.first ) ) )
  {
    try
    {
      kwiver::vital::image_container_sptr image{
        video_reader_->frame_image() };
      if( has_mask )
      {
        kwiver::vital::image_container_sptr maskImage{
          mask_reader_->frame_image() };
        data_list_.push_back(
          coloration_data( image, maskImage, cam_ptr, cam_itr.first ) );
      }
      else
      {
        kwiver::vital::image_container_sptr maskImage;
        data_list_.push_back(
          coloration_data( image, maskImage, cam_ptr, cam_itr.first ) );
      }
    }
    catch ( kwiver::vital::image_type_mismatch_exception const& )
    {
    }
  }
}

// ----------------------------------------------------------------------------
void
mesh_coloration
::initialize_data_list( int frame_id )
{
  video_reader_->open( video_path_ );

  kwiver::vital::timestamp ts;
  auto cam_map = cameras_->cameras();
  bool has_mask = true;
  if( mask_path_.empty() )
  {
    has_mask = false;
  }
  else
  {
    try
    {
      mask_reader_->open( mask_path_ );
    }
    catch ( std::exception const& )
    {
      has_mask = false;
      LOG_ERROR( logger_, "Cannot open mask file: " << mask_path_ );
    }
  }
  // Take a subset of images
  if( frame_id < 0 )
  {
    int counter = 0;

    for( auto const& cam_itr : cam_map )
    {
      if( ( counter++ ) % sampling_ != 0 )
      {
        continue;
      }
      push_data( cam_itr, ts, has_mask );
    }
  }
  // Take the current image
  else
  {
    auto cam_itr = cam_map.find( frame_id );
    if( cam_itr != cam_map.end() )
    {
      push_data( *cam_itr, ts, has_mask );
    }
  }
  video_reader_->close();

  if( has_mask )
  {
    mask_reader_->close();
  }
}

// ----------------------------------------------------------------------------
vtkSmartPointer< vtkRenderWindow >
mesh_coloration
::create_depth_buffer_pipeline()
{
  vtkNew< vtkRenderer > ren;
  auto ren_win = vtkSmartPointer< vtkRenderWindow >::New();
  if( ren_win )
  {
    ren_win->OffScreenRenderingOn();
    ren_win->SetMultiSamples( 0 );
    ren_win->AddRenderer( ren );

    vtkNew< vtkPolyDataMapper > mapper;
    mapper->SetInputDataObject( input_ );

    vtkNew< vtkActor > actor;
    actor->SetMapper( mapper );
    ren->AddActor( actor );
  }
  return ren_win;
}

// ----------------------------------------------------------------------------
vtkSmartPointer< vtkFloatArray >
mesh_coloration
::render_depth_buffer(
  vtkSmartPointer< vtkRenderWindow > ren_win,
  kwiver::vital::camera_perspective_sptr camera_persp,
  int width, int height, double depthRange[ 2 ] )
{
  ren_win->SetSize( width, height );

  double* bounds = input_->GetBounds();
  double const bb[ 8 ][ 3 ] = {
    { bounds[ 0 ], bounds[ 2 ], bounds[ 4 ] },
    { bounds[ 1 ], bounds[ 2 ], bounds[ 4 ] },
    { bounds[ 0 ], bounds[ 3 ], bounds[ 4 ] },
    { bounds[ 1 ], bounds[ 3 ], bounds[ 4 ] },

    { bounds[ 0 ], bounds[ 2 ], bounds[ 5 ] },
    { bounds[ 1 ], bounds[ 2 ], bounds[ 5 ] },
    { bounds[ 0 ], bounds[ 3 ], bounds[ 5 ] },
    { bounds[ 1 ], bounds[ 3 ], bounds[ 5 ] },
  };
  depthRange[ 0 ] = std::numeric_limits< double >::max();
  depthRange[ 1 ] = std::numeric_limits< double >::lowest();

  for( int i : kvr::iota( 8 ) )
  {
    double depth =
      camera_persp->depth( kwiver::vital::vector_3d( bb[ i ][ 0 ],
                                                     bb[ i ][ 1 ],
                                                     bb[ i ][ 2 ] ) );
    if( depth < depthRange[ 0 ] )
    {
      depthRange[ 0 ] = depth;
    }
    if( depth > depthRange[ 1 ] )
    {
      depthRange[ 1 ] = depth;
    }
  }
  // we only render points in front of the camera
  double minDepth = depthRange[ 1 ] * 0.001;
  if( depthRange[ 0 ] <  minDepth)
  {
    depthRange[ 0 ] = minDepth;
  }
  vtkNew< vtkKwiverCamera > cam;
  int imageDimensions[ 2 ] = { width, height };
  cam->SetCamera( camera_persp );
  cam->SetImageDimensions( imageDimensions );
  cam->Update();
  cam->SetClippingRange( depthRange[ 0 ], depthRange[ 1 ] );

  vtkRenderer* ren = ren_win->GetRenderers()->GetFirstRenderer();
  vtkCamera* camera = ren->GetActiveCamera();
  camera->ShallowCopy( cam );
  ren_win->Render();

  vtkNew< vtkWindowToImageFilter > filter;
  filter->SetInput( ren_win );
  filter->SetScale( 1 );
  filter->SetInputBufferTypeToZBuffer();
  filter->Update();

  vtkSmartPointer< vtkFloatArray > zBuffer =
    vtkFloatArray::SafeDownCast(
      filter->GetOutput()->GetPointData()->GetArray( 0 ) );
  return zBuffer;
}

} // namespace vtk

} // namespace arrows

} // namespace kwiver
