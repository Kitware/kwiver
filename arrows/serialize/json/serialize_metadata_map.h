// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface for serialize_metadata_map
/// \link kwiver::vital::algo::algorithm_def algorithm definition \endlink.

#ifndef VITAL_ARROWS_SERIALIZATION_JSON_SERIALIZE_METADATA_MAP_H_
#define VITAL_ARROWS_SERIALIZATION_JSON_SERIALIZE_METADATA_MAP_H_

#include <arrows/serialize/json/kwiver_serialize_json_export.h>

#include <vital/algo/serialize_metadata_map.h>

#include <vital/vital_config.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace serialize {

namespace json {

/// An implementation class for reading and writing metadata maps.
///
/// This class is a concrete implementation for reading and writing
/// video metadata.
class KWIVER_SERIALIZE_JSON_EXPORT serialize_metadata_map
  : public vital::algo::serialize_metadata_map
{
public:
  PLUGIN_INFO( "json",
               "Perform IO on video metadata using JSON." )

  serialize_metadata_map();

  ~serialize_metadata_map();

  /// Load in the data from a file.
  ///
  /// \throws file_not_read_exception
  ///   Thrown if the file can't be opened for reading, likely due to
  ///   permissions or not being present.
  ///
  /// \param filename The path to the file to load.
  /// \returns An image container referring to the loaded image.
  kwiver::vital::metadata_map_sptr load_(
    std::string const& filename ) const override;

  /// Save metadata to a file.
  ///
  /// \throws file_write_exception
  ///   Thrown if the file can't be opened, likely due to permissions or a
  ///   missing containing directory.
  ///
  /// \param filename The path to the file to save.
  /// \param data The metadata map for a video.
  void save_( std::string const& filename,
              kwiver::vital::metadata_map_sptr data ) const override;

private:
  class priv;

  std::unique_ptr< priv > d_;
};

} // namespace json

} // namespace serialize

} // namespace arrows

} // end namespace

#endif