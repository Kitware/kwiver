// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_SERIALIZE_JSON_KLV_METADATA_MAP_IO_H_
#define KWIVER_ARROWS_SERIALIZE_JSON_KLV_METADATA_MAP_IO_H_

#include <arrows/serialize/json/klv/kwiver_serialize_json_klv_export.h>

#include <vital/algo/metadata_map_io.h>

namespace kwiver {

namespace arrows {

namespace serialize {

namespace json {

class KWIVER_SERIALIZE_JSON_KLV_EXPORT metadata_map_io_klv
  : public vital::algo::metadata_map_io
{
public:
  PLUGIN_INFO( "klv-json",
               "Perform IO on video KLV metadata using JSON." );

  metadata_map_io_klv();

  virtual ~metadata_map_io_klv();

  vital::metadata_map_sptr
  load_( std::istream& fin, std::string const& filename ) const override;

  void
  save_( std::ostream& fout, vital::metadata_map_sptr data,
         std::string const& filename ) const override;

private:
  class priv;

  std::unique_ptr< priv > d;
};

} // namespace json

} // namespace serialize

} // namespace arrows

} // namespace kwiver

#endif
