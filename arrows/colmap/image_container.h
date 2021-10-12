#ifndef KWIVER_ARROWS_COLMAP_COLMAP_H
#define KWIVER_ARROWS_COLMAP_COLMAP_H

#include <arrows/colmap/kwiver_algo_colmap_export.h>
#include <colmap/mvs/image.h>
#include <iostream>
#include <vital/types/image_container.h>

namespace kwiver {

namespace arrows {

namespace colmap_arrow {

class KWIVER_ALGO_COLMAP_EXPORT image_container : public vital::image_container
{
public:
  /// Converts a vital image to a colmap Bitmap type
  static colmap::Bitmap vital_to_bitmap( kwiver::vital::image img );

  /// Converts a vital image to a colmap Image type
  static colmap::mvs::Image vital_to_colmap( kwiver::vital::image& img );
};

} // namespace colmap

} // namespace arrows

} // namespace kwiver
#endif
