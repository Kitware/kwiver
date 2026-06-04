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
  PLUGGABLE_IMPL(
    metadata_map_io_klv,
    "Perform IO on video KLV metadata using JSON.",
    PARAM_DEFAULT(
      compress,
      bool,
      "Set to true to read and write compressed JSON instead.",
      false )
  );

  virtual ~metadata_map_io_klv();

  vital::metadata_map_sptr
  load_( std::istream& fin, std::string const& filename ) const override;
  std::ios_base::openmode
  load_open_mode( std::string const& filename ) const override;

  void
  save_(
    std::ostream& fout, vital::metadata_map_sptr data,
    std::string const& filename ) const override;
  std::ios_base::openmode
  save_open_mode( std::string const& filename ) const override;

protected:
  void initialize() override;

private:
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace json

} // namespace serialize

} // namespace arrows

} // namespace kwiver

#endif
