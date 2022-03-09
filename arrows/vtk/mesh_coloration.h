// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VTK_MESH_COLORATION_H_
#define KWIVER_ARROWS_VTK_MESH_COLORATION_H_

#include <arrows/vtk/kwiver_algo_vtk_export.h>

#include <vital/algo/video_input.h>

#include <vital/config/config_block_types.h>

#include <vital/types/camera_map.h>
#include <vital/types/camera_perspective.h>

#include <vtkSmartPointer.h>

#include <string>
#include <vector>

class vtkFloatArray;
class vtkPolyData;
class vtkRenderWindow;

namespace kwiver {

namespace arrows {

namespace vtk {

/// Color a mesh from a video and cameras.
class KWIVER_ALGO_VTK_EXPORT mesh_coloration :
  public kwiver::vital::noncopyable
{
public:
  /// Construct object to color a mesh.
  ///
  /// Video[, mask] and cameras need to be set separately.
  mesh_coloration();

  /// Construct object to color a mesh.
  ///
  /// \param video_config Configuration for reading the video
  /// \param video_path Video file path
  /// \param mask_config Configuration for reading the mask
  /// \param mask_path Mask file path
  mesh_coloration( kwiver::vital::config_block_sptr const& video_config,
                   std::string const& video_path,
                   kwiver::vital::config_block_sptr const& mask_config,
                   std::string const& mask_path,
                   kwiver::vital::camera_map_sptr const& cameras );
  /// Set video input.
  void set_video( kwiver::vital::config_block_sptr const& video_config,
                  std::string const& video_path );
  /// Set mask to restrict area to be colored. Optional.
  void set_mask( kwiver::vital::config_block_sptr const& mask_config,
                 std::string const& mask_path );
  /// Set cameras (and frames) to be used for coloring.
  void set_cameras( kwiver::vital::camera_map_sptr const& cameras );
  /// Input mesh to be colored. This is not modified.
  void set_input( vtkSmartPointer< vtkPolyData > input );
  /// Input mesh to be colored.
  vtkSmartPointer< vtkPolyData > get_input();
  /// Output mesh.
  vtkSmartPointer< vtkPolyData > get_output();

  /// Set which frames to choose for coloring.
  ///
  /// A frame is chosen if \c frame mod \p sampling is zero.
  void set_frame_sampling( int sampling );

  /// Set color from frame
  void
  set_frame( int frame )
  {
    frame_ = frame;
  }

  /// Set whether to compute the average color (\c false) or to save colors for
  /// all frames.
  ///
  /// The frames selected are chosen using the sampling parameter.
  void
  set_all_frames( bool all_frames )
  {
    all_frames_ = all_frames;
  }

  /// Compare the depth buffer value with the depth of the mesh point.
  ///
  /// We use \p threshold >= 0 to fix floating point inaccuracies. Default
  /// value is 0, bigger values will remove more points.
  void
  set_occlusion_threshold( float threshold )
  {
    occlusion_threshold_ = threshold;
  }

  /// Set whether to remove occluded points.
  void
  set_remove_occluded( bool remove_occluded )
  {
    remove_occluded_ = remove_occluded;
  }

  /// Set whether to remove masked points.
  void
  set_remove_masked( bool remove_masked )
  {
    remove_masked_ = remove_masked;
  }

  /// Set whether to remove (\c false) points not colored
  void
  set_remove_not_colored( bool remove_not_colored )
  {
    remove_not_colored_ = remove_not_colored;
  }

  /// Color the mesh.
  ///
  /// Adds mean and median colors to \c output_ if \c all_frames is \c false,
  /// or adds an array of colors for each camera (frame) otherwise.
  ///
  /// \return \c true if successful, \c false if an error occurred.
  bool colorize();

  /// Reports progress when coloring the mesh
  virtual void report_progress_changed( std::string const& message,
                                        int percentage ) = 0;

protected:
  void initialize_data_list( int frame_id );
  void push_data( kwiver::vital::camera_map::map_camera_t::value_type cam_itr,
                  kwiver::vital::timestamp& ts, bool has_mask );
  vtkSmartPointer< vtkRenderWindow > create_depth_buffer_pipeline();
  vtkSmartPointer< vtkFloatArray > render_depth_buffer(
    vtkSmartPointer< vtkRenderWindow > ren_win,
    kwiver::vital::camera_perspective_sptr camera, int width,
    int height, double range[ 2 ] );

protected:
  vtkSmartPointer< vtkPolyData > input_;
  vtkSmartPointer< vtkPolyData > output_;
  int frame_sampling_;
  int frame_;
  bool all_frames_;
  float occlusion_threshold_;
  bool remove_occluded_;
  bool remove_masked_;
  bool remove_not_colored_;

  kwiver::vital::logger_handle_t logger_;

  struct coloration_data
  {
    coloration_data( kwiver::vital::image_container_sptr imageContainer,
                     kwiver::vital::image_container_sptr maskImageContainer,
                     kwiver::vital::camera_perspective_sptr camera,
                     kwiver::vital::frame_id_t frame )
      : image_{ imageContainer->get_image() },
        mask_image_{
          maskImageContainer ? maskImageContainer->get_image()
                             : kwiver::vital::image_of< uint8_t >{} },
        camera_{ camera }, frame_{ frame }
    {}
    kwiver::vital::image_of< uint8_t > image_;
    kwiver::vital::image_of< uint8_t > mask_image_;
    kwiver::vital::camera_perspective_sptr camera_;
    kwiver::vital::frame_id_t frame_;
  };

  std::vector< coloration_data > data_list_;

  std::string video_path_;
  kwiver::vital::algo::video_input_sptr video_reader_;
  std::string mask_path_;
  std::string mesh_output_path_;
  kwiver::vital::algo::video_input_sptr mask_reader_;
  kwiver::vital::camera_map_sptr cameras_;
};

} // namespace vtk

} // namespace arrows

} // namespace kwiver

#endif
