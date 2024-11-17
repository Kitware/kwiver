#include <iostream>
#include <arrows/mvg/triangulate.h>
#include <vital/io/camera_rig_io.h>

using std::cout;
using std::cerr;
using std::atof;

int main(int argc, char *argv[])
{
  if (argc != 6)
  {
    cerr << "Usage: " << argv[0] << " camfile leftX leftY rightX rightY\n";
    return -1;
  }

  auto stereo_rig_ptr = kwiver::vital::read_stereo_rig( argv[1] );
  cout << "left: " << stereo_rig_ptr->left() << "\n";
  cout << "right: " << stereo_rig_ptr->right() << "\n";

  kwiver::vital::simple_camera_perspective& left_cam(dynamic_cast<kwiver::vital::simple_camera_perspective&>(* (stereo_rig_ptr->left())));
  kwiver::vital::simple_camera_perspective& right_cam(dynamic_cast<kwiver::vital::simple_camera_perspective&>(* (stereo_rig_ptr->right())));
  cout << "left camera:\n" << left_cam << "\n";
  cout << "right camera:\n" << right_cam << "\n";

  Eigen::Matrix<float, 2, 1> leftPt(atof(argv[2]), atof(argv[3])), rightPt(atof(argv[4]), atof(argv[5]));

  auto out_pt = kwiver::arrows::mvg::triangulate_fast_two_view(left_cam, right_cam, leftPt, rightPt);
  cout << "Left point: " << leftPt << "\n";
  cout << "Right point: " << rightPt << "\n";
  cout << "Output point:\n" << out_pt << "\n";
}

