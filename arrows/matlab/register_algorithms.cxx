// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Matlab algorithm registration implementation

#include <arrows/matlab/kwiver_algo_matlab_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <vital/algo/detected_object_set_output.h>
#include <vital/algo/image_filter.h>
#include <vital/algo/image_object_detector.h>

#include <arrows/matlab/matlab_detection_output.h>
#include <arrows/matlab/matlab_image_filter.h>
#include <arrows/matlab/matlab_image_object_detector.h>

namespace kwiver {

namespace arrows {

namespace matlab {

extern "C"
KWIVER_ALGO_MATLAB_PLUGIN_EXPORT
void
register_factories( ::kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpl.add_factory< vital::algo::image_object_detector,
    matlab_image_object_detector >( "matlab" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.matlab" );

  fact = vpl.add_factory< vital::algo::image_filter,
    matlab_image_filter >( "matlab" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.matlab" );

  fact = vpl.add_factory< vital::algo::detected_object_set_output,
    matlab_detection_output >( "matlab" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.matlab" );
}

} // namespace matlab

} // namespace arrows

}     // end namespace
