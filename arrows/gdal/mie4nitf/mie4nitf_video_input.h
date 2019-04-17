/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief \todo
 */

#ifndef KWIVER_ARROWS_MIE4NITF_MIE4NITF_VIDEO_INPUT_H
#define KWIVER_ARROWS_MIE4NITF_MIE4NITF_VIDEO_INPUT_H

#include <vital/algo/video_input.h>

#include <arrows/mie4nitf/kwiver_algo_mie4nitf_export.h>


namespace kwiver {
namespace arrows {
namespace mie4nitf {

/// Video input using mie4nitf services.
// ---------------------------------------------------------------------------
/**
 * This class implements a video input algorithm using mie4nitf video services.
 *
 */
class KWIVER_ALGO_MIE4NITF_EXPORT mie4nitf_video_input
  : public vital::algorithm_impl < mie4nitf_video_input, vital::algo::video_input >
{
public:
  /// Constructor
  mie4nitf_video_input();
  virtual ~mie4nitf_video_input();

  /// Get this algorithm's \link vital::config_block configuration block \endlink
  virtual vital::config_block_sptr get_configuration() const;

  /// Set this algorithm's properties via a config block
  virtual void set_configuration(vital::config_block_sptr config);

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration(vital::config_block_sptr config) const;

  virtual void open( std::string video_name );
  virtual void close();

  virtual bool end_of_video() const;
  virtual bool good() const;

  virtual bool seekable() const;
  virtual size_t num_frames() const
  {
    // NOT an actual const method !
    // We need to go through the entire video to find out
    return const_cast<mie4nitf_video_input*>(this)->private_num_frames();
  }

  virtual bool next_frame( kwiver::vital::timestamp& ts,
                           uint32_t timeout = 0 );
  virtual bool seek_frame(kwiver::vital::timestamp& ts,
                          kwiver::vital::timestamp::frame_t frame_number,
                          uint32_t timeout = 0) ;

  virtual kwiver::vital::timestamp frame_timestamp() const;
  virtual kwiver::vital::image_container_sptr frame_image();
  virtual kwiver::vital::metadata_vector frame_metadata();
  virtual kwiver::vital::metadata_map_sptr metadata_map();

private:
  /// private implementation class
  class priv;
  const std::unique_ptr<priv> d;

  size_t private_num_frames();
};

} } } // end namespace

#endif // KWIVER_ARROWS_MIE4NITF_MIE4NITF_VIDEO_INPUT_H
