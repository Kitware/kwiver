// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface to the KLV 0806 POI Set parser.

#include <arrows/klv/kwiver_algo_klv_export.h>

#include "klv_packet.h"
#include "klv_set.h"

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0806_poi_set_tag : klv_lds_key
{
  KLV_0806_POI_SET_UNKNOWN      = 0,
  KLV_0806_POI_SET_NUMBER       = 1,
  KLV_0806_POI_SET_LATITUDE     = 2,
  KLV_0806_POI_SET_LONGITUDE    = 3,
  KLV_0806_POI_SET_ALTITUDE     = 4,
  KLV_0806_POI_SET_TYPE         = 5,
  KLV_0806_POI_SET_TEXT         = 6,
  KLV_0806_POI_SET_SOURCE_ICON  = 7,
  KLV_0806_POI_SET_SOURCE_ID    = 8,
  KLV_0806_POI_SET_LABEL        = 9,
  KLV_0806_POI_SET_OPERATION_ID = 10,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_poi_set_tag tag );

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST0806 POI Set tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0806_poi_set_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a KLV ST0806 point-of-interest local set.
class KWIVER_ALGO_KLV_EXPORT klv_0806_poi_set_format
  : public klv_local_set_format
{
public:
  klv_0806_poi_set_format();

  std::string
  description() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
