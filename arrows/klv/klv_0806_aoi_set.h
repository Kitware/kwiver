// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0806 AOI Set parser.

#include <arrows/klv/kwiver_algo_klv_export.h>

#include "klv_packet.h"
#include "klv_set.h"

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0806_aoi_set_tag : klv_lds_key
{
  KLV_0806_AOI_SET_UNKNOWN                  = 0,
  KLV_0806_AOI_SET_NUMBER                   = 1,
  KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_1  = 2,
  KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_1 = 3,
  KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_3  = 4,
  KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_3 = 5,
  KLV_0806_AOI_SET_TYPE                     = 6,
  KLV_0806_AOI_SET_TEXT                     = 7,
  KLV_0806_AOI_SET_SOURCE_ID                = 8,
  KLV_0806_AOI_SET_LABEL                    = 9,
  KLV_0806_AOI_SET_OPERATION_ID             = 10,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_aoi_set_tag tag );

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST0806 POI Set tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0806_aoi_set_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a KLV ST0806 area-of-interest local set.
class KWIVER_ALGO_KLV_EXPORT klv_0806_aoi_set_format
  : public klv_local_set_format
{
public:
  klv_0806_aoi_set_format();

  std::string
  description() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
