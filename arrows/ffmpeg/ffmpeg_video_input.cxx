/*ckwg +29
 * Copyright 2018-2020 by Kitware, Inc.
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
 * \brief Implementation file for video input using FFMPEG.
 */

#include <arrows/ffmpeg/ffmpeg_video_input_impl.h>
#include <arrows/ffmpeg/ffmpeg_video_input.h>

namespace kwiver {
namespace arrows {
namespace ffmpeg {


// ==================================================================
ffmpeg_video_input
::ffmpeg_video_input()
  : d( new ffmpeg_video_input_impl() )
{
  attach_logger( "ffmpeg_video_input" ); // get appropriate logger

  this->set_capability(vital::algo::video_input::HAS_EOV, true);
  this->set_capability(vital::algo::video_input::HAS_FRAME_NUMBERS, true);
  this->set_capability(vital::algo::video_input::HAS_FRAME_DATA, true);
  this->set_capability(vital::algo::video_input::HAS_METADATA, false);

  this->set_capability(vital::algo::video_input::HAS_FRAME_TIME, false);
  this->set_capability(vital::algo::video_input::HAS_ABSOLUTE_FRAME_TIME, false);
  this->set_capability(vital::algo::video_input::HAS_TIMEOUT, false);
  this->set_capability(vital::algo::video_input::IS_SEEKABLE, true);
}


ffmpeg_video_input
::~ffmpeg_video_input()
{
  this->close();
}


// ------------------------------------------------------------------
// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
ffmpeg_video_input
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = vital::algo::video_input::get_configuration();

  return config;
}


// ------------------------------------------------------------------
// Set this algorithm's properties via a config block
void
ffmpeg_video_input
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated vital::config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.

  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);
}


// ------------------------------------------------------------------
bool
ffmpeg_video_input
::check_configuration(vital::config_block_sptr config) const
{
  bool retcode(true); // assume success

  return retcode;
}


// ------------------------------------------------------------------
void
ffmpeg_video_input
::open( std::string video_name )
{
  // Close any currently opened file
  this->close();

  d->video_path = video_name;

  {
    std::lock_guard< std::mutex > lock(d->open_mutex);

    if (!kwiversys::SystemTools::FileExists(d->video_path))
    {
      // Throw exception
      VITAL_THROW( kwiver::vital::file_not_found_exception, video_name, "File not found");
    }

    if (!d->open(video_name))
    {
      VITAL_THROW( kwiver::vital::video_runtime_exception, "Video stream open failed for unknown reasons");
    }
    this->set_capability(vital::algo::video_input::HAS_METADATA,
                         d->f_data_index >= 0);
    d->end_of_video = false;
  }
}


// ------------------------------------------------------------------
void
ffmpeg_video_input
::close()
{
  d->close();

  d->video_path = "";
  d->frame_advanced = 0;
  d->end_of_video = true;
  d->number_of_frames = 0;
  d->have_loop_vars = false;
  d->metadata.clear();
}

// ------------------------------------------------------------------
bool
ffmpeg_video_input
::next_frame( kwiver::vital::timestamp& ts,
              uint32_t timeout )
{
  return d->next_frame( ts, timeout );
}

// ------------------------------------------------------------------
bool ffmpeg_video_input::seek_frame(kwiver::vital::timestamp& ts,
  kwiver::vital::timestamp::frame_t frame_number,
  uint32_t timeout)
{
  // Quick return if the stream isn't open.
  if (!d->is_opened())
  {
    VITAL_THROW( vital::file_not_read_exception, d->video_path, "Video not open");
    return false;
  }
  if (frame_number <= 0)
  {
    return false;
  }

  if (timeout != 0)
  {
    LOG_WARN(this->logger(), "Timeout argument is not supported.");
  }

  bool ret = d->seek( frame_number );
  d->end_of_video = !ret;
  if (ret)
  {
    ts = d->frame_timestamp();
  };
  return ret;
}


// ------------------------------------------------------------------
kwiver::vital::image_container_sptr
ffmpeg_video_input
::frame_image( )
{
  return d->frame_image();
}


// ------------------------------------------------------------------
kwiver::vital::metadata_vector
ffmpeg_video_input
::frame_metadata()
{
  return d->current_metadata();
}


// ------------------------------------------------------------------
kwiver::vital::metadata_map_sptr
ffmpeg_video_input
::metadata_map()
{
  d->process_loop_dependencies();

  return std::make_shared<kwiver::vital::simple_metadata_map>(d->metadata_map);
}


// ------------------------------------------------------------------
bool
ffmpeg_video_input
::end_of_video() const
{
  return d->end_of_video;
}


// ------------------------------------------------------------------
bool
ffmpeg_video_input
::seekable() const
{
  return true;
}

// ------------------------------------------------------------------
size_t
ffmpeg_video_input
::num_frames() const
{
  d->process_loop_dependencies();

  return d->number_of_frames;
}

bool
ffmpeg_video_input
::good() const
{
 return d->good();
}

kwiver::vital::timestamp
ffmpeg_video_input
::frame_timestamp() const
{
  return d->frame_timestamp();
}
} } } // end namespaces
