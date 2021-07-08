// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef ARROWS_CORE_VIDEO_INPUT_ARF_H
#define ARROWS_CORE_VIDEO_INPUT_ARF_H

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/video_input.h>

namespace kwiver {
namespace arrows {
namespace core {

// ----------------------------------------------------------------
/// Video reader for the NVESD ARF file format.
// (Automatic Target Recognition Working Group Raster Format)
class KWIVER_ALGO_CORE_EXPORT video_input_arf
  : public vital::algo::video_input
{
public:
  PLUGIN_INFO( "arf",
               "Read video metadata in NVESD ARF format. "
               "This algorithm takes an arf video file" )

  /// Constructor
  video_input_arf();
  virtual ~video_input_arf();

  /// Get this algorithm's \link vital::config_block configuration block \endlink
  vital::config_block_sptr get_configuration() const override;

  /// Set this algorithm's properties via a config block
  void set_configuration(vital::config_block_sptr config) override;

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration(vital::config_block_sptr config) const override;

  void open(std::string list_name) override;
  void close() override;

  bool end_of_video() const override;
  bool good() const override;
  bool seekable() const override;
  size_t num_frames() const override;

  bool next_frame( kwiver::vital::timestamp& ts,
                   uint32_t timeout = 0 ) override;

  bool seek_frame( kwiver::vital::timestamp& ts,
                   kwiver::vital::timestamp::frame_t frame_number,
                   uint32_t timeout = 0 ) override;

  kwiver::vital::timestamp frame_timestamp() const override;
  kwiver::vital::image_container_sptr frame_image() override;
  kwiver::vital::metadata_vector frame_metadata() override;
  kwiver::vital::metadata_map_sptr metadata_map() override;

private:
  class priv;
  std::unique_ptr<priv> const d;
};

} } } // end namespace

#endif
