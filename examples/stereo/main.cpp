#include <iostream>
#include <set>
#include <arrows/mvg/triangulate.h>
#include <vital/io/camera_rig_io.h>

using std::cout;
using std::cerr;
using std::atof;

int main(int argc, char *argv[])
{
  std::set<int> valid_argc{6, 7, 10, 11};
  if (valid_argc.count( argc ) == 0)
  {
    cerr << "Usage: " << argv[0] << " camfile [xOffset] lX lY rX rY [lX2 LY2 rX2 rY2]\n"
         << "\n"
         << "Loads calibrates stereo camera file from camfile via stereo_rig_io;\n"
         << "computes world point from left / right image X,Y.\n"
         << "\n"
         << "With two points, also computes world distance between them.\n"
         << "\n"
         << "Optional xOffset (in pixels) is subtracted from all right X coordinates\n"
         << "(e.g. when picking coordinates from left/right pairs in a single image)\n";

    return -1;
  }

  // load the cameras

  auto stereo_rig_ptr = kwiver::vital::read_stereo_rig( argv[1] );
  kwiver::vital::simple_camera_perspective& left_cam(dynamic_cast<kwiver::vital::simple_camera_perspective&>(* (stereo_rig_ptr->left())));
  kwiver::vital::simple_camera_perspective& right_cam(dynamic_cast<kwiver::vital::simple_camera_perspective&>(* (stereo_rig_ptr->right())));
  cout << "left camera:\n" << left_cam << "\n";
  cout << "right camera:\n" << right_cam << "\n";

  // figure out if an offset was supplied, and how many points we're computing

  int right_x_offset = 0, pt1_offset = 2;
  if ((argc == 7) || (argc == 11))
  {
    right_x_offset = std::atoi( argv[2] );
    ++pt1_offset;
  }
  int pt2_offset =
    (argc == 10) || (argc == 11)
    ? pt1_offset + 4
    : -1;


  // always compute the first point

  Eigen::Matrix<float, 2, 1>
    lP1(atof(argv[pt1_offset]), atof(argv[pt1_offset+1])),
    rP1(atof(argv[pt1_offset+2]) - right_x_offset, atof(argv[pt1_offset+3]));
  auto outP1 = kwiver::arrows::mvg::triangulate_fast_two_view(left_cam, right_cam, lP1, rP1);
  cout << "Left point 1:\n" << lP1 << "\n"
       << "Right point 1:\n" << rP1 << "\n"
       << "Output point 1:\n" << outP1 << "\n";

  // compute second point and distance if given
  if (pt2_offset != -1)
  {
    Eigen::Matrix<float, 2, 1>
      lP2(atof(argv[pt2_offset]), atof(argv[pt2_offset+1])),
      rP2(atof(argv[pt2_offset+2]) - right_x_offset, atof(argv[pt2_offset+3]));
    auto outP2 = kwiver::arrows::mvg::triangulate_fast_two_view(left_cam, right_cam, lP2, rP2);
    cout << "\n"
         << "Left point 2:\n" << lP2 << "\n"
         << "Right point 2:\n" << rP2 << "\n"
         << "Output point 2:\n" << outP2 << "\n"
         << "\n"
         << "Distance:\n" << (outP2-outP1).norm() << "\n";
  }
}
