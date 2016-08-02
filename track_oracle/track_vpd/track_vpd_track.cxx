/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_vpd_track.h"

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::string;

namespace kwiver {
namespace track_oracle {

string
track_vpd_track_type
::object_type_to_str( unsigned t )
{
  switch (t)
  {
  case 1: return "Person";
  case 2: return "Car";
  case 3: return "Vehicle";
  case 4: return "GenericObject";
  case 5: return "Bicycle";
  }
  return "";
}

unsigned
track_vpd_track_type
::str_to_object_type( const string& s )
{
  if (s == "Person") return 1;
  if (s == "Car") return 2;
  if (s == "Vehicle") return 3;
  if (s == "GenericObject") return 4;
  if (s == "Bicycle") return 5;
  return 0;
}

} // ...track_oracle
} // ...kwiver
