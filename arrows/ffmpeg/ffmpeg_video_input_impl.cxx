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

#include <arrows/ffmpeg/ffmpeg_video_input_impl.h>
#include <arrows/ffmpeg/ffmpeg_init.h>
#include <vital/logger/logger.h>

std::mutex kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::open_mutex;

kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::ffmpeg_video_input_impl():
    f_format_context(nullptr),
    f_video_index(-1),
    f_data_index(-1),
    f_video_encoding(nullptr),
    f_video_stream(nullptr),
    f_frame(nullptr),
    f_software_context(nullptr),
    f_start_time(-1),
    f_backstep_size(-1),
    f_frame_number_offset(0),
    video_path(""),
    metadata(0),
    frame_advanced(0),
    end_of_video(true),
    number_of_frames(0),
    have_loop_vars(false)
{
  f_packet.data = nullptr;
  logger = kwiver::vital::get_logger( "ffmpeg_video_input_implementation" );
  ffmpeg_init();
}

bool
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::is_opened()
{
  return f_start_time != -1;
}

bool
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::open(std::string video_name)
{
  // Open the file
  int err = avformat_open_input(&f_format_context,
                                 video_path.c_str(),
                                 NULL,
                                 NULL);
  if (err != 0)
  {
    LOG_ERROR(logger,
              "Error " << err << " trying to open " << video_name);
    return false;
  }

  // Get the stream information by reading a bit of the file
  if (avformat_find_stream_info(f_format_context, NULL) < 0)
  {
    return false;
  }

  // Find a video stream, and optionally a data stream.
  // Use the first ones we find.
  f_video_index = -1;
  f_data_index = -1;
  AVCodecContext* codec_context_origin = NULL;
  for (unsigned i = 0; i < f_format_context->nb_streams; ++i)
  {
    AVCodecContext *const enc = f_format_context->streams[i]->codec;
    if (enc->codec_type == AVMEDIA_TYPE_VIDEO && f_video_index < 0)
    {
      f_video_index = i;
      codec_context_origin = enc;
    }
    else if (enc->codec_type == AVMEDIA_TYPE_DATA && f_data_index < 0)
    {
      f_data_index = i;
    }
  }

  if (f_video_index < 0)
  {
    LOG_ERROR(logger,
              "Error: could not find a video stream in " << video_path);
    return false;
  }

  if (f_data_index < 0)
  {
    LOG_INFO(logger, "No data stream available");
    // Fallback for the DATA stream if incorrectly coded as UNKNOWN.
    for (unsigned i = 0; i < f_format_context->nb_streams; ++i)
    {
      AVCodecContext *enc = f_format_context->streams[i]->codec;
      if (enc->codec_type == AVMEDIA_TYPE_UNKNOWN)
      {
        f_data_index = i;
        LOG_INFO(logger,
                 "Using AVMEDIA_TYPE_UNKNOWN stream as a data stream");
      }
    }
  }

  av_dump_format(f_format_context, 0, video_path.c_str(), 0);

  // Open the stream
  AVCodec* codec = avcodec_find_decoder(codec_context_origin->codec_id);
  if (!codec)
  {
    LOG_ERROR(logger,
              "Error: Codec " << codec_context_origin->codec_descriptor
              << " (" << codec_context_origin->codec_id << ") not found");
    return false;
  }

  // Copy context
  f_video_encoding = avcodec_alloc_context3(codec);
  if (avcodec_copy_context(f_video_encoding, codec_context_origin) != 0)
  {
    LOG_ERROR(logger,
              "Error: Could not copy codec "
              << f_video_encoding->codec_id);
    return false;
  }

  // Open codec
  if (avcodec_open2(f_video_encoding, codec, NULL) < 0)
  {
    LOG_ERROR(logger,
              "Error: Could not open codec "
              << f_video_encoding->codec_id);
    return false;
  }

  // Use group of picture (GOP) size for seek back step if avaiable
  if ( f_video_encoding->gop_size > 0 )
  {
    f_backstep_size = f_video_encoding->gop_size;
  }
  else
  {
    // If GOP size not available use 12 which is a common GOP size.
    f_backstep_size = 12;
  }

  f_video_stream = f_format_context->streams[f_video_index];
  f_frame = av_frame_alloc();

  // The MPEG 2 codec has a latency of 1 frame when encoded in an AVI
  // stream, so the pts of the last packet (stored in pts) is
  // actually the next frame's pts.
  if (f_video_stream->codec->codec_id == AV_CODEC_ID_MPEG2VIDEO &&
    std::string("avi") == f_format_context->iformat->name)
  {
    f_frame_number_offset = 1;
  }

  // Not sure if this does anything, but no harm either
  av_init_packet(&f_packet);
  f_packet.data = nullptr;
  f_packet.size = 0;

  // Advance to first valid frame to get start time
  f_start_time = 0;
  if ( advance() )
  {
      f_start_time = f_pts;
  }
  else
  {
      LOG_ERROR(logger,
                "Error: failed to find valid frame to set start time");
      f_start_time = -1;
      return false;
  }

  // Now seek back to the start of the video
  auto seek_rslt = av_seek_frame( f_format_context,
                                  f_video_index,
                                  INT64_MIN,
                                  AVSEEK_FLAG_BACKWARD );
  avcodec_flush_buffers( f_video_encoding );
  if (seek_rslt < 0 )
  {
      LOG_ERROR(logger,
                "Error: failed to return to start after setting start time");
      return false;
  }
  frame_advanced = 0;
  f_frame->data[0] = NULL;

  return true;
}

void
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::close()
{
  if (f_packet.data) {
    av_free_packet(&f_packet);  // free last packet
  }

  if (f_frame)
  {
    av_freep(&f_frame);
  }
  f_frame = nullptr;

  if (f_video_encoding && f_video_encoding->opaque)
  {
    av_freep(&f_video_encoding->opaque);
  }

  f_video_index = -1;
  f_data_index = -1;
  f_start_time = -1;

  if (f_video_stream)
  {
    avcodec_close(f_video_stream ->codec);
    f_video_stream = nullptr;
  }
  if (f_format_context)
  {
    avformat_close_input(&f_format_context);
    f_format_context = nullptr;
  }

  f_video_encoding = nullptr;
}

bool
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::advance()
{
  // Quick return if the file isn't open.
  if (!is_opened())
  {
    frame_advanced = 0;
    return false;
  }

  if (f_packet.data)
  {
    av_free_packet(&f_packet);  // free previous packet
  }
  frame_advanced = 0;

  // clear the metadata from the previous frame
  metadata.clear();

  while (frame_advanced == 0
         && av_read_frame(f_format_context, &f_packet) >= 0)
  {
    // Make sure that the packet is from the actual video stream.
    if (f_packet.stream_index == f_video_index)
    {
      int err = avcodec_decode_video2(f_video_encoding,
        f_frame, &frame_advanced,
        &f_packet);
      if (err == AVERROR_INVALIDDATA)
      {// Ignore the frame and move to the next
        av_free_packet(&f_packet);
        continue;
      }
      if (err < 0)
      {
        LOG_ERROR(logger, "Error decoding packet");
        av_free_packet(&f_packet);
        return false;
      }

      f_pts = av_frame_get_best_effort_timestamp(f_frame);
      if (f_pts == AV_NOPTS_VALUE)
      {
        f_pts = 0;
      }
    }

    // grab the metadata from this packet if from the metadata stream
    else if (f_packet.stream_index == f_data_index)
    {
      metadata.insert(metadata.end(), f_packet.data,
        f_packet.data + f_packet.size);
    }

    if (!frame_advanced)
    {
      av_free_packet(&f_packet);
    }
  }

  // From ffmpeg apiexample.c: some codecs, such as MPEG, transmit the
  // I and P frame with a latency of one frame. You must do the
  // following to have a chance to get the last frame of the video.
  if (!frame_advanced)
  {
    av_init_packet(&f_packet);
    f_packet.data = nullptr;
    f_packet.size = 0;

    int err = avcodec_decode_video2(f_video_encoding,
      f_frame, &frame_advanced,
      &f_packet);
    if (err >= 0)
    {
      f_pts += static_cast<int64_t>(stream_time_base_to_frame());
    }
  }

  // The cached frame is out of date, whether we managed to get a new
  // frame or not.
  current_image_memory = nullptr;

  if (!frame_advanced)
  {
    f_frame->data[0] = NULL;
  }

  return static_cast<bool>(frame_advanced);
}

bool
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::seek( uint64_t frame )
{
  // Time for frame before requested frame. The frame before is requested so
  // advance will called at least once in case the request lands on a keyframe.
  int64_t frame_ts = (static_cast<int>(f_frame_number_offset) + frame - 1) *
    stream_time_base_to_frame() + f_start_time;

  bool advance_successful = false;
  do
  {
    auto seek_rslt = av_seek_frame( f_format_context,
                                    f_video_index, frame_ts,
                                    AVSEEK_FLAG_BACKWARD );
    avcodec_flush_buffers( f_video_encoding );

    if ( seek_rslt < 0 )
    {
      return false;
    }

    advance_successful = advance();

    // Continue to make seek request further back until we land at a frame
    // that is before the requested frame.
    frame_ts -= f_backstep_size * stream_time_base_to_frame();
  }
  while( frame_number() > frame - 1 || !advance_successful );

  // Now advance forward until we reach the requested frame.
  while( frame_number() < frame - 1 )
  {
    if ( !advance() )
    {
      return false;
    }

    if ( frame_number() > frame -1 )
    {
      LOG_ERROR( logger, "seek went past requested frame." );
      return false;
    }
  }

  return true;
}

double
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::current_pts() const
{
  return f_pts * av_q2d(f_video_stream->time_base);
}

double
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::stream_time_base_to_frame() const
{
  if (f_video_stream->avg_frame_rate.num == 0.0)
  {
    return av_q2d(av_inv_q(av_mul_q(f_video_stream->time_base,
      f_video_stream->r_frame_rate)));
  }
  return av_q2d(
         av_inv_q(
         av_mul_q(f_video_stream->time_base,
                  f_video_stream->avg_frame_rate)));
}

bool
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::is_valid() const
{
  return f_frame && f_frame->data[0];
}

unsigned int
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::frame_number() const
{
  // Quick return if the stream isn't open.
  if (!is_valid())
  {
    return static_cast<unsigned int>(-1);
  }

  return static_cast<unsigned int>(
    (f_pts - f_start_time) / stream_time_base_to_frame()
    - static_cast<int>(f_frame_number_offset));
}

void
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::set_default_metadata(kwiver::vital::metadata_sptr md)
{
  // Add frame number to timestamp
  kwiver::vital::timestamp ts;
  ts.set_frame( frame_number() );
  md->set_timestamp( ts );

  // Add file name/uri
  md->add( NEW_METADATA_ITEM( vital::VITAL_META_VIDEO_URI, video_path ) );

  // Mark whether the frame is a key frame
  if ( f_frame->key_frame > 0 )
  {
    md->add( NEW_METADATA_ITEM( vital::VITAL_META_VIDEO_KEY_FRAME, true ) );
  }
  else
  {
    md->add( NEW_METADATA_ITEM( vital::VITAL_META_VIDEO_KEY_FRAME, false ) );
  }
}

kwiver::vital::metadata_vector
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::current_metadata()
{
  kwiver::vital::metadata_vector retval;

  // Copy the current raw metadata
  std::deque<uint8_t> md_buffer = metadata;

  kwiver::vital::klv_data klv_packet;

  // If we have collected enough of the stream to make a KLV packet
  while ( klv_pop_next_packet( md_buffer, klv_packet ) )
  {
    auto meta = std::make_shared<kwiver::vital::metadata>();

    try
    {
      converter.convert( klv_packet, *(meta) );
    }
    catch ( kwiver::vital::metadata_exception const& e )
    {
      LOG_WARN( logger, "Metadata exception: " << e.what() );
      continue;
    }

    // If the metadata was even partially decided, then add to the list.
    if ( ! meta->empty() )
    {
      set_default_metadata( meta );

      retval.push_back( meta );
    } // end valid metadata packet.
  } // end while

  // if no metadata from the stream, add a basic metadata item
  if ( retval.empty() )
  {
    auto meta = std::make_shared<kwiver::vital::metadata>();
    set_default_metadata( meta );

    retval.push_back(meta);
  }

  return retval;
}

void
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::process_loop_dependencies()
{
  // is stream open?
  if ( ! is_opened() )
  {
    VITAL_THROW( vital::file_not_read_exception, video_path, "Video not open" );
  }

  if ( !have_loop_vars )
  {
    std::lock_guard< std::mutex > lock( open_mutex );

    auto initial_frame_number = frame_number();

    if ( !frame_advanced && !end_of_video )
    {
      initial_frame_number = 0;
    }

    // Add metadata for current frame
    if ( frame_advanced )
    {
      number_of_frames++;
      metadata_map.insert(
        std::make_pair( frame_number(), current_metadata() ) );
    }

    // Advance video stream to end
    while ( advance() )
    {
      number_of_frames++;
      metadata_map.insert(
        std::make_pair( frame_number(), current_metadata() ) );
    }

    // Close and reopen to reset
    close();
    open( video_path );

    // Advance back to original frame number
    unsigned int frame_num = 0;
    while ( frame_num < initial_frame_number && advance() )
    {
      number_of_frames++;
      ++frame_num;
      metadata_map.insert(
        std::make_pair( frame_number(), current_metadata() ) );
    }

    have_loop_vars = true;
  }
}

kwiver::vital::image_container_sptr
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::frame_image()
{
  // Quick return if the stream isn't valid
  if (!is_valid())
  {
    return nullptr;
  }

  AVCodecContext* enc = f_format_context->streams[f_video_index]->codec;

  // If we have not already converted this frame, try to convert it
  if (!current_image_memory && f_frame->data[0] != 0)
  {
    int width = enc->width;
    int height = enc->height;
    int depth = 3;
    vital::image_pixel_traits  pixel_trait = vital::image_pixel_traits_of<unsigned char>();
    bool direct_copy;

    // If the pixel format is not recognized by then convert the data into RGB_24
    switch (enc->pix_fmt)
    {
      case AV_PIX_FMT_GRAY8:
      {
        depth = 1;
        direct_copy = true;
        break;
      }
      case AV_PIX_FMT_RGBA:
      {
        depth = 4;
        direct_copy = true;
        break;
      }
      case AV_PIX_FMT_MONOWHITE:
      case AV_PIX_FMT_MONOBLACK:
      {
        depth = 1;
        pixel_trait = vital::image_pixel_traits_of<bool>();
        direct_copy = true;
        break;
      }
      default:
      {
        direct_copy = false;
      }
    }
    if (direct_copy)
    {
      int size = avpicture_get_size(enc->pix_fmt, width, height);
      current_image_memory = vital::image_memory_sptr(new vital::image_memory(size));

      AVPicture frame;
      avpicture_fill(&frame, (uint8_t*)current_image_memory->data(),
                     enc->pix_fmt, width, height);
      av_picture_copy(&frame, (AVPicture*)f_frame, enc->pix_fmt, width, height);
    }
    else
    {
      int size = width * height * depth;
      current_image_memory = std::make_shared<vital::image_memory>(size);

      f_software_context = sws_getCachedContext(f_software_context,
                                                width, height, enc->pix_fmt,
                                                width, height, AV_PIX_FMT_RGB24,
                                                SWS_BILINEAR,
                                                NULL, NULL, NULL);

      if (!f_software_context)
      {
        LOG_ERROR(logger, "Couldn't create conversion context");
        return nullptr;
      }

      AVPicture rgb_frame;
      avpicture_fill(&rgb_frame, (uint8_t*)current_image_memory->data(),
                     AV_PIX_FMT_RGB24, width, height);

      sws_scale(f_software_context,
                f_frame->data, f_frame->linesize,
                0, height,
                rgb_frame.data, rgb_frame.linesize);
    }

    vital::image image(current_image_memory,
                       current_image_memory->data(),
                       width, height, depth,
                       depth, depth * width, 1
                      );
    current_image = std::make_shared<vital::simple_image_container>(
                                        vital::simple_image_container(image));
  }

  return current_image;
}

bool
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::next_frame( kwiver::vital::timestamp& ts,
                                                             uint32_t timeout )
{
  if (!is_opened())
  {
    VITAL_THROW( vital::file_not_read_exception, video_path, "Video not open");
  }

  bool ret = advance();

  end_of_video = !ret;
  if (ret)
  {
    ts = frame_timestamp();
  }
  return ret;
}


// ------------------------------------------------------------------
kwiver::vital::timestamp
kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::frame_timestamp() const
{
  if (!good())
  {
    return {};
  }

  // We don't always have all components of a timestamp, so start with
  // an invalid TS and add the data we have.
  kwiver::vital::timestamp ts;
  ts.set_frame(frame_number() + f_frame_number_offset + 1);

  return ts;
}


// ------------------------------------------------------------------
bool
 kwiver::arrows::ffmpeg::ffmpeg_video_input_impl::good() const
{
  return is_valid() && frame_advanced;
}
