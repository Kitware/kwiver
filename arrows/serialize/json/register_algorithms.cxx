// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Defaults plugin algorithm registration interface impl
 */

#include <arrows/serialize/json/kwiver_serialize_json_plugin_export.h>
#include <vital/algo/algorithm_factory.h>

#include "activity.h"
#include "activity_type.h"
#include "bounding_box.h"
#include "detected_object.h"
#include "detected_object_set.h"
#include "detected_object_type.h"
#include "timestamp.h"
#include "image.h"
#include "string.h"
#include "track_state.h"
#include "object_track_state.h"
#include "track.h"
#include "track_set.h"
#include "object_track_set.h"

namespace kwiver {
namespace arrows {
namespace serialize {
namespace json {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_SERIALIZE_JSON_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  kwiver::vital::serializer_registrar reg( vpm, "arrows.serialize.json",
                                           "json" );

  if (reg.is_module_loaded())
  {
    return;
  }

  using namespace kwiver::arrows::serialize::json;

  reg.register_algorithm< activity >();
  reg.register_algorithm< activity_type >();
  reg.register_algorithm< bounding_box >();
  reg.register_algorithm< detected_object >();
  reg.register_algorithm< detected_object_set >();
  reg.register_algorithm< detected_object_type >();
  reg.register_algorithm< timestamp >();
  reg.register_algorithm< image >();
  reg.register_algorithm< image >( "kwiver:mask" );
  reg.register_algorithm< string >();
  reg.register_algorithm< track_state >();
  reg.register_algorithm< object_track_state >();
  reg.register_algorithm< track >();
  reg.register_algorithm< track_set >();
  reg.register_algorithm< object_track_set >();
  reg.register_algorithm< string >( "kwiver:file_name" );
  reg.register_algorithm< string >( "kwiver:image_name" );
  reg.register_algorithm< string >( "kwiver:video_name" );

  reg.mark_module_as_loaded();
}

} // end namespace json
} // end namespace serialize
} // end namespace arrows
} // end namespace kwiver
