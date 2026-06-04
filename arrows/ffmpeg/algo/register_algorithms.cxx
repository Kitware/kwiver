// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Register FFMPEG algorithms implementation

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/ffmpeg/algo/ffmpeg_image_io.h>
#include <arrows/ffmpeg/algo/ffmpeg_video_input.h>
#include <arrows/ffmpeg/algo/ffmpeg_video_input_clip.h>
#include <arrows/ffmpeg/algo/ffmpeg_video_output.h>

namespace kwiver {

namespace arrows {

namespace ffmpeg {

extern "C"
KWIVER_ALGO_FFMPEG_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact =
    vpm.add_factory< vital::algo::video_input, ffmpeg_video_input >( "ffmpeg" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_ffmpeg" );

  fact =
    vpm.add_factory< vital::algo::video_input,
      ffmpeg_video_input_clip >( "ffmpeg_clip" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_ffmpeg" );

  /* XXX(ffmpeg-rewire)
   *  fact =
   *  vpm.add_factory< vital::algo::video_input,
   *   ffmpeg_video_input_rewire >( "ffmpeg_rewire" );
   *  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_ffmpeg" );
   */

  fact =
    vpm.add_factory< vital::algo::video_output,
      ffmpeg_video_output >( "ffmpeg" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_ffmpeg" );

  fact =
    vpm.add_factory< vital::algo::image_io,
      ffmpeg_image_io >( "ffmpeg" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_ffmpeg" );
}

} // end namespace ffmpeg

} // end namespace arrows

} // end namespace kwiver
