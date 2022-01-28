// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation file for video input using FFmpeg.

#include "ffmpeg_init.h"
#include "ffmpeg_video_input.h"
#include "ffmpeg_video_settings.h"

#include <arrows/klv/klv_convert_vital.h>
#include <arrows/klv/klv_demuxer.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/klv/klv_muxer.h>
#include <arrows/klv/misp_time.h>

#include <vital/exceptions/io.h>
#include <vital/exceptions/video.h>

#include <vital/range/iota.h>

#include <vital/types/image_container.h>
#include <vital/types/timestamp.h>

#include <vital/util/tokenize.h>

#include <vital/vital_config.h>

#include <kwiversys/SystemTools.hxx>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <iomanip>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

namespace kv = kwiver::vital;
namespace kvr = kv::range;

namespace kwiver {

namespace arrows {

namespace ffmpeg {

namespace {

// ----------------------------------------------------------------------------
struct ffmpeg_klv_stream
{
  ffmpeg_klv_stream( AVStream* stream );

  ffmpeg_klv_stream( ffmpeg_klv_stream const& ) = delete;
  ffmpeg_klv_stream( ffmpeg_klv_stream&& ) = delete;

  void send_packet( AVPacket* packet );

  void advance( kv::optional< uint64_t > backup_timestamp = kv::nullopt,
                int64_t max_pts = INT64_MAX );

  void reset();

  kv::metadata_sptr vital_metadata( uint64_t timestamp, bool smooth_packets );

  AVStream* stream;
  std::multimap< int64_t, std::vector< uint8_t > > buffer;
  std::vector< uint8_t > bytes;
  std::vector< klv::klv_packet > packets;
  klv::klv_timeline timeline;
  klv::klv_demuxer demuxer;
  klv::klv_muxer muxer;
};

// ----------------------------------------------------------------------------
ffmpeg_klv_stream
::ffmpeg_klv_stream( AVStream* stream )
: stream{ stream },
  buffer{},
  bytes{},
  packets{},
  timeline{},
  demuxer( timeline ),
  muxer( timeline )
{
  if( !stream )
  {
    throw std::logic_error( "ffmpeg_klv_stream given null stream" );
  }
  if( stream->codecpar->codec_id != AV_CODEC_ID_SMPTE_KLV )
  {
    throw std::logic_error( "ffmpeg_klv_stream given non-KLV stream" );
  }
}

// ----------------------------------------------------------------------------
void
ffmpeg_klv_stream
::send_packet( AVPacket* packet )
{
  if( packet->stream_index != stream->index )
  {
    return;
  }
  auto const begin = packet->data;
  auto const end = begin + packet->size;
  buffer.emplace( packet->pts, std::vector< uint8_t >{ begin, end } );
}

// ----------------------------------------------------------------------------
void
ffmpeg_klv_stream
::advance( kv::optional< uint64_t > backup_timestamp, int64_t max_pts )
{
  packets.clear();

  for( auto it = buffer.begin(); it != buffer.end(); )
  {
    if( it->first <= max_pts || it->first == AV_NOPTS_VALUE )
    {
      bytes.insert( bytes.end(), it->second.begin(), it->second.end() );
      it = buffer.erase( it );
    }
    else
    {
      break;
    }
  }

  auto it = bytes.cbegin();
  while( it != bytes.cend() )
  {
    try
    {
      auto const length =
        static_cast< size_t >( std::distance( it, bytes.cend() ) );
      packets.emplace_back( klv::klv_read_packet( it, length ) );
    }
    catch( kwiver::vital::metadata_buffer_overflow const& )
    {
      // We only have part of a packet; quit until we have more data
      break;
    }
    catch( kwiver::vital::metadata_exception const& e )
    {
      LOG_ERROR( kwiver::vital::get_logger( "klv" ),
                 "error while parsing KLV packet: " << e.what() );
      it = bytes.cend();
    }
  }

  // Weirdness here to get around CentOS compiler bug
  bytes.erase( bytes.begin(),
               bytes.begin() + std::distance( bytes.cbegin(), it ) );

  if( packets.empty() )
  {
    return;
  }

  demuxer.send_frame( packets, backup_timestamp );
}

// ----------------------------------------------------------------------------
void
ffmpeg_klv_stream
::reset()
{
  buffer.clear();
  bytes.clear();
  packets.clear();
  timeline.clear();
  demuxer.reset();
  muxer.reset();
}

// ----------------------------------------------------------------------------
kv::metadata_sptr
ffmpeg_klv_stream
::vital_metadata( uint64_t timestamp, bool smooth_packets )
{
  auto result = klv::klv_to_vital_metadata( timeline, timestamp );
  auto& klv_result = dynamic_cast< klv::klv_metadata& >( *result );
  if( smooth_packets )
  {
    muxer.send_frame( timestamp );
    klv_result.set_klv( muxer.receive_frame() );
  }
  else
  {
    klv_result.set_klv( packets );
  }
  klv_result.add< kv::VITAL_META_METADATA_ORIGIN >( "KLV" );
  klv_result.add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( stream->index );
  return result;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
// Private implementation class
class ffmpeg_video_input::priv
{
public:
  // f_* variables are FFmpeg specific

  AVFormatContext* f_format_context = avformat_alloc_context();
  AVCodecContext* f_video_encoding = nullptr;
  AVStream* f_video_stream = nullptr;
  AVFrame* f_frame = nullptr;
  AVFrame* f_filtered_frame = nullptr;
  AVPacket* f_packet = nullptr;
  SwsContext* f_software_context = nullptr;
  AVFilterGraph* f_filter_graph = nullptr;
  AVFilterContext* f_filter_sink_context = nullptr;
  AVFilterContext* f_filter_src_context = nullptr;

  // Start time of the stream, to offset the pts when computing the frame
  // number
  // (in stream time base)
  int64_t f_start_time = -1;

  // Presentation timestamp (in stream time base)
  int64_t f_pts;

  // MISP timestamp (microseconds)
  std::map< uint64_t, klv::misp_timestamp > m_pts_to_misp;

  // Number of frames to back step when seek fails to land on frame before
  // request
  int64_t f_backstep_size = -1;

  // Some codec/file format combinations need a frame number offset.
  // These codecs have a delay between reading packets and generating frames.
  unsigned f_frame_number_offset = 0;

  // Name of video we opened
  std::string video_path = "";

  // FFMPEG filter description string
  // What you put after -vf in the ffmpeg command line tool
  std::string filter_desc = "yadif=deint=1";

  // Storage for current frame's raw metadata
  std::list< ffmpeg_klv_stream > klv_streams;


  /// Storage for the metadata map.
  vital::metadata_map::map_metadata_t metadata_map;
  kv::metadata_vector metadata;

  static std::mutex open_mutex;

  // For logging in priv methods
  vital::logger_handle_t logger;

  // Current image frame.
  vital::image_memory_sptr current_image_memory;
  kwiver::vital::image_container_sptr current_image;

  // local state
  bool frame_advanced = false;
  bool end_of_video = true;
  size_t number_of_frames = 0;
  bool collected_all_metadata = false;
  bool estimated_num_frames = false;
  bool sync_metadata = true;
  bool use_misp_timestamps = false;
  bool smooth_klv_packets = false;
  bool is_draining = false;
  size_t max_seek_back_attempts = 10;

  // --------------------------------------------------------------------------
  priv() {}

  // --------------------------------------------------------------------------

  ///  @brief Whether the video was opened.
  ///
  ///  @return \b true if video was opened.
  bool
  is_opened()
  {
    return this->f_start_time != -1;
  }

  // --------------------------------------------------------------------------

  ///  @brief Open the given video.
  ///
  ///  @return \b true if video was opened.
  bool
  open( std::string video_name )
  {
    // Open the file
    auto err =
      avformat_open_input( &f_format_context, video_path.c_str(), NULL, NULL );
    if( err != 0 )
    {
      LOG_ERROR( logger, "Error " << err << " trying to open " << video_name );
      return false;
    }

    // Get the stream information by reading a bit of the file
    if( avformat_find_stream_info( f_format_context, NULL ) < 0 )
    {
      return false;
    }

    // Find a video stream, and optionally a data stream.
    // Use the first ones we find.
    for( auto const i : kvr::iota( f_format_context->nb_streams ) )
    {
      auto const stream = f_format_context->streams[ i ];
      auto const params = stream->codecpar;
      if( params->codec_type == AVMEDIA_TYPE_VIDEO )
      {
        f_video_stream = stream;
      }
      else if( params->codec_id == AV_CODEC_ID_SMPTE_KLV )
      {
        klv_streams.emplace_back( stream );
      }
    }

    if( !f_video_stream )
    {
      LOG_ERROR( logger,
                 "Error: could not find a video stream in " << video_path );
      return false;
    }
    auto const video_params = f_video_stream->codecpar;

    LOG_INFO( logger, "Found " << klv_streams.size() << " KLV stream(s)" );

    av_dump_format( f_format_context, 0, video_path.c_str(), 0 );

    auto const codec_descriptor =
      avcodec_descriptor_get( video_params->codec_id );
    std::string const codec_name =
      codec_descriptor ? codec_descriptor->long_name : "<unknown>";

    // Open the stream
    auto const codec = avcodec_find_decoder( video_params->codec_id );
    if( !codec )
    {
      LOG_ERROR( logger,
                 "Error: Codec " << codec_name << " "
                 << "(" << video_params->codec_id << ") not found" );
      return false;
    }

    // Copy context
    f_video_encoding = avcodec_alloc_context3( codec );
    if( avcodec_parameters_to_context( f_video_encoding, video_params ) > 0 )
    {
      LOG_ERROR( logger,
                 "Error: Could not fill codec context " << codec_name );
      return false;
    }

    // Open codec
    if( avcodec_open2( f_video_encoding, codec, NULL ) < 0 )
    {
      LOG_ERROR( logger,
                 "Error: Could not open codec " << f_video_encoding->codec_id );
      return false;
    }

    if( !std::all_of( filter_desc.begin(), filter_desc.end(), isspace ) &&
        !init_filters( filter_desc ) )
    {
      return false;
    }

    // Use group of picture (GOP) size for seek back step if avaiable
    // If GOP size not available use 12 which is a common GOP size.
    f_backstep_size =
      ( f_video_encoding->gop_size > 0 ) ? f_video_encoding->gop_size : 12;

    f_frame = av_frame_alloc();
    f_filtered_frame = av_frame_alloc();
    f_packet = av_packet_alloc();

    // The MPEG 2 codec has a latency of 1 frame when encoded in an AVI
    // stream, so the pts of the last packet (stored in pts) is
    // actually the next frame's pts.
    if( codec->id == AV_CODEC_ID_MPEG2VIDEO &&
        std::string( "avi" ) == f_format_context->iformat->name )
    {
      f_frame_number_offset = 1;
    }

    // Start time taken from the first decodable frame
    av_seek_frame( f_format_context, f_video_stream->index, 0,
                   AVSEEK_FLAG_FRAME );
    int send_err;
    int recv_err;
    do {
      // Read frames until we can successfully decode one
      av_read_frame( f_format_context, f_packet );
      send_err = avcodec_send_packet( f_video_encoding, f_packet );
      recv_err = avcodec_receive_frame( f_video_encoding, f_frame );
      av_packet_unref( f_packet );
    } while( send_err || recv_err );
    f_start_time = f_frame->best_effort_timestamp;
    // Seek back to start
    av_seek_frame( f_format_context, f_video_stream->index, 0,
                   AVSEEK_FLAG_FRAME );
    avcodec_flush_buffers( f_video_encoding );

    frame_advanced = false;
    f_frame->data[ 0 ] = NULL;
    return true;
  }

  // --------------------------------------------------------------------------

  ///  @brief Close the current video.
  void
  close()
  {
    f_start_time = -1;
    klv_streams.clear();
    is_draining = false;
    metadata.clear();
    f_video_stream = nullptr;
    av_frame_free( &f_frame );
    av_frame_free( &f_filtered_frame );
    av_packet_free( &f_packet );
    avformat_close_input( &f_format_context );
    avformat_free_context( f_format_context );
    avcodec_free_context( &f_video_encoding );
    avfilter_graph_free( &f_filter_graph );
  }

  // --------------------------------------------------------------------------

  ///  @brief Initialize the filter graph
  bool
  init_filters( std::string const& filters_desc )
  {
    auto deleter =
      []( AVFilterInOut** ptr ){
        avfilter_inout_free( ptr );
        delete[] ptr;
      };

    using AVFilterInOut_ptr =
      std::unique_ptr< AVFilterInOut*, decltype( deleter ) >;

    char args[ 512 ];
    int ret = 0;
    auto* const buffersrc = avfilter_get_by_name( "buffer" );
    auto* const buffersink = avfilter_get_by_name( "buffersink" );
    AVFilterInOut_ptr outputs( new AVFilterInOut*[ 1 ], deleter );
    *outputs.get() = avfilter_inout_alloc();

    AVFilterInOut_ptr inputs( new AVFilterInOut*[ 1 ], deleter );
    *inputs.get() = avfilter_inout_alloc();

    AVRational time_base = f_video_stream->time_base;
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_RGB24, AV_PIX_FMT_GRAY8,
                                      AV_PIX_FMT_NONE };
    this->f_filter_graph = avfilter_graph_alloc();
    if( !outputs || !inputs || !f_filter_graph )
    {
      LOG_ERROR( this->logger, "Failed to alloation filter graph" );
      return false;
    }
    // Buffer video source
    // The decoded frames from the decoder will be inserted here.
    snprintf( args, sizeof( args ),
              "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
              f_video_encoding->width, f_video_encoding->height,
              f_video_encoding->pix_fmt,
              time_base.num, time_base.den,
              f_video_encoding->sample_aspect_ratio.num,
              f_video_encoding->sample_aspect_ratio.den );
    ret = avfilter_graph_create_filter( &f_filter_src_context, buffersrc, "in",
                                        args, NULL, f_filter_graph );
    if( ret < 0 )
    {
      LOG_ERROR( this->logger, "Cannot create buffer source" );
      return false;
    }
    // Buffer video sink
    // To terminate the filter chain.
    ret = avfilter_graph_create_filter( &f_filter_sink_context,
                                        buffersink, "out",
                                        NULL, NULL, f_filter_graph );
    if( ret < 0 )
    {
      LOG_ERROR( this->logger, "Cannot create buffer sink" );
      return false;
    }
    ret = av_opt_set_int_list( f_filter_sink_context, "pix_fmts", pix_fmts,
                               AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN );
    if( ret < 0 )
    {
      LOG_ERROR( this->logger, "Cannot set output pixel format" );
      return false;
    }

    // Set the endpoints for the filter graph. The filter_graph will
    // be linked to the graph described by filters_desc.

    // The buffer source output must be connected to the input pad of
    // the first filter described by filters_desc; since the first
    // filter input label is not specified, it is set to "in" by
    // default.
    ( *outputs )->name = av_strdup( "in" );
    ( *outputs )->filter_ctx = f_filter_src_context;
    ( *outputs )->pad_idx = 0;
    ( *outputs )->next = NULL;

    // The buffer sink input must be connected to the output pad of
    // the last filter described by filters_desc; since the last
    // filter output label is not specified, it is set to "out" by
    // default.
    ( *inputs )->name = av_strdup( "out" );
    ( *inputs )->filter_ctx = f_filter_sink_context;
    ( *inputs )->pad_idx = 0;
    ( *inputs )->next = NULL;

    if( avfilter_graph_parse_ptr( f_filter_graph, filters_desc.c_str(),
                                  inputs.get(), outputs.get(), NULL ) < 0 )
    {
      LOG_ERROR( this->logger, "Failed to parse AV filter graph" );
      return false;
    }

    if( avfilter_graph_config( f_filter_graph, NULL ) < 0 )
    {
      LOG_ERROR( this->logger, "Failed to configure AV filter graph" );
      return false;
    }

    return true;
  }

  // --------------------------------------------------------------------------
  // Try to get a decoded frame from FFmpeg, return FFmpeg error code.
  int
  query_frame()
  {
    auto const result = avcodec_receive_frame( f_video_encoding, f_frame );

    if( result < 0 )
    {
      return result;
    }

    f_pts = f_frame->best_effort_timestamp;
    if( f_pts == AV_NOPTS_VALUE )
    {
      f_pts = 0;
    }

    frame_advanced = true;

    return result;
  }

  // --------------------------------------------------------------------------
  ///  @brief Advance to the next frame (but don't acquire an image).
  ///
  ///  @return \b true if video was valid and we found a frame.
  bool
  advance()
  {
    frame_advanced = false;
    metadata.clear();

    // Quick return if the file isn't open.
    if( !is_opened() )
    {
      return false;
    }

    if( is_draining )
    {
      frame_advanced = query_frame() >= 0;
    }

    while( !frame_advanced && !is_draining &&
           av_read_frame( f_format_context, f_packet ) >= 0 )
    {
      // Video stream packet?
      if( f_packet->stream_index == f_video_stream->index )
      {
        auto const packet_begin = f_packet->data;
        auto const packet_end = f_packet->data + f_packet->size;
        auto misp_it = klv::find_misp_timestamp( packet_begin, packet_end );
        if( misp_it != packet_end )
        {
          auto const timestamp = klv::read_misp_timestamp( misp_it );
          m_pts_to_misp.emplace( f_packet->pts, timestamp );
        }

        auto err = avcodec_send_packet( f_video_encoding, f_packet );
        if( err < 0 )
        {
          LOG_ERROR( logger, "Error sending packet to decoder" );
          return false;
        }

        err = query_frame();

        // Ignore the frame and move to the next
        if( err == AVERROR_INVALIDDATA || err == AVERROR( EAGAIN ) )
        {
          av_packet_unref( f_packet );
          continue;
        }
        if( err < 0 )
        {
          LOG_ERROR( logger, "Error decoding packet" );
          av_packet_unref( f_packet );
          return false;
        }
      }

      // KLV packet?
      for( auto& stream : klv_streams )
      {
        if( f_packet->stream_index == stream.stream->index )
        {
          stream.send_packet( f_packet );
          break;
        }
      }

      // Free packet
      av_packet_unref( f_packet );
    }

    // End of video? Get all still-buffered frames from decoder
    if( !frame_advanced && !is_draining )
    {
      is_draining = true;
      avcodec_send_packet( f_video_encoding, nullptr );
      if( query_frame() < 0 )
      {
        frame_advanced = false;
      }
    }

    // The cached frame is out of date, whether we managed to get a new
    // frame or not.
    current_image_memory = nullptr;
    if( !frame_advanced )
    {
      f_frame->data[ 0 ] = NULL;
    }

    // Advance KLV
    for( auto& stream : klv_streams )
    {
      auto const frame_delta =
        av_q2d( av_inv_q( f_video_stream->avg_frame_rate ) );
      uint64_t const backup_timestamp =
        stream.demuxer.frame_time() +
        static_cast< uint64_t >( frame_delta ) * 1000000;

      stream.advance( backup_timestamp,
                      sync_metadata ? f_frame->pts : INT64_MAX );
    }

    return frame_advanced;
  }

  // --------------------------------------------------------------------------

  ///  @brief Seek to a specific frame
  ///
  ///  @return \b true if video was valid and we found a frame.
  bool
  seek( uint64_t frame )
  {
    is_draining = false;

    // Time for frame before requested frame. The frame before is requested so
    // advance will called at least once in case the request lands on a
    // keyframe.
    int64_t frame_ts =
      ( static_cast< int >( f_frame_number_offset ) + frame - 1 ) *
      this->stream_time_base_to_frame() + this->f_start_time;

    bool advance_successful = false;
    size_t num_of_attempts = 0;
    do
    {
      metadata.clear();
      for( auto& stream : klv_streams )
      {
        stream.reset();
      }

      auto err =
        av_seek_frame( f_format_context, f_video_stream->index, frame_ts,
                       AVSEEK_FLAG_BACKWARD );
      avcodec_flush_buffers( f_video_encoding );

      if( err < 0 )
      {
        return false;
      }

      advance_successful = advance();

      // Continue to make seek request further back until we land at a frame
      // that is before the requested frame.
      frame_ts -= f_backstep_size * stream_time_base_to_frame();
      if( ++num_of_attempts > max_seek_back_attempts )
      {
        LOG_ERROR( logger,
                   "Seek failed: unable to seek back to early timestamp" );
        return false;
      }
    } while( frame_number() > frame - 1 || !advance_successful );

    // Now advance forward until we reach the requested frame.
    while( frame_number() < frame - 1 )
    {
      if( !advance() )
      {
        return false;
      }

      if( frame_number() > frame - 1 )
      {
        LOG_ERROR( logger, "seek went past requested frame." );
        return false;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  ///  @brief Get the current timestamp
  ///
  ///  @return \b Current timestamp.
  double
  current_pts() const
  {
    return this->f_pts * av_q2d( this->f_video_stream->time_base );
  }

  // --------------------------------------------------------------------------

  ///  @brief Returns the double value to convert from a stream time base to
  ///   a frame number
  double
  stream_time_base_to_frame() const
  {
    if( this->f_video_stream->avg_frame_rate.num == 0.0 )
    {
      return av_q2d(
        av_inv_q(
          av_mul_q( this->f_video_stream->time_base,
                    this->f_video_stream->r_frame_rate ) ) );
    }
    return av_q2d(
      av_inv_q(
        av_mul_q( this->f_video_stream->time_base,
                  this->f_video_stream->avg_frame_rate ) ) );
  }

  bool
  is_valid() const
  {
    return this->f_frame && this->f_frame->data[ 0 ];
  }

  // --------------------------------------------------------------------------

  ///  @brief Return the current frame number
  ///
  ///  @return \b Current frame number.
  unsigned int
  frame_number() const
  {
    // Quick return if the stream isn't open or we're before the first
    // decodable frame.
    if( !this->is_valid() || this->f_pts < this->f_start_time )
    {
      return static_cast< unsigned int >( -1 );
    }

    return static_cast< unsigned int >(
      ( this->f_pts - this->f_start_time ) /
      this->stream_time_base_to_frame() -
      static_cast< int >( this->f_frame_number_offset ) + 0.5 );
  }

  void
  set_default_metadata( kwiver::vital::metadata_sptr md )
  {
    // Add frame number to timestamp
    kwiver::vital::timestamp ts;
    ts.set_frame( this->frame_number() + 1 );
    auto const time_seconds =
      ( f_pts - f_start_time ) * av_q2d( this->f_video_stream->time_base );
    ts.set_time_usec( static_cast< uint64_t >( time_seconds * 1000000.0 ) ) ;
    md->set_timestamp( ts );

    // Add file name/uri
    md->add< vital::VITAL_META_VIDEO_URI >( video_path );

    // Mark whether the frame is a key frame
    md->add< vital::VITAL_META_VIDEO_KEY_FRAME >( f_frame->key_frame > 0 );

    // Add image dimensions
    md->add< vital::VITAL_META_IMAGE_WIDTH >( f_frame->width );
    md->add< vital::VITAL_META_IMAGE_HEIGHT >( f_frame->height );

    // Add frame rate
    if( f_video_stream->avg_frame_rate.num > 0 )
    {
      md->add< vital::VITAL_META_VIDEO_FRAME_RATE >(
        av_q2d( f_video_stream->avg_frame_rate ) );
    }

    // Add bitrate
    auto bitrate = f_video_encoding->bit_rate;
    if( !bitrate )
    {
      bitrate = f_video_encoding->bit_rate_tolerance;
    }
    if( bitrate )
    {
      md->add< vital::VITAL_META_VIDEO_BITRATE >( bitrate );
    }

    // Add compression information
    static std::map< int, std::string > h262_profiles = {
      { FF_PROFILE_MPEG2_SIMPLE, "Simple" },
      { FF_PROFILE_MPEG2_MAIN, "Main" },
      { FF_PROFILE_MPEG2_SNR_SCALABLE, "SNR Scalable" },
      { FF_PROFILE_MPEG2_SS, "Spatially Scalable" },
      { FF_PROFILE_MPEG2_HIGH, "High" },
      { FF_PROFILE_MPEG2_422, "4:2:2" },
    };
    static std::map< int, std::string > h262_levels = {
      { 10, "Low" },
      { 8, "Main" },
      { 6, "High-1440" },
      { 4, "High" },
    };
    static std::map< int, std::string > h264_profiles = {
      { FF_PROFILE_H264_BASELINE, "Baseline" },
      { FF_PROFILE_H264_CONSTRAINED_BASELINE, "Constrained Baseline" },
      { FF_PROFILE_H264_MAIN, "Main" },
      { FF_PROFILE_H264_EXTENDED, "Extended" },
      { FF_PROFILE_H264_HIGH, "High" },
      { FF_PROFILE_H264_HIGH_10, "High 10" },
      { FF_PROFILE_H264_HIGH_422, "High 4:2:2" },
      { FF_PROFILE_H264_HIGH_444_PREDICTIVE, "High 4:4:4 Predictive" },
      { FF_PROFILE_H264_HIGH_10_INTRA, "High 10 Intra" },
      { FF_PROFILE_H264_HIGH_422_INTRA, "High 4:2:2 Intra" },
      { FF_PROFILE_H264_HIGH_444_INTRA, "High 4:4:4 Intra" },
      { FF_PROFILE_H264_CAVLC_444, "CAVLC 4:4:4 Intra" },
    };
    static std::map< int, std::string > h265_profiles = {
      { FF_PROFILE_HEVC_MAIN, "Main" },
      { FF_PROFILE_HEVC_MAIN_10, "Main 10" },
      { FF_PROFILE_HEVC_MAIN_STILL_PICTURE, "Main Still Picture" },
    };

    std::string compression_type;
    std::string compression_profile;
    std::string compression_level;
    switch( f_video_encoding->codec_id )
    {
      case AV_CODEC_ID_MPEG2VIDEO:
      {
        compression_type = "H.262";
        auto const profile_it =
          h262_profiles.find( f_video_encoding->profile );
        compression_profile =
          ( profile_it == h262_profiles.end() ) ? "Other" : profile_it->second;
        auto const level_it = h262_levels.find( f_video_encoding->level );
        compression_level =
          ( level_it == h262_levels.end() ) ? "Other" : level_it->second;
        break;
      }
      case AV_CODEC_ID_H264:
      {
        compression_type = "H.264";
        auto const profile_it =
          h264_profiles.find( f_video_encoding->profile );
        compression_profile =
          ( profile_it == h264_profiles.end() ) ? "Other" : profile_it->second;
        std::stringstream ss;
        ss << std::setprecision( 2 )
           << ( f_video_encoding->level / 10.0 );
        compression_level = ss.str();
        break;
      }
      case AV_CODEC_ID_H265:
      {
        compression_type = "H.265";
        auto const profile_it =
          h265_profiles.find( f_video_encoding->profile );
        compression_profile =
          ( profile_it == h265_profiles.end() ) ? "Other" : profile_it->second;
        std::stringstream ss;
        ss << std::setprecision( 2 )
           << ( f_video_encoding->level / 30.0 );
        compression_level = ss.str();
        break;
      }
      default:
        break;
    }

    if( !compression_type.empty() )
    {
      md->add< vital::VITAL_META_VIDEO_COMPRESSION_TYPE >( compression_type );
    }

    if( !compression_profile.empty() )
    {
      md->add< vital::VITAL_META_VIDEO_COMPRESSION_PROFILE >(
        compression_profile );
    }

    if( !compression_level.empty() )
    {
      md->add< vital::VITAL_META_VIDEO_COMPRESSION_LEVEL >( compression_level );
    }
  }

  kwiver::vital::metadata_vector
  current_metadata()
  {
    if( !metadata.empty() )
    {
      return metadata;
    }

    uint64_t misp_timestamp = 0;
    if( use_misp_timestamps )
    {
      auto const it = m_pts_to_misp.find( f_frame->pts );
      if( it != m_pts_to_misp.end() )
      {
        misp_timestamp = it->second.timestamp;
      }
      else
      {
        LOG_ERROR( logger,
                   "No MISP timestamp found for frame " << frame_number() );
      }
    }

    for( auto& stream : klv_streams )
    {
      auto const timestamp =
        misp_timestamp
        ? misp_timestamp
        : stream.demuxer.frame_time();
      auto stream_metadata =
        stream.vital_metadata( timestamp, smooth_klv_packets );
      set_default_metadata( stream_metadata );
      metadata.emplace_back( std::move( stream_metadata ) );
    }

    if( metadata.empty() )
    {
      auto default_metadata = std::make_shared< kv::metadata >();
      set_default_metadata( default_metadata );
      metadata.emplace_back( std::move( default_metadata ) );
    }

    return metadata;
  }

  // --------------------------------------------------------------------------

  ///  @brief Loop over all frames to collect metadata
  void
  collect_all_metadata()
  {
    // is stream open?
    if( !this->is_opened() )
    {
      VITAL_THROW( vital::file_not_read_exception, video_path,
                   "Video not open" );
    }

    if( !collected_all_metadata )
    {
      std::lock_guard< std::mutex > lock( open_mutex );

      auto initial_frame_number = this->frame_number();

      if( !frame_advanced && !end_of_video )
      {
        initial_frame_number = 0;
      }

      // Add metadata for current frame
      if( frame_advanced )
      {
        this->metadata_map.emplace(
            this->frame_number(), this->current_metadata() );
      }

      // Advance video stream to end
      while( this->advance() )
      {
        this->metadata_map.emplace(
            this->frame_number(), this->current_metadata() );
      }

      // Close and reopen to reset
      this->close();
      this->open( video_path );

      // Advance back to original frame number
      unsigned int frame_num = 0;
      while( frame_num < initial_frame_number && this->advance() )
      {
        ++frame_num;
        this->metadata_map.emplace(
            this->frame_number(), this->current_metadata() );
      }

      collected_all_metadata = true;
    }
  }

  // --------------------------------------------------------------------------

  ///  @brief Seek to the end of the video to estimate number of frames
  void
  estimate_num_frames()
  {
    // is stream open?
    if( !this->is_opened() )
    {
      VITAL_THROW( vital::file_not_read_exception, video_path,
                   "Video not open" );
    }

    if( !estimated_num_frames )
    {
      std::lock_guard< std::mutex > lock( open_mutex );

      auto initial_frame_number = this->frame_number();

      if( !frame_advanced && !end_of_video )
      {
        initial_frame_number = 0;
      }

      bool advance_successful = false;
      // Seek to as close as possibly to the end of the video
      int64_t frame_ts = this->f_video_stream->duration + this->f_start_time;
      do
      {
        for( auto& stream : klv_streams )
        {
          stream.reset();
        }
        auto err =
          av_seek_frame( f_format_context,f_video_stream->index, frame_ts,
                         AVSEEK_FLAG_BACKWARD );
        avcodec_flush_buffers( f_video_encoding );
        is_draining = false;

        if( err < 0 )
        {
          break;
        }

        advance_successful = advance();

        // Continue to make seek request further back until we land at a valid
        // frame
        frame_ts -= f_backstep_size * stream_time_base_to_frame();
      } while( !advance_successful );

      LOG_DEBUG( this->logger,
                 "Seeked to near end frame: " << this->frame_number() );

      // Step through the end of the video to find the last valid frame number
      do
      {
        number_of_frames = this->frame_number();
      } while( this->advance() );
      // The number of frames is one greater than the last valid frame number
      ++number_of_frames;

      LOG_DEBUG( this->logger,
                 "Found " << number_of_frames << " video frames" );

      // Set this flag so we do not have a count frames next time
      estimated_num_frames = true;

      // Return the video to its state before seeking to the end
      if( initial_frame_number == 0 )
      {
        // Close and reopen to reset
        this->close();
        this->open( video_path );
      }
      else
      {
        this->seek( initial_frame_number );
      }
    }
  }
}; // end of internal class.

// static open interlocking mutex
std::mutex ffmpeg_video_input::priv::open_mutex;

// ----------------------------------------------------------------------------
ffmpeg_video_input
::ffmpeg_video_input()
  : d( new priv() )
{
  attach_logger( "ffmpeg_video_input" ); // get appropriate logger
  d->logger = this->logger();

  this->set_capability( vital::algo::video_input::HAS_EOV, true );
  this->set_capability( vital::algo::video_input::HAS_FRAME_NUMBERS, true );
  this->set_capability( vital::algo::video_input::HAS_FRAME_DATA, true );
  this->set_capability( vital::algo::video_input::HAS_METADATA, false );

  this->set_capability( vital::algo::video_input::HAS_FRAME_TIME, false );
  this->set_capability( vital::algo::video_input::HAS_ABSOLUTE_FRAME_TIME,
                        false );
  this->set_capability( vital::algo::video_input::HAS_TIMEOUT, false );
  this->set_capability( vital::algo::video_input::IS_SEEKABLE, true );

  ffmpeg_init();
}

ffmpeg_video_input
::~ffmpeg_video_input()
{
  this->close();
}

// ----------------------------------------------------------------------------
// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
ffmpeg_video_input
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config =
    vital::algo::video_input::get_configuration();

  config->set_value(
    "filter_desc", d->filter_desc,
    "A string describing the libavfilter pipeline to apply when reading "
    "the video.  Only filters that operate on each frame independently "
    "will currently work.  The default \"yadif=deint=1\" filter applies "
    "deinterlacing only to frames which are interlaced.  "
    "See details at https://ffmpeg.org/ffmpeg-filters.html" );

  config->set_value(
    "sync_metadata", d->sync_metadata,
    "When set to true will attempt to synchronize the metadata by "
    "caching metadata packets whose timestamp is greater than the "
    "current frame's timestamp until a frame is reached with timestamp "
    "that is equal or greater than the metadata's timestamp." );

  config->set_value(
    "use_misp_timestamps", d->use_misp_timestamps,
    "When set to true, will attempt to use correlate KLV packet data to "
    "frames using the MISP timestamps embedding in the frame packets. This is "
    "technically the correct way to decode KLV, but the frame timestamps are "
    "wrongly encoded so often in real-world data that it is turned off by "
    "default. When turned off, the frame timestamps are emulated by looking "
    "at the KLV packets near each frame." );

  config->set_value(
    "smooth_klv_packets", d->smooth_klv_packets,
    "When set to true, will output 'smoothed' KLV packets: one packet for each "
    "standard for each frame with the current value of every existing tag. "
    "Otherwise, will report packets as they appear in the source video." );

  return config;
}

// ----------------------------------------------------------------------------
// Set this algorithm's properties via a config block
void
ffmpeg_video_input
::set_configuration( vital::config_block_sptr in_config )
{
  // Starting with our generated vital::config_block to ensure that assumed
  // values are present
  // An alternative is to check for key presence before performing a
  // get_value() call.

  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  d->filter_desc = config->get_value< std::string >( "filter_desc",
                                                     d->filter_desc );

  d->use_misp_timestamps = config->get_value< bool >( "use_misp_timestamps",
                                                      d->use_misp_timestamps );

  d->sync_metadata = config->get_value< bool >( "sync_metadata",
                                                d->sync_metadata );

  d->smooth_klv_packets = config->get_value< bool >( "smooth_klv_packets",
                                                     d->smooth_klv_packets );
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  bool retcode( true ); // assume success

  return retcode;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input
::open( std::string video_name )
{
  // Close any currently opened file
  this->close();

  d->video_path = video_name;

  {
    std::lock_guard< std::mutex > lock( d->open_mutex );

    if( !kwiversys::SystemTools::FileExists( d->video_path ) )
    {
      // Throw exception
      VITAL_THROW( kwiver::vital::file_not_found_exception, video_name,
                   "File not found" );
    }

    if( !d->open( video_name ) )
    {
      VITAL_THROW( kwiver::vital::video_runtime_exception,
                   "Video stream open failed for unknown reasons" );
    }
    this->set_capability( vital::algo::video_input::HAS_METADATA,
                          !d->klv_streams.empty() );
    d->end_of_video = false;
  }
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input
::close()
{
  d->close();

  d->video_path = "";
  d->frame_advanced = false;
  d->end_of_video = true;
  d->number_of_frames = 0;
  d->collected_all_metadata = false;
  d->estimated_num_frames = false;
  d->klv_streams.clear();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::next_frame( kwiver::vital::timestamp& ts,
              VITAL_UNUSED uint32_t timeout )
{
  if( !d->is_opened() )
  {
    VITAL_THROW( vital::file_not_read_exception, d->video_path,
                 "Video not open" );
  }

  bool ret = d->advance();

  d->end_of_video = !ret;
  if( ret )
  {
    ts = this->frame_timestamp();
  }
  return ret;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::seek_frame( kwiver::vital::timestamp& ts,
              kwiver::vital::timestamp::frame_t frame_number,
              uint32_t timeout )
{
  // Quick return if the stream isn't open.
  if( !d->is_opened() )
  {
    VITAL_THROW( vital::file_not_read_exception, d->video_path,
                 "Video not open" );
    return false;
  }
  if( frame_number <= 0 )
  {
    return false;
  }

  if( timeout != 0 )
  {
    LOG_WARN( this->logger(), "Timeout argument is not supported." );
  }

  bool ret = d->seek( frame_number );
  d->end_of_video = !ret;
  if( ret )
  {
    ts = this->frame_timestamp();
  }
  return ret;
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
ffmpeg_video_input
::frame_image()
{
  // Quick return if the stream isn't valid
  if( !d->is_valid() )
  {
    return nullptr;
  }

  AVCodecParameters* params = d->f_video_stream->codecpar;

  // If we have not already converted this frame, try to convert it
  if( !d->current_image_memory && d->f_frame->data[ 0 ] != 0 )
  {
    int width = params->width;
    int height = params->height;
    int depth = 3;
    vital::image_pixel_traits pixel_trait =
      vital::image_pixel_traits_of< unsigned char >();
    bool direct_copy;
    AVFrame* frame = d->f_frame;
    auto filtered_frame_ref =
      std::unique_ptr< AVFrame, void ( * )( AVFrame* ) >{ nullptr, nullptr };

    if( d->f_filter_src_context && d->f_filter_sink_context )
    {
      // Since we are only reading one frame at a time we need to push this
      // frame into the filter pipeline repeatedly until the same frame comes
      // out the other side.
      do
      {
        // Push the decoded frame into the filter graph
        if( av_buffersrc_add_frame_flags( d->f_filter_src_context, d->f_frame,
                                          AV_BUFFERSRC_FLAG_KEEP_REF ) < 0 )
        {
          LOG_ERROR( this->logger(), "Error while feeding the filter graph" );
          return nullptr;
        }
        // Pull a filtered frame from the filter graph
        filtered_frame_ref = { d->f_filtered_frame, &av_frame_unref };

        auto const ret = av_buffersink_get_frame( d->f_filter_sink_context,
                                                  d->f_filtered_frame );
        if( ret == AVERROR_EOF )
        {
          return nullptr;
        }
        if( ret == AVERROR( EAGAIN ) )
        {
          continue;
        }
      } while( d->f_frame->best_effort_timestamp !=
               d->f_filtered_frame->best_effort_timestamp );
      frame = d->f_filtered_frame;
    }

    AVPixelFormat pix_fmt = static_cast< AVPixelFormat >( frame->format );
    switch( pix_fmt )
    {
      case AV_PIX_FMT_GRAY8:
      {
        depth = 1;
        direct_copy = true;
        break;
      }
      case AV_PIX_FMT_RGB24:
      {
        depth = 3;
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
        pixel_trait = vital::image_pixel_traits_of< bool >();
        direct_copy = true;
        break;
      }
      default:
      {
        direct_copy = false;
      }
    }

    if( direct_copy )
    {
      int size = av_image_get_buffer_size( pix_fmt, width, height, 1 );
      d->current_image_memory =
        std::make_shared< vital::image_memory >( size );

      AVFrame picture;
      av_image_fill_arrays(
        picture.data, picture.linesize,
        static_cast< uint8_t* >( d->current_image_memory->data() ),
        pix_fmt, width, height, 1 );

      auto framedata = const_cast< const uint8_t** >( frame->data );
      av_image_copy( picture.data, picture.linesize,
                     framedata, frame->linesize,
                     pix_fmt, width, height );
    }
    else
    // If the pixel format is not recognized, convert the data into RGB_24
    {
      int size = width * height * depth;
      d->current_image_memory =
        std::make_shared< vital::image_memory >( size );

      d->f_software_context = sws_getCachedContext(
        d->f_software_context,
        width, height, pix_fmt,
        width, height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL, NULL, NULL );

      if( !d->f_software_context )
      {
        LOG_ERROR( this->logger(), "Couldn't create conversion context" );
        return nullptr;
      }

      AVFrame rgb_frame;
      av_image_fill_arrays(
        rgb_frame.data, rgb_frame.linesize,
        static_cast< uint8_t* >( d->current_image_memory->data() ),
        AV_PIX_FMT_RGB24, width, height, 1 );

      sws_scale( d->f_software_context, frame->data, frame->linesize,
                 0, height, rgb_frame.data, rgb_frame.linesize );
    }

    vital::image image(
      d->current_image_memory,
      d->current_image_memory->data(),
      width, height, depth,
      depth, depth * width, 1 );
    d->current_image =
      std::make_shared< vital::simple_image_container >( image );
  }

  return d->current_image;
}

// ----------------------------------------------------------------------------
kwiver::vital::timestamp
ffmpeg_video_input
::frame_timestamp() const
{
  if( !this->good() || d->frame_number() == static_cast< unsigned int >( -1 ) )
  {
    return {};
  }

  // We don't always have all components of a timestamp, so start with
  // an invalid TS and add the data we have.
  kwiver::vital::timestamp ts;
  ts.set_frame( d->frame_number() + d->f_frame_number_offset + 1 );

  return ts;
}

// ----------------------------------------------------------------------------
kwiver::vital::metadata_vector
ffmpeg_video_input
::frame_metadata()
{
  return d->current_metadata();
}

// ----------------------------------------------------------------------------
kwiver::vital::metadata_map_sptr
ffmpeg_video_input
::metadata_map()
{
  d->collect_all_metadata();

  return std::make_shared< kwiver::vital::simple_metadata_map >(
    d->metadata_map );
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::end_of_video() const
{
  return d->end_of_video;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::good() const
{
  return d->is_valid() && d->frame_advanced;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::seekable() const
{
  return true;
}

// ----------------------------------------------------------------------------
size_t
ffmpeg_video_input
::num_frames() const
{
  d->estimate_num_frames();

  return d->number_of_frames;
}

// ----------------------------------------------------------------------------
kwiver::vital::video_settings_uptr
ffmpeg_video_input
::implementation_settings() const
{
  if( !d->is_opened() )
  {
    return nullptr;
  }

  auto const result = new ffmpeg_video_settings{};
  result->frame_rate = d->f_video_stream->avg_frame_rate;
  result->parameters.reset( avcodec_parameters_alloc() );
  avcodec_parameters_from_context( result->parameters.get(),
                                   d->f_video_encoding );
  result->klv_stream_count = d->klv_streams.size();
  return kwiver::vital::video_settings_uptr{ result };
}


} // namespace ffmpeg

} // namespace arrows

} // end namespaces
