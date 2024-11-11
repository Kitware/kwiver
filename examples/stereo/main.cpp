#include <iostream>
#include <arrows/mvg/triangulate.h>
#include <vital/io/camera_rig_io.h>

int main(int argc, char *argv[])
{
  std::cout << "Hello world\n";

  Eigen::Matrix<float, 2, 1> pt;
  pt << 320,  240;
  std::cout << "Input point: " << pt << "\n";

  auto stereo_rig_ptr = kwiver::vital::read_stereo_rig( "/Users/roddy/work/doe-windfarm/t001-2024-09-bootup/data/Calib_Results_stereo_SH18_12_rectified.json" );
  std::cout << "left: " << stereo_rig_ptr->left() << "\n";
  std::cout << "right: " << stereo_rig_ptr->right() << "\n";

  kwiver::vital::simple_camera_perspective& left_cam(dynamic_cast<kwiver::vital::simple_camera_perspective&>(* (stereo_rig_ptr->left())));
  kwiver::vital::simple_camera_perspective& right_cam(dynamic_cast<kwiver::vital::simple_camera_perspective&>(* (stereo_rig_ptr->right())));
  std::cout << "left camera:\n" << left_cam << "\n";
  std::cout << "right camera:\n" << right_cam << "\n";
  auto out_pt = kwiver::arrows::mvg::triangulate_fast_two_view(left_cam, right_cam, pt, pt);
  std::cout << "Output point: " << out_pt << "\n";
}

