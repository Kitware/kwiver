/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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

#ifndef KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_INPUT_IMPL_H
#define KWIVER_ARROWS_FFMPEG_FFMPEG_VIDEO_INPUT_IMPL_H

#include <arrows/ffmpeg/kwiver_algo_ffmpeg_impl_export.h>

#include <vital/exceptions/io.h>
#include <vital/exceptions/video.h>
#include <vital/klv/convert_metadata.h>
#include <vital/klv/misp_time.h>
#include <vital/klv/klv_data.h>
#include <vital/util/tokenize.h>
#include <vital/types/image_container.h>
#include <vital/types/metadata_map.h>
#include <vital/types/image.h>
#include <vital/types/timestamp.h>

#include <kwiversys/SystemTools.hxx>

#include <deque>
#include <mutex>
#include <memory>
#include <vector>
#include <sstream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <mutex>

namespace kwiver {
namespace arrows {
namespace ffmpeg {

class KWIVER_ALGO_FFMPEG_IMPL_EXPORT ffmpeg_video_input_impl
{
public:
  // f_* variables are FFMPEG specific

  AVFormatContext* f_format_context;
  int f_video_index;
  int f_data_index;
  AVCodecContext* f_video_encoding;
  AVStream* f_video_stream;
  AVFrame* f_frame;
  AVPacket f_packet;
  SwsContext* f_software_context;

  // Start time of the stream, to offset the pts when computing the frame number.
  // (in stream time base)
  int64_t f_start_time;

  // Presentation timestamp (in stream time base)
  int64_t f_pts;

  // Number of frames to back step when seek fails to land on frame before request
  int64_t f_backstep_size;

  // Some codec/file format combinations need a frame number offset.
  // These codecs have a delay between reading packets and generating frames.
  unsigned f_frame_number_offset;

  // Name of video we opened
  std::string video_path;

  // the buffer of metadata from the data stream
  std::deque<uint8_t> metadata;

  // metadata converter object
  kwiver::vital::convert_metadata converter;

  /**
   * Storage for the metadata map.
   */
  vital::metadata_map::map_metadata_t metadata_map;

  static std::mutex open_mutex;

  // For logging in priv methods
  vital::logger_handle_t logger;

  // Current image frame.
  vital::image_memory_sptr current_image_memory;
  kwiver::vital::image_container_sptr current_image;

  // local state
  int frame_advanced; // This is a boolean check value really
  bool end_of_video;
  size_t number_of_frames;
  bool have_loop_vars;

  /*
   * Constructor for ffmpeg_video_input_impl
   */
  ffmpeg_video_input_impl();

  // ==================================================================
  /*
  * @brief Whether the video was opened.
  *
  * @return \b true if video was opened.
  */
  bool is_opened();

  // ==================================================================
  /*
  * @brief Open the given video.
  *
  * @return \b true if video was opened.
  */
  bool open(std::string video_name);

  // ==================================================================
  /*
  * @brief Close the current video.
  */
  void close();

  // ==================================================================
  /*
  * @brief Advance to the next frame (but don't acquire an image).
  *
  * @return \b true if video was valid and we found a frame.
  */
  bool advance();

  // ==================================================================
  /*
  * @brief Seek to a specific frame
  *
  * @return \b true if video was valid and we found a frame.
  */
  bool seek( uint64_t frame );

  // ==================================================================
  /*
  * @brief Get the current timestamp
  *
  * @return \b Current timestamp.
  */
  double current_pts() const;

  // ==================================================================
  /*
  * @brief Returns the double value to convert from a stream time base to
  *  a frame number
  */
  double stream_time_base_to_frame() const;

  // ==================================================================
  /*
  * @brief Check if video is valid
  *
  * @return \b true if the video is valid
  */
  bool is_valid() const;

  // ==================================================================
  /*
  * @brief Return the current frame number
  *
  * @return \b Current frame number.
  */
  unsigned int frame_number() const;

  void set_default_metadata(kwiver::vital::metadata_sptr md);

  // ==================================================================
  /*
   * @brief Get metadata from the current video stream
   *
   * @return \b metadata associated with the video
   */
  kwiver::vital::metadata_vector current_metadata();

  // ==================================================================
  /*
  * @brief Loop over all frames to collect metadata and exact frame count
  *
  * @return \b Current frame number.
  */
  void process_loop_dependencies();

  // =================================================================
  /*
   * @brief Get current image in the video
   *
   * @return \b image container shared pointer from the video
   */
  kwiver::vital::image_container_sptr frame_image();

  // =================================================================
  /*
   * @brief Move video pointer to the next frame
   *
   * @return \b returns true if the video is not at end
   */
  bool next_frame( kwiver::vital::timestamp& ts,
                   uint32_t timeout=0 );

  kwiver::vital::timestamp frame_timestamp() const;

  bool good() const;
};

}
}
}
#endif
