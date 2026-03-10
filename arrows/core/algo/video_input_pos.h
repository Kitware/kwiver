// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef ARROWS_CORE_VIDEO_INPUT_POS_H
#define ARROWS_CORE_VIDEO_INPUT_POS_H

#include <vital/algo/video_input.h>

#include <arrows/core/kwiver_algo_core_export.h>

namespace kwiver {

namespace arrows {

namespace core {

/// Metadata reader using the AFRL POS file format.
// ----------------------------------------------------------------------------
/// This class implements a video input algorithm that returns only metadata.
///
/// The algorithm takes configuration for a directory full of images
/// and an associated directory name for the metadata files. These
/// metadata files have the same base name as the image files.
class KWIVER_ALGO_CORE_EXPORT video_input_pos
  : public vital::algo::video_input
{
public:
  PLUGGABLE_IMPL(
    video_input_pos,
    "Read video metadata in AFRL POS format."
    " The algorithm takes configuration for a directory full of images"
    " and an associated directory name for the metadata files."
    " These metadata files have the same base name as the image files."
    " Each metadata file is associated with the image file"
    " of the same base name.",
    PARAM_DEFAULT(
      metadata_directory, std::string,
      "Name of directory containing metadata files.", "" ),
    PARAM_DEFAULT(
      metadata_extension, std::string,
      "File extension of metadata files.",
      ".pos" ) );

  virtual ~video_input_pos();

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// @brief Open a list of images.
  ///
  /// This method opens the file that contains the list of images. The
  /// individual image names are used to find the associated metadata
  /// file in the directory supplied via the configuration.
  ///
  /// @param list_name Name of file that contains list of images.
  void open( std::string list_name ) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;
  bool seekable() const override;
  size_t num_frames() const override;

  bool next_frame(
    kwiver::vital::timestamp& ts,
    vital::time_usec_t timeout = 0 ) override;

  bool seek_frame(
    kwiver::vital::timestamp& ts,
    kwiver::vital::timestamp::frame_t frame_number,
    vital::time_usec_t timeout = 0 ) override;

  kwiver::vital::timestamp frame_timestamp() const override;
  kwiver::vital::image_container_sptr frame_image() override;
  kwiver::vital::metadata_vector frame_metadata() override;

protected:
  void initialize() override;

private:
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace core

} // namespace arrows

}     // end namespace

#endif // ARROWS_CORE_VIDEO_INPUT_POS_H
