// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation file for video input using FFmpeg.

#include "ffmpeg_cuda.h"
#include "ffmpeg_init.h"
#include "ffmpeg_util.h"
#include "ffmpeg_video_input.h"
#include "ffmpeg_video_raw_image.h"
#include "ffmpeg_video_raw_metadata.h"
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
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
}

#include <iomanip>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <vector>

namespace kv = kwiver::vital;
namespace kva = kv::algo;
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

  klv::klv_stream_settings settings() const;

  void send_packet( AVPacket* packet );

  void advance( std::optional< uint64_t > backup_timestamp,
                int64_t max_pts, int64_t max_pos );

  void reset();

  kv::metadata_sptr vital_metadata( uint64_t timestamp, bool smooth_packets );

  AVStream* stream;
  std::vector< packet_uptr > buffer;
  std::vector< packet_uptr > this_frame_buffer;
  std::vector< uint8_t > bytes;
  std::vector< klv::klv_packet > packets;
  klv::klv_timeline timeline;
  klv::klv_demuxer demuxer;
  klv::klv_muxer muxer;
  uint64_t timeline_retention;

  constexpr static uint64_t default_timeline_retention = 5000000; // 5 seconds
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
  muxer( timeline ),
  timeline_retention{ default_timeline_retention }
{
  if( !stream )
  {
    throw std::logic_error( "ffmpeg_klv_stream given null stream" );
  }
}

// ----------------------------------------------------------------------------
klv::klv_stream_settings
ffmpeg_klv_stream
::settings() const
{
  klv::klv_stream_settings result;
  result.index = stream->index;
  switch( stream->codecpar->profile )
  {
#if LIBAVCODEC_VERSION_MAJOR > 57
    case FF_PROFILE_KLVA_SYNC:
      result.type = klv::KLV_STREAM_TYPE_SYNC;
      break;
    case FF_PROFILE_KLVA_ASYNC:
      result.type = klv::KLV_STREAM_TYPE_ASYNC;
      break;
#endif
    default:
      LOG_DEBUG(
        kv::get_logger( "klv" ),
        "Could not determine synchronicity of KLV stream " << stream->index
        << "; reporting as asynchronous" );
      result.type = klv::KLV_STREAM_TYPE_ASYNC;
      break;
  }
  return result;
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

#if LIBAVCODEC_VERSION_MAJOR > 57
  // Fill in KLV profile (if not previously determined) by looking at packet
  // MPEG-TS stream identifier
  if( stream->codecpar->profile < 0 )
  {
#if LIBAVCODEC_VERSION_MAJOR > 58
    size_t length = 0;
#else
    int length = 0;
#endif
    auto const stream_id =
      av_packet_get_side_data( packet, AV_PKT_DATA_MPEGTS_STREAM_ID, &length );
    if( length )
    {
      switch( *stream_id )
      {
        case 0xBD:
          stream->codecpar->profile = FF_PROFILE_KLVA_ASYNC;
          break;
        case 0xFC:
          stream->codecpar->profile = FF_PROFILE_KLVA_SYNC;
          break;
        default:
          break;
      }
    }
  }
#endif

  packet_uptr packet_ref{ av_packet_alloc() };
  throw_error_null( packet_ref.get(), "Failed to allocate packet" );
  throw_error_code(
    av_packet_ref( packet_ref.get(), packet ),
    "Failed to create packet reference" );
  buffer.emplace_back( std::move( packet_ref ) );
}

// ----------------------------------------------------------------------------
void
ffmpeg_klv_stream
::advance(
  std::optional< uint64_t > backup_timestamp, int64_t max_pts, int64_t max_pos )
{
  this_frame_buffer.clear();
  packets.clear();

  for( auto it = buffer.begin(); it != buffer.end(); )
  {
    auto const& packet = **it;
    if( ( packet.pts != AV_NOPTS_VALUE && packet.pts <= max_pts ) ||
        ( packet.pts == AV_NOPTS_VALUE && packet.pos <= max_pos ) )
    {
      bytes.insert( bytes.end(), packet.data, packet.data + packet.size );
      this_frame_buffer.emplace_back( std::move( *it ) );
      it = buffer.erase( it );
    }
    else
    {
      ++it;
    }
  }

  auto it = &*bytes.cbegin();
  while( it != &*bytes.cend() )
  {
    try
    {
      auto const length =
        static_cast< size_t >( std::distance( it, &*bytes.cend() ) );
      packets.emplace_back( klv::klv_read_packet( it, length ) );
    }
    catch( kv::metadata_buffer_overflow const& )
    {
      // We only have part of a packet; quit until we have more data
      break;
    }
    catch( kv::metadata_exception const& e )
    {
      LOG_ERROR( kv::get_logger( "klv" ),
        "Error while parsing KLV packet: " << e.what() );
      it = &*bytes.cend();
    }
  }

  // Weirdness here to get around CentOS compiler bug
  bytes.erase( bytes.begin(),
               bytes.begin() + std::distance( &*bytes.cbegin(), it ) );

  if( packets.empty() )
  {
    return;
  }

  demuxer.send_frame( packets, backup_timestamp );

  // Remove old data
  auto const timestamp = demuxer.frame_time();
  for( auto& entry : timeline )
  {
    entry.second.erase( {
      timestamp - std::min( timeline_retention, timestamp ),
      timestamp } );
  }
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
    klv_result.klv() = muxer.receive_frame();
  }
  else
  {
    klv_result.klv() = packets;
  }
  klv_result.add< kv::VITAL_META_METADATA_ORIGIN >( "KLV" );
  klv_result.add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( stream->index );

#if LIBAVCODEC_VERSION_MAJOR > 57
  if( stream->codecpar->profile >= 0 )
  {
    klv_result.add< kv::VITAL_META_VIDEO_DATA_STREAM_SYNCHRONOUS >(
      stream->codecpar->profile == FF_PROFILE_KLVA_SYNC
    );
  }
#endif

  return result;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
// Private implementation class
class ffmpeg_video_input::priv
{
public:
  struct frame_state;
  struct open_video_state;

  struct frame_state {
    frame_state( open_video_state& parent );
    frame_state( frame_state const& ) = delete;
    frame_state( frame_state&& ) = default;
    ~frame_state();

    frame_state& operator=( frame_state const& ) = delete;
    frame_state& operator=( frame_state&& ) = default;

    kv::image_container_sptr convert_image();
    kv::metadata_vector const& convert_metadata();

    ffmpeg_video_raw_image& get_raw_image();
    ffmpeg_video_raw_metadata& get_raw_metadata();

    open_video_state* parent;
    kv::logger_handle_t logger;

    frame_uptr frame;
    frame_uptr processed_frame;

    kv::image_memory_sptr image_memory;
    kv::image_container_sptr image;
    kv::video_raw_image_sptr raw_image;

    std::optional< kv::metadata_vector > metadata;
    kv::video_raw_metadata_sptr raw_metadata;

    bool is_draining;
  };

  struct open_video_state {
    struct filter_parameters {
      explicit filter_parameters( AVCodecContext const& codec_context );
      explicit filter_parameters( AVFrame const& frame );

      bool operator==( filter_parameters const& other ) const;
      bool operator!=( filter_parameters const& other ) const;

      int width;
      int height;
      AVPixelFormat pix_fmt;
      AVRational sample_aspect_ratio;
    };

    open_video_state( priv& parent, std::string const& path );
    ~open_video_state();

    bool try_codec();
    void init_filters( filter_parameters const& parameters );
    bool advance();
    void seek( kv::frame_id_t frame_number );
    void set_video_metadata( kv::metadata& md );
    double curr_time() const;
    double duration() const;
    AVRational frame_rate() const;
    size_t num_frames() const;
    kv::frame_id_t frame_number() const;
    kv::timestamp timestamp() const;
    kv::video_settings_uptr implementation_settings() const;

    priv* parent;
    kv::logger_handle_t logger;

    std::string path;

    format_context_uptr format_context;
    codec_context_uptr codec_context;
    AVCodec const* codec;

    AVStream* video_stream;

    filter_graph_uptr filter_graph;
    AVFilterContext* filter_sink_context;
    AVFilterContext* filter_source_context;
    std::optional< filter_parameters > filter_params;

    sws_context_uptr image_conversion_context;

    int64_t start_ts;
    std::map< int64_t, klv::misp_timestamp > pts_to_misp_ts;
    std::map< int64_t, int64_t > packet_pos_to_dts;
    int64_t prev_frame_dts;
    int64_t prev_video_dts;
    std::multimap< int64_t, packet_uptr > lookahead;

    std::list< ffmpeg_klv_stream > klv_streams;
    kv::metadata_map_sptr all_metadata;

    std::optional< frame_state > frame;

    bool lookahead_at_eof;
    bool at_eof;
  };

  ffmpeg_video_input& parent;
  kv::logger_handle_t logger;

  hardware_device_context_uptr hardware_device_context;

  bool klv_enabled;
  bool use_misp_timestamps;
  bool smooth_klv_packets;
  std::string unknown_stream_behavior;
  std::string filter_description;
  uint64_t retain_klv_duration;
  bool cuda_enabled;
  int cuda_device_index;

  std::optional< open_video_state > video;

  priv( ffmpeg_video_input& parent );
  ~priv();

  bool is_open() const;
  void assert_open( std::string const& fn_name ) const;
  bool is_valid() const;
  void open( std::string const& path );
  void close();

  void hardware_init();
  void cuda_init();

  AVHWDeviceContext* hardware_device() const;
#ifdef KWIVER_ENABLE_FFMPEG_CUDA
  AVCUDADeviceContext* cuda_device() const;
#endif
};

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv
::priv( ffmpeg_video_input& parent )
  : parent( parent ),
    logger{ kv::get_logger( "ffmpeg_video_input" ) },
    hardware_device_context{ nullptr },
    klv_enabled{ true },
    use_misp_timestamps{ false },
    smooth_klv_packets{ false },
    unknown_stream_behavior{ "klv" },
    filter_description{ "yadif=deint=1" },
    retain_klv_duration{ ffmpeg_klv_stream::default_timeline_retention },
#ifdef KWIVER_ENABLE_FFMPEG_CUDA
    cuda_enabled{ true },
#else
    cuda_enabled{ false },
#endif
    cuda_device_index{ 0 },
    video{}
{}

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv
::~priv()
{}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input::priv
::is_open() const
{
  return video.has_value();
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv
::assert_open( std::string const& fn_name ) const
{
  if( !is_open() )
  {
    VITAL_THROW(
      kv::file_not_read_exception, "<unknown file>",
      "Function " + fn_name + " called before successful open()" );
  }
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input::priv
::is_valid() const
{
  return is_open() && video->frame;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv
::open( std::string const& path )
{
  hardware_init();
  video.emplace( *this, path );
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv
::close()
{
  video.reset();
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv
::hardware_init()
{
  if( !hardware_device_context && cuda_enabled )
  {
    try
    {
      cuda_init();
    }
    catch( std::exception const& e )
    {
      LOG_ERROR( logger, "CUDA initialization failed: " << e.what() );
    }
  }
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv
::cuda_init()
{
#ifdef KWIVER_ENABLE_FFMPEG_CUDA
  hardware_device_context =
    std::move( cuda_create_context( cuda_device_index ) );
#else
  LOG_DEBUG(
    logger,
    "Could not initialize CUDA: Not compiled with KWIVER_ENABLE_CUDA" );
#endif
}

// ----------------------------------------------------------------------------
AVHWDeviceContext*
ffmpeg_video_input::priv
::hardware_device() const
{
  if( !hardware_device_context )
  {
    return nullptr;
  }
  return reinterpret_cast< AVHWDeviceContext* >(
    hardware_device_context->data );
}

// ----------------------------------------------------------------------------
#ifdef KWIVER_ENABLE_FFMPEG_CUDA
AVCUDADeviceContext*
ffmpeg_video_input::priv
::cuda_device() const
{
  if( !hardware_device() ||
      hardware_device()->type != AV_HWDEVICE_TYPE_CUDA )
  {
    return nullptr;
  }
  return static_cast< AVCUDADeviceContext* >( hardware_device()->hwctx );
}
#endif

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv::frame_state
::frame_state( open_video_state& parent )
  : parent{ &parent },
    logger{ parent.logger },
    frame{},
    processed_frame{},
    image_memory{},
    image{},
    raw_image{},
    metadata{},
    raw_metadata{},
    is_draining{ false }
{
  // Allocate frame containers
  frame.reset(
    throw_error_null( av_frame_alloc(), "Could not allocate frame" ) );
  processed_frame.reset(
    throw_error_null( av_frame_alloc(), "Could not allocate frame" ) );

  // Allocate raw data containers
  raw_image.reset( new ffmpeg_video_raw_image{} );
  raw_metadata.reset( new ffmpeg_video_raw_metadata{} );
}

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv::frame_state
::~frame_state()
{}

// ----------------------------------------------------------------------------
kv::image_container_sptr
ffmpeg_video_input::priv::frame_state
::convert_image()
{
  if( image )
  {
    return image;
  }

  // Transfer frame data from hardware device
  if( frame->hw_frames_ctx )
  {
    throw_error_code(
      av_hwframe_transfer_data( processed_frame.get(), frame.get(), 0 ),
      "Could not read frame data from hardware device" );
    throw_error_code(
      av_frame_copy_props( processed_frame.get(), frame.get() ),
      "Could not copy frame properties" );
    av_frame_unref( frame.get() );
    av_frame_move_ref( frame.get(), processed_frame.get() );
  }

  // Run the frame through the filter graph
  if( parent->filter_source_context && parent->filter_sink_context )
  {
    // Check for parameter changes
    open_video_state::filter_parameters const frame_params{ *frame };
    if( frame_params != *parent->filter_params )
    {
      parent->init_filters( frame_params );
    }

    int recv_err;
    do{
      throw_error_code(
        av_buffersrc_add_frame_flags(
          parent->filter_source_context, frame.get(),
          AV_BUFFERSRC_FLAG_KEEP_REF ),
        "Could not feed frame to filter graph" );

      av_frame_unref( processed_frame.get() );
      recv_err =
        av_buffersink_get_frame(
          parent->filter_sink_context, processed_frame.get() );

      if( recv_err == AVERROR_EOF )
      {
        return nullptr;
      }
      if( recv_err == AVERROR( EAGAIN ) )
      {
        continue;
      }
      throw_error_code( recv_err, "Could not read frame from filter graph" );
    } while(
      recv_err == AVERROR( EAGAIN ) ||
      processed_frame->best_effort_timestamp != frame->best_effort_timestamp );
    av_frame_unref( frame.get() );
    av_frame_move_ref( frame.get(), processed_frame.get() );
  }

  // Determine pixel formats
  auto const src_pix_fmt = static_cast< AVPixelFormat >( frame->format );
  // TODO: Detect and support grayscale, alpha, binary
  auto const dst_pix_fmt = AV_PIX_FMT_RGB24;
  size_t const depth = 3;

  // Determine image dimensions
  auto const params = parent->video_stream->codecpar;
  auto const width = static_cast< size_t >( params->width );
  auto const height = static_cast< size_t >( params->height );
  auto const image_size = width * height * depth + AV_INPUT_BUFFER_PADDING_SIZE;

  // Allocate enough space for the output image
  if( !image_memory || image_memory->size() < image_size )
  {
    image_memory = std::make_shared< kv::image_memory >( image_size );
  }

  // Get image converter
  parent->image_conversion_context.reset(
    throw_error_null(
      sws_getCachedContext(
        parent->image_conversion_context.release(),
        width, height, src_pix_fmt,
        width, height, dst_pix_fmt,
        SWS_BICUBIC, nullptr, nullptr, nullptr ),
      "Could not create image conversion context" ) );

  // Setup frame to receive converted image
  processed_frame->width = width;
  processed_frame->height = height;
  processed_frame->format = dst_pix_fmt;
  processed_frame->data[ 0 ] = static_cast< uint8_t* >( image_memory->data() );
  processed_frame->linesize[ 0 ] = width * depth;

  // Convert pixel format
  throw_error_code(
    sws_scale(
      parent->image_conversion_context.get(),
      frame->data, frame->linesize,
      0, height,
      processed_frame->data, processed_frame->linesize ) );

  // Clear frame structure
  av_frame_unref( processed_frame.get() );

  // Package up and return result
  return image = std::make_shared< kv::simple_image_container >(
    kv::image(
      image_memory, image_memory->data(),
      width, height, depth,
      depth, depth * width, 1 ) );
}

// ----------------------------------------------------------------------------
kv::metadata_vector const&
ffmpeg_video_input::priv::frame_state
::convert_metadata()
{
  if( metadata.has_value() )
  {
    return *metadata;
  }
  metadata.emplace();


  if( parent->parent->klv_enabled )
  {
    // Find MISP timestamp for this frame
    uint64_t misp_timestamp = 0;
    if( parent->parent->use_misp_timestamps )
    {
      auto const it =
        parent->pts_to_misp_ts.find( frame->best_effort_timestamp );
      if( it != parent->pts_to_misp_ts.end() )
      {
        misp_timestamp = it->second.microseconds().count();
      }
      else
      {
        LOG_ERROR( logger,
          "No MISP timestamp found for frame " << parent->frame_number() );
      }
    }

    // Add one metadata packet per KLV stream
    for( auto& stream : parent->klv_streams )
    {
      auto const timestamp =
        misp_timestamp ? misp_timestamp : stream.demuxer.frame_time();
      auto stream_metadata =
        stream.vital_metadata( timestamp, parent->parent->smooth_klv_packets );
      stream_metadata->add< kv::VITAL_META_UNIX_TIMESTAMP_SOURCE >(
        misp_timestamp ? "misp" : "klv" );

      parent->set_video_metadata( *stream_metadata );
      metadata->emplace_back( std::move( stream_metadata ) );
    }
  }

  // If there are no metadata streams, add a packet with just video metadata
  if( metadata->empty() )
  {
    auto video_metadata = std::make_shared< kv::metadata >();
    parent->set_video_metadata( *video_metadata );
    metadata->emplace_back( std::move( video_metadata ) );
  }

  return *metadata;
}

// ----------------------------------------------------------------------------
ffmpeg_video_raw_image&
ffmpeg_video_input::priv::frame_state
::get_raw_image()
{
  return dynamic_cast< ffmpeg_video_raw_image& >( *raw_image );
}

// ----------------------------------------------------------------------------
ffmpeg_video_raw_metadata&
ffmpeg_video_input::priv::frame_state
::get_raw_metadata()
{
  return dynamic_cast< ffmpeg_video_raw_metadata& >( *raw_metadata );
}

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv::open_video_state::filter_parameters
::filter_parameters( AVCodecContext const& codec_context )
  : width{ codec_context.width },
    height{ codec_context.height },
    pix_fmt{
      codec_context.hw_device_ctx
      ? codec_context.sw_pix_fmt
      : codec_context.pix_fmt },
    sample_aspect_ratio{ codec_context.sample_aspect_ratio }
{}

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv::open_video_state::filter_parameters
::filter_parameters( AVFrame const& frame )
  : width{ frame.width },
    height{ frame.height },
    pix_fmt{ static_cast< AVPixelFormat >( frame.format ) },
    sample_aspect_ratio{ frame.sample_aspect_ratio }
{}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input::priv::open_video_state::filter_parameters
::operator==( filter_parameters const& other ) const
{
  return
    width == other.width &&
    height == other.height &&
    pix_fmt == other.pix_fmt &&
    sample_aspect_ratio.num == other.sample_aspect_ratio.num &&
    sample_aspect_ratio.den == other.sample_aspect_ratio.den;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input::priv::open_video_state::filter_parameters
::operator!=( filter_parameters const& other ) const
{
  return !( *this == other );
}

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv::open_video_state
::open_video_state( priv& parent, std::string const& path )
  : parent{ &parent },
    logger{ parent.logger },
    path{ path },
    format_context{ nullptr },
    codec_context{ nullptr },
    codec{ nullptr },
    video_stream{ nullptr },
    filter_graph{ nullptr },
    filter_sink_context{ nullptr },
    filter_source_context{ nullptr },
    image_conversion_context{ nullptr },
    start_ts{ 0 },
    pts_to_misp_ts{},
    packet_pos_to_dts{},
    prev_frame_dts{ AV_NOPTS_VALUE },
    prev_video_dts{ AV_NOPTS_VALUE },
    lookahead{},
    klv_streams{},
    all_metadata{ nullptr },
    frame{},
    lookahead_at_eof{ false },
    at_eof{ false }
{
// Try to probe the file for stream information
  constexpr size_t max_probe_tries = 4;
  int64_t probesize = 5'000'000; // 5 MB
  int64_t max_analyze_duration = 10 * AV_TIME_BASE; // 10 seconds
  uint64_t increase_factor = 100;
  for( auto const i : kvr::iota( max_probe_tries ) )
  {
    // Open the file
    {
      AVFormatContext* ptr = nullptr;
      throw_error_code(
        avformat_open_input( &ptr, path.c_str(), NULL, NULL ),
        "Could not open input stream" );
      format_context.reset( ptr );
    }
    format_context->probesize = probesize;
    format_context->max_analyze_duration = max_analyze_duration;

    // Get the stream information by reading a bit of the file
    throw_error_code(
      avformat_find_stream_info( format_context.get(), NULL ),
      "Could not read stream information" );

    // Find a video stream, and optionally any data streams
    for( auto const j : kvr::iota( format_context->nb_streams ) )
    {
      auto const stream = format_context->streams[ j ];
      auto const params = stream->codecpar;
      if( params->codec_type == AVMEDIA_TYPE_VIDEO )
      {
        if( params->width <= 0 || params->height <= 0 )
        {
          LOG_ERROR(
            logger,
            "FFmpeg cannot determine the characteristics of video stream "
            << stream->index << "; this stream will be ignored" );
          continue;
        }
        if( video_stream )
        {
          LOG_WARN(
            logger,
            "Multiple video streams are not currently supported; stream "
            << stream->index << " will be ignored" );
          continue;
        }
        video_stream = stream;
      }
      else if( parent.klv_enabled &&
              params->codec_id == AV_CODEC_ID_SMPTE_KLV )
      {
        klv_streams.emplace_back( stream );
      }
      else if( parent.klv_enabled &&
              params->codec_id == AV_CODEC_ID_NONE )
      {
        if( ( params->codec_type == AVMEDIA_TYPE_DATA ||
              params->codec_type == AVMEDIA_TYPE_UNKNOWN ) &&
            parent.unknown_stream_behavior == "klv" )
        {
          LOG_INFO( logger,
            "Treating unknown stream " << stream->index << " as KLV" );
          klv_streams.emplace_back( stream );
        }
        else
        {
          LOG_INFO( logger, "Ignoring unknown stream " << stream->index );
        }
      }
    }

    if( video_stream )
    {
      // Success!
      break;
    }

    // Increase how much of file to analyze on later attempts
    LOG_ERROR(
      logger,
      "Could not find a valid video stream in the input on attempt "
      << ( i + 1 ) << " of " << max_probe_tries );
    probesize *= increase_factor;
    max_analyze_duration *= increase_factor;

    // Clear state
    klv_streams.clear();
  }

  // Confirm stream characteristics
  throw_error_null( video_stream,
    "Could not find a valid video stream in the input" );
  av_dump_format(
    format_context.get(), video_stream->index, path.c_str(), 0 );

  if( parent.klv_enabled )
  {
    LOG_INFO( logger, "Found " << klv_streams.size() << " KLV stream(s)" );
    for( auto& klv_stream : klv_streams )
    {
      klv_stream.timeline_retention = parent.retain_klv_duration;
    }
  }

  // Dig up information about the video's codec
  auto const video_params = video_stream->codecpar;
  auto const codec_id = video_params->codec_id;
  LOG_INFO(
    logger, "Video requires codec type: " << pretty_codec_name( codec_id ) );

  // Codec prioritization scheme:
  // (1) Choose hardware over software codecs
  auto const codec_cmp =
    [ & ]( AVCodec const* lhs, AVCodec const* rhs ) -> bool {
      return
        std::make_tuple( is_hardware_codec( lhs ) ) >
        std::make_tuple( is_hardware_codec( rhs ) );
    };
  std::multiset<
    AVCodec const*, std::function< bool( AVCodec const*, AVCodec const* ) > >
  possible_codecs{ codec_cmp };

  // Find all compatible CUDA codecs
#ifdef KWIVER_ENABLE_FFMPEG_CUDA
  if( parent.cuda_device() )
  {
    auto const cuda_codecs = cuda_find_decoders( *video_params );
    possible_codecs.insert( cuda_codecs.begin(), cuda_codecs.end() );
  }
#endif

  // Find all compatible software codecs
  AVCodec const* codec_ptr = nullptr;
#if LIBAVCODEC_VERSION_MAJOR > 57
  for( void* it = nullptr; ( codec_ptr = av_codec_iterate( &it ) ); )
#else
  while( ( codec_ptr = av_codec_next( codec_ptr ) ) )
#endif
  {
    if( codec_ptr->id == codec_id &&
        av_codec_is_decoder( codec_ptr ) &&
        !is_hardware_codec( codec_ptr ) &&
        !( codec_ptr->capabilities & AV_CODEC_CAP_EXPERIMENTAL ) )
    {
      possible_codecs.emplace( codec_ptr );
    }
  }

  // Find the first compatible codec that works, in priority order
  for( auto const possible_codec : possible_codecs )
  {
    codec = possible_codec;
    if( try_codec() )
    {
      break;
    }
    else
    {
      codec = nullptr;
    }
  }

  throw_error_null(
    codec,
    "Could not open video with any known input codec. ",
    possible_codecs.size(), " codecs were tried. ",
    "Required codec type: ", pretty_codec_name( codec_id ) );
  LOG_INFO(
    logger, "Successfully loaded codec: " << pretty_codec_name( codec ) );
}

// ----------------------------------------------------------------------------
ffmpeg_video_input::priv::open_video_state
::~open_video_state()
{}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input::priv::open_video_state
::try_codec()
{
  LOG_TRACE(
    parent->logger, "Trying input codec: " << pretty_codec_name( codec ) );

  // Allocate context
  codec_context.reset(
    throw_error_null(
      avcodec_alloc_context3( codec ),
      "Could not allocate context for input codec: ",
      pretty_codec_name( codec ) ) );

  // Fill in context
  throw_error_code(
    avcodec_parameters_to_context(
      codec_context.get(), video_stream->codecpar ),
    "Could not fill parameters for input codec: ",
    pretty_codec_name( codec ) );

  if( is_hardware_codec( codec ) )
  {
    codec_context->hw_device_ctx =
      av_buffer_ref( parent->hardware_device_context.get() );
  }

  codec_context->thread_count = 0;
  codec_context->thread_type = FF_THREAD_FRAME;

  // Open codec
  auto const err = avcodec_open2( codec_context.get(), codec, NULL );
  if( err < 0 )
  {
    LOG_WARN(
      parent->logger,
      "Could not open input codec: " << pretty_codec_name( codec ) << ": "
      << error_string( err ) );
    return false;
  }

  // Initialize filter graph
  init_filters( filter_parameters{ *codec_context } );

  // Read frames until we can successfully decode one to get start timestamp
  {
    packet_uptr tmp_packet{
      throw_error_null(
        av_packet_alloc(), "Could not allocate packet memory" ) };
    frame_uptr tmp_frame{
      throw_error_null(
        av_frame_alloc(), "Could not allocate frame memory" ) };
    int send_err;
    int recv_err;
    do {
      throw_error_code(
        av_read_frame( format_context.get(), tmp_packet.get() ),
        "Could not read frame" );

      send_err = avcodec_send_packet( codec_context.get(), tmp_packet.get() );
      recv_err = avcodec_receive_frame( codec_context.get(), tmp_frame.get() );
      if( recv_err != AVERROR_EOF && recv_err != AVERROR( EAGAIN ) )
      {
        throw_error_code( recv_err, "Could not read frame from decoder" );
      }
      if( send_err < 0 &&
          send_err != AVERROR( EAGAIN ) &&
          send_err != AVERROR_INVALIDDATA )
      {
        // There's something wrong with the codec setup; try a different one
        LOG_WARN(
          parent->logger, "Could not read beginning of video with codec "
          << pretty_codec_name( codec ) << ": " << error_string( send_err ) );
        return false;
      }
      av_packet_unref( tmp_packet.get() );
    } while( send_err || recv_err );
    start_ts = tmp_frame->best_effort_timestamp;
  }

  // Seek back to start
  auto seek_err =
    av_seek_frame(
      format_context.get(), -1, INT64_MIN,
      AVSEEK_FLAG_BYTE | AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY );
  if( seek_err < 0 )
  {
    // Sometimes seeking by byte position is not allowed, so try by timestamp
    throw_error_code(
      av_seek_frame(
        format_context.get(), -1, INT64_MIN,
        AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY ),
      "Could not seek to beginning of video" );
  }
  avcodec_flush_buffers( codec_context.get() );

  return true;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv::open_video_state
::init_filters( filter_parameters const& parameters )
{
  // Check for empty filter string
  if( std::all_of(
    parent->filter_description.begin(),
    parent->filter_description.end(), isspace ) )
  {
    return;
  }

  // Allocate filter graph
  filter_graph.reset(
    throw_error_null(
      avfilter_graph_alloc(), "Could not allocate filter graph" ) );

  // Create the input buffer
  {
    std::stringstream ss;
    ss << "video_size=" << parameters.width << "x"
                        << parameters.height
      << ":pix_fmt=" << parameters.pix_fmt
      << ":time_base=" << video_stream->time_base.num << "/"
                       << video_stream->time_base.den
      << ":pixel_aspect=" << parameters.sample_aspect_ratio.num << "/"
                          << parameters.sample_aspect_ratio.den;
    throw_error_code(
      avfilter_graph_create_filter(
        &filter_source_context, avfilter_get_by_name( "buffer" ),
        "in", ss.str().c_str(), NULL, filter_graph.get() ),
      "Could not create buffer source" );
  }

  // Create the output buffer
  throw_error_code(
    avfilter_graph_create_filter(
      &filter_sink_context, avfilter_get_by_name( "buffersink" ),
      "out", NULL, NULL, filter_graph.get() ),
    "Could not create buffer sink" );

  // Create the input node
  filter_in_out_uptr output{
    throw_error_null(
      avfilter_inout_alloc(),
      "Could not allocate filter output" ) };
  output->name = av_strdup( "in" );
  output->filter_ctx = filter_source_context;
  output->pad_idx = 0;
  output->next = NULL;

  // Create the output node
  filter_in_out_uptr input{
    throw_error_null(
      avfilter_inout_alloc(),
      "Could not allocate filter input" ) };
  input->name = av_strdup( "out" );
  input->filter_ctx = filter_sink_context;
  input->pad_idx = 0;
  input->next = NULL;

  // Parse graph
  {
    auto input_ptr = input.release();
    auto output_ptr = output.release();
    auto const err =
      avfilter_graph_parse_ptr(
        filter_graph.get(), parent->filter_description.c_str(),
        &input_ptr, &output_ptr, NULL );
    avfilter_inout_free( &input_ptr );
    avfilter_inout_free( &output_ptr );
    throw_error_code( err, "Could not parse filter graph" );
  }

  // Configure graph
  throw_error_code(
    avfilter_graph_config( filter_graph.get(), NULL ),
    "Could not configure filter graph" );

  filter_params.emplace( parameters );
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input::priv::open_video_state
::advance()
{
  if( at_eof )
  {
    return false;
  }

  // Clear old frame and create new one
  frame_state new_frame{ *this };
  if( frame.has_value() )
  {
    new_frame.image_memory = std::move( frame->image_memory );
    new_frame.is_draining = frame->is_draining;
  }
  frame.reset();

  // Run through video until we can assemble a frame image
  while( !frame.has_value() && !at_eof )
  {
    // We need at least one video packet before we could expect another frame
    auto first_video_it = lookahead.end();

    // We want to make sure each stream has caught up before actually using the
    // next video packet
    std::vector< int64_t > most_recent_dts(
      format_context->nb_streams, AV_NOPTS_VALUE );

    // Take stock of packets we have in storage
    for( auto it = lookahead.begin(); it != lookahead.end(); ++it )
    {
      most_recent_dts.at( it->second->stream_index ) =
        std::max(
          most_recent_dts.at( it->second->stream_index ), it->first );
      if( first_video_it == lookahead.end() &&
          it->second->stream_index == video_stream->index )
      {
        first_video_it = it;
      }
    }

    // Functor determining if we need to parse more of other streams before
    // continuing with decoding the video
    auto const looked_ahead_enough =
      [ this, &most_recent_dts, &first_video_it ]() -> bool
      {
        if( first_video_it == lookahead.end() )
        {
          return false;
        }

        for( auto const& stream : klv_streams )
        {
          auto const dts = most_recent_dts.at( stream.stream->index );
          if( dts == AV_NOPTS_VALUE ||
              dts <= first_video_it->first ||
              dts <= first_video_it->second->pts )
          {
            return false;
          }
        }

        return true;
      };

    // Read until all streams are up-to-date with the video stream
    packet_uptr packet{
      throw_error_null( av_packet_alloc(), "Could not allocate packet" ) };
    while( !lookahead_at_eof && !looked_ahead_enough() )
    {
      av_packet_unref( packet.get() );

      auto const read_err =
        av_read_frame( format_context.get(), packet.get() );
      if( read_err == AVERROR_EOF )
      {
        // End of input
        lookahead_at_eof = true;
        break;
      }
      throw_error_code( read_err, "Could not read next packet from file" );

      // Sanity check for stream index
      if( packet->stream_index < 0 ||
          static_cast< unsigned int >( packet->stream_index ) >=
            most_recent_dts.size() )
      {
        continue;
      }

      // Process video packet
      if( packet->stream_index == video_stream->index )
      {
        // Need pts
        if( packet->pts == AV_NOPTS_VALUE )
        {
          LOG_ERROR(
            parent->logger,
            "Dropping video packet with invalid pts" );
          continue;
        }

        // Replace any weird dts with a guess
        if( packet->dts == AV_NOPTS_VALUE )
        {
          packet->dts =
            ( prev_video_dts == AV_NOPTS_VALUE )
            ? 0
            : ( prev_video_dts + 1 );
        }
        else if(
          packet->dts == AV_NOPTS_VALUE ||
          packet->dts < prev_video_dts ||
          packet->dts > packet->pts )
        {
          if( prev_video_dts != AV_NOPTS_VALUE && packet->duration > 0 )
          {
            packet->dts = prev_video_dts + packet->duration;
            packet->dts = std::min( packet->dts, packet->pts );
          }
          else
          {
            packet->dts = packet->pts;
          }
        }
        prev_video_dts = packet->dts;
      }

      // Guess the missing DTS field for asynchronous KLV
      auto packet_dts = packet->dts;
      for( auto& stream : klv_streams )
      {
        if( packet->stream_index != stream.stream->index ||
            stream.settings().type != klv::KLV_STREAM_TYPE_ASYNC )
        {
          continue;
        }

        packet_dts =
          lookahead.empty() ? 0 : std::prev( lookahead.end() )->first;
        break;
      }

      // Put the packet in the lookahead buffer
      auto new_packet =
        throw_error_null( av_packet_alloc(), "Could not allocate packet" );
      auto const it = lookahead.emplace( packet_dts, std::move( new_packet ) );
      av_packet_move_ref( it->second.get(), packet.get() );

      // Update stats for this stream
      most_recent_dts.at( it->second->stream_index ) =
        std::max(
          packet_dts, most_recent_dts.at( it->second->stream_index ) );
      if( first_video_it == lookahead.end() &&
          it->second->stream_index == video_stream->index )
      {
        first_video_it = it;
      }
    }

    // Couldn't find next video packet? Tell the decoder to flush any remaining
    // buffered frames
    if( first_video_it == lookahead.end() &&
        lookahead_at_eof && !new_frame.is_draining )
    {
      avcodec_send_packet( codec_context.get(), nullptr );
      new_frame.is_draining = true;
    }

    // Process next video packet, if there is one
    if( first_video_it != lookahead.end() )
    {
      packet = std::move( first_video_it->second );
      lookahead.erase( first_video_it );
      first_video_it = lookahead.end();

      // Record packet as raw image
      new_frame.get_raw_image().packets.emplace_back(
        throw_error_null(
          av_packet_alloc(), "Could not allocate packet" ) );
      throw_error_code(
        av_packet_ref(
          new_frame.get_raw_image().packets.back().get(), packet.get() ),
        "Could not give packet to raw image cache" );
      packet_pos_to_dts.emplace( packet->pos, packet->dts );

      // Find MISP timestamp
      for( auto const tag_type : { klv::MISP_TIMESTAMP_TAG_STRING,
                                   klv::MISP_TIMESTAMP_TAG_UUID } )
      {
        auto it =
          klv::find_misp_timestamp(
            packet->data, packet->data + packet->size, tag_type );
        if( it != packet->data + packet->size )
        {
          auto const timestamp = klv::read_misp_timestamp( it );
          pts_to_misp_ts.emplace( packet->pts, timestamp );
          break;
        }
      }

      // Send packet to decoder
      auto const send_err =
        avcodec_send_packet( codec_context.get(), packet.get() );
      if( send_err != AVERROR_INVALIDDATA )
      {
        throw_error_code( send_err, "Decoder rejected packet" );
      }
    }

    // Receive decoded frame
    auto const recv_err =
      avcodec_receive_frame( codec_context.get(), new_frame.frame.get() );
    auto dts_lookup_failed = false;
    switch( recv_err )
    {
      case 0:
        // Success
        frame = std::move( new_frame );

        // Look up the dts of the packet that contained this frame
        if( auto const it = packet_pos_to_dts.find( frame->frame->pkt_pos );
            it != packet_pos_to_dts.end() )
        {
          frame->get_raw_image().frame_dts = it->second;
          prev_frame_dts = it->second;
          packet_pos_to_dts.erase( it );
        }
        else
        {
          LOG_DEBUG(
            parent->logger,
            "Raw frame dts lookup failed, likely due to corruption" );
          dts_lookup_failed = true;
          frame->get_raw_image().frame_dts = prev_frame_dts;

          // We can't erase an entry from packet_pos_to_dts, since we don't
          // know for sure which entry to erase
        }
        frame->get_raw_image().frame_pts = frame->frame->best_effort_timestamp;
        frame->get_raw_image().is_keyframe = frame->frame->key_frame;

        // Clean up
        if( frame->frame->key_frame && !dts_lookup_failed )
        {
          auto const it =
            packet_pos_to_dts.lower_bound( frame->frame->pkt_pos );
          if( it != packet_pos_to_dts.begin() )
          {
            auto const cleanup_count =
              std::distance( packet_pos_to_dts.begin(), it );
            LOG_DEBUG(
              parent->logger,
              "Cleaning up " << cleanup_count << " dts lookup entries" );
            packet_pos_to_dts.erase( packet_pos_to_dts.begin(), it );
          }
        }
        break;
      case AVERROR_EOF:
        // End of file
        at_eof = true;
        break;
      case AVERROR_INVALIDDATA:
      case AVERROR( EAGAIN ):
        // Acceptable errors
        break;
      default:
        // Unacceptable errors
        throw_error_code( recv_err, "Decoder returned error" );
        break;
    }
  }

  if( frame.has_value() )
  {
    // Give the KLV stream processors all packets up to this new frame image
    for( auto it = lookahead.begin(); it != lookahead.end(); )
    {
      if( it->second->pts > frame->frame->best_effort_timestamp )
      {
        ++it;
        continue;
      }

      auto found = false;
      for( auto& stream : klv_streams )
      {
        if( it->second->stream_index != stream.stream->index )
        {
          continue;
        }

        found = true;
        stream.send_packet( it->second.get() );
        it = lookahead.erase( it );
        break;
      }

      if( !found )
      {
        ++it;
      }
    }
  }

  // Advance KLV
  for( auto& stream : klv_streams )
  {
    auto const frame_delta = av_q2d( av_inv_q( frame_rate() ) );
    auto const backup_timestamp =
      stream.demuxer.frame_time() +
      static_cast< uint64_t >( frame_delta * 1000000u );
    auto max_pts = INT64_MAX;
    auto max_pos = INT64_MAX;
    if( frame.has_value() )
    {
      max_pts = frame->frame->best_effort_timestamp;
      max_pos = frame->frame->pkt_pos;
    }

    stream.advance( backup_timestamp, max_pts, max_pos );
  }

  return frame.has_value();
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv::open_video_state
::seek( kv::frame_id_t frame_number )
{
  if( frame_number == this->frame_number() )
  {
    return;
  }

  // Get to the desired frame by seeking some number of frames before it, then
  // iterating forward. If we still don't have a frame image for that frame,
  // try again by seeking even further back. Finding the last keyframe is
  // necessary to properly form an image, and some decoders will buffer even
  // more frames than that.
  auto const backstep_size =
    codec_context->gop_size ? codec_context->gop_size : 12;
  constexpr size_t maximum_attempts = 5;
  for( size_t i = 0; i < maximum_attempts; ++i )
  {
    // Clear current state
    prev_frame_dts = AV_NOPTS_VALUE;
    prev_video_dts = AV_NOPTS_VALUE;
    lookahead.clear();
    lookahead_at_eof = false;
    at_eof = false;
    frame.reset();
    for( auto& stream : klv_streams )
    {
      stream.reset();
    }

    // Increasing backstep intervals on further tries
    size_t const backstep = i ? ( ( 1 << ( i - 1 ) ) * backstep_size ) : 1;

    // Determine timestamp from frame number
    int64_t converted_timestamp = frame_number - backstep;
    converted_timestamp =
      av_rescale( converted_timestamp, frame_rate().den, frame_rate().num );
    converted_timestamp =
      av_rescale(
        converted_timestamp,
        video_stream->time_base.den, video_stream->time_base.num );
    converted_timestamp += start_ts;

    // Do the seek
    throw_error_code(
      av_seek_frame(
        format_context.get(), video_stream->index, converted_timestamp,
        AVSEEK_FLAG_BACKWARD ),
      "Could not seek to frame ", frame_number );
    if( codec_context )
    {
      avcodec_flush_buffers( codec_context.get() );
    }

    // Move forward through frames until we get to the desired frame
    do
    {
      advance();
      if( at_eof )
      {
        throw_error(
          "Could not seek to frame ", frame_number + 1, ": "
          "End of file reached" );
      }
    } while( this->frame_number() < frame_number );

    // Check for success
    if( this->frame_number() == frame_number )
    {
      break;
    }
  }

  // Check for failure
  if( this->frame_number() > frame_number )
  {
    LOG_WARN(
      kv::get_logger( "klv" ),
      "Could not seek exactly to frame " << ( frame_number + 1 ) << ": "
      "Ended up on frame " << ( this->frame_number() + 1 ) );
  }
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input::priv::open_video_state
::set_video_metadata( kv::metadata& md )
{
  // Add frame number to timestamp
  md.set_timestamp( timestamp() );

  // Add file name/uri
  md.add< kv::VITAL_META_VIDEO_URI >( path );

  // Mark whether the frame is a key frame
  md.add< kv::VITAL_META_VIDEO_KEY_FRAME >( frame->frame->key_frame > 0 );

  // Add image dimensions
  md.add< kv::VITAL_META_IMAGE_WIDTH >( frame->frame->width );
  md.add< kv::VITAL_META_IMAGE_HEIGHT >( frame->frame->height );

  // Add frame rate
  if( frame_rate().num > 0 )
  {
    md.add< kv::VITAL_META_VIDEO_FRAME_RATE >( av_q2d( frame_rate() ) );
  }

  // Add bitrate
  auto bitrate = codec_context->bit_rate;
  if( !bitrate )
  {
    bitrate = codec_context->bit_rate_tolerance;
  }
  if( bitrate )
  {
    md.add< kv::VITAL_META_VIDEO_BITRATE >( bitrate );
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
  switch( codec_context->codec_id )
  {
    case AV_CODEC_ID_MPEG2VIDEO:
    {
      compression_type = "H.262";
      auto const profile_it = h262_profiles.find( codec_context->profile );
      compression_profile =
        ( profile_it == h262_profiles.end() ) ? "Other" : profile_it->second;
      auto const level_it = h262_levels.find( codec_context->level );
      compression_level =
        ( level_it == h262_levels.end() ) ? "Other" : level_it->second;
      break;
    }
    case AV_CODEC_ID_H264:
    {
      compression_type = "H.264";
      auto const profile_it = h264_profiles.find( codec_context->profile );
      compression_profile =
        ( profile_it == h264_profiles.end() ) ? "Other" : profile_it->second;
      std::stringstream ss;
      ss << std::setprecision( 2 ) << ( codec_context->level / 10.0 );
      compression_level = ss.str();
      break;
    }
    case AV_CODEC_ID_H265:
    {
      compression_type = "H.265";
      auto const profile_it = h265_profiles.find( codec_context->profile );
      compression_profile =
        ( profile_it == h265_profiles.end() ) ? "Other" : profile_it->second;
      std::stringstream ss;
      ss << std::setprecision( 2 ) << ( codec_context->level / 30.0 );
      compression_level = ss.str();
      break;
    }
    default:
      break;
  }

  if( !compression_type.empty() )
  {
    md.add< kv::VITAL_META_VIDEO_COMPRESSION_TYPE >( compression_type );
  }

  if( !compression_profile.empty() )
  {
    md.add< kv::VITAL_META_VIDEO_COMPRESSION_PROFILE >(
      compression_profile );
  }

  if( !compression_level.empty() )
  {
    md.add< kv::VITAL_META_VIDEO_COMPRESSION_LEVEL >( compression_level );
  }
}

// ----------------------------------------------------------------------------
double
ffmpeg_video_input::priv::open_video_state
::curr_time() const
{
  if( !frame.has_value() ||
      frame->frame->best_effort_timestamp == AV_NOPTS_VALUE )
  {
    return 0.0;
  }

  return
    static_cast< double >( frame->frame->best_effort_timestamp - start_ts ) *
    av_q2d( video_stream->time_base );
}

// ----------------------------------------------------------------------------
double
ffmpeg_video_input::priv::open_video_state
::duration() const
{
  return
    ( video_stream->start_time + video_stream->duration - start_ts ) *
    av_q2d( video_stream->time_base );
}

// ----------------------------------------------------------------------------
AVRational
ffmpeg_video_input::priv::open_video_state
::frame_rate() const
{
  auto result = video_stream->avg_frame_rate;
  if( !result.num )
  {
    result = video_stream->r_frame_rate;
  }
  return result;
}

// ----------------------------------------------------------------------------
size_t
ffmpeg_video_input::priv::open_video_state
::num_frames() const
{
  return static_cast< size_t >( duration() * av_q2d( frame_rate() ) + 0.5 );
}

// ----------------------------------------------------------------------------
kv::frame_id_t
ffmpeg_video_input::priv::open_video_state
::frame_number() const
{
  if( !frame.has_value() ||
      frame->frame->best_effort_timestamp == AV_NOPTS_VALUE )
  {
    return -1;
  }

  return static_cast< kv::frame_id_t >(
    curr_time() * av_q2d( frame_rate() ) + 0.5 );
}

// ----------------------------------------------------------------------------
kv::timestamp
ffmpeg_video_input::priv::open_video_state
::timestamp() const
{
  if( !frame.has_value() )
  {
    return {};
  }
  return kv::timestamp{
    static_cast< kv::time_usec_t >( curr_time() * 1000000.0 + 0.5 ),
    frame_number() + 1 };
}

// ----------------------------------------------------------------------------
kv::video_settings_uptr
ffmpeg_video_input::priv::open_video_state
::implementation_settings() const
{
  ffmpeg_video_settings_uptr result{ new ffmpeg_video_settings{} };
  result->frame_rate = frame_rate();
  for( auto const& stream : klv_streams )
  {
    result->klv_streams.emplace_back( stream.settings() );
  }
  result->start_timestamp = format_context->start_time;

  if( codec_context )
  {
    throw_error_code(
      avcodec_parameters_from_context(
        result->parameters.get(), codec_context.get() ),
      "Could not fill codec parameters from context" );

    if( codec_context->hw_device_ctx )
    {
      result->parameters->format = codec_context->sw_pix_fmt;
    }
  }

  return std::move( result );
}

// ----------------------------------------------------------------------------
ffmpeg_video_input
::ffmpeg_video_input()
  : d( new priv( *this ) )
{
  attach_logger( "ffmpeg_video_input" );
  d->logger = logger();

  set_capability( kva::video_input::HAS_EOV, true );
  set_capability( kva::video_input::HAS_FRAME_NUMBERS, true );
  set_capability( kva::video_input::HAS_FRAME_DATA, true );
  set_capability( kva::video_input::HAS_METADATA, false );
  set_capability( kva::video_input::HAS_FRAME_TIME, false );
  set_capability( kva::video_input::HAS_ABSOLUTE_FRAME_TIME, false );
  set_capability( kva::video_input::HAS_TIMEOUT, false );
  set_capability( kva::video_input::IS_SEEKABLE, true );
  set_capability( kva::video_input::HAS_RAW_IMAGE, true );
  set_capability( kva::video_input::HAS_RAW_METADATA, true );
  set_capability( kva::video_input::HAS_UNINTERPRETED_DATA, false );

  ffmpeg_init();
}

// ----------------------------------------------------------------------------
ffmpeg_video_input
::~ffmpeg_video_input()
{
  close();
}

// ----------------------------------------------------------------------------
kv::config_block_sptr
ffmpeg_video_input
::get_configuration() const
{
  // Get base config from base class
  kv::config_block_sptr config =
    kva::video_input::get_configuration();

  config->set_value(
    "filter_desc", d->filter_description,
    "A string describing the libavfilter pipeline to apply when reading "
    "the video.  Only filters that operate on each frame independently "
    "will currently work.  The default \"yadif=deint=1\" filter applies "
    "deinterlacing only to frames which are interlaced.  "
    "See details at https://ffmpeg.org/ffmpeg-filters.html" );

  config->set_value(
    "klv_enabled", d->klv_enabled,
    "When set to false, will not attempt to process any KLV metadata found in "
    "the video file. This may be useful if only processing imagery."
  );

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

  config->set_value(
    "unknown_stream_behavior", d->unknown_stream_behavior,
    "Set to 'klv' to treat unknown streams as KLV. "
    "Set to 'ignore' to ignore unknown streams (default)." );

  config->set_value(
    "retain_klv_duration", d->retain_klv_duration,
    "Number of microseconds of past KLV to retain in case of backwards "
    "timestamp jumps. Defaults to 5000000." );

  config->set_value(
    "cuda_enabled", d->cuda_enabled,
    "When set to true, uses CUDA/CUVID to accelerate video decoding." );

  config->set_value(
    "cuda_device_index", d->cuda_device_index,
    "Integer index of the CUDA-enabled device to use for decoding. "
    "Defaults to 0." );

  return config;
}

// ----------------------------------------------------------------------------
// Set this algorithm's properties via a config block
void
ffmpeg_video_input
::set_configuration( kv::config_block_sptr in_config )
{
  if( d->is_open() )
  {
    VITAL_THROW(
      kv::video_config_exception,
      "Cannot change video configuration while video is open" );
  }

  // Starting with our generated kv::config_block to ensure that assumed
  // values are present
  // An alternative is to check for key presence before performing a
  // get_value() call.

  kv::config_block_sptr config = get_configuration();
  config->merge_config( in_config );

  d->filter_description =
    config->get_value< std::string >(
      "filter_desc", d->filter_description );

  d->klv_enabled =
    config->get_value< bool >( "klv_enabled", d->klv_enabled );

  d->use_misp_timestamps =
    config->get_value< bool >(
      "use_misp_timestamps", d->use_misp_timestamps );

  d->smooth_klv_packets =
    config->get_value< bool >(
      "smooth_klv_packets", d->smooth_klv_packets );

  d->unknown_stream_behavior =
    config->get_value< std::string >(
      "unknown_stream_behavior", d->unknown_stream_behavior );

  d->retain_klv_duration =
    config->get_value< uint64_t >(
      "retain_klv_duration", d->retain_klv_duration );

  d->cuda_enabled =
    config->get_value< bool >(
      "cuda_enabled", d->cuda_enabled );

  if( !d->cuda_enabled && d->hardware_device() &&
      d->hardware_device()->type == AV_HWDEVICE_TYPE_CUDA )
  {
    // Turn off the active CUDA instance
    d->hardware_device_context.reset();
  }

  d->cuda_device_index =
    config->get_value< int >(
      "cuda_device_index", d->cuda_device_index );
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::check_configuration( VITAL_UNUSED kv::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input
::open( std::string video_name )
{
  // Close any currently opened file
  close();

  // Ensure input file exists
  if( !kwiversys::SystemTools::FileExists( video_name ) )
  {
    VITAL_THROW( kv::file_not_found_exception, video_name, "File not found" );
  }

  // Attempt to open input file
  try
  {
    d->open( video_name );
  }
  catch( std::exception const& e )
  {
    VITAL_THROW(
      kv::video_runtime_exception,
      "Could not open FFmpeg video input `" + video_name + "`: " + e.what() );
  }

  set_capability(
    kva::video_input::HAS_METADATA, !d->video->klv_streams.empty() );
}

// ----------------------------------------------------------------------------
void
ffmpeg_video_input
::close()
{
  d->close();
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::next_frame( kv::timestamp& ts,
              VITAL_UNUSED uint32_t timeout )
{
  d->assert_open( "next_frame()" );

  if( d->video->advance() )
  {
    ts = frame_timestamp();
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::seek_frame( kv::timestamp& ts,
              kv::timestamp::frame_t frame_number,
              uint32_t timeout )
{
  d->assert_open( "seek_frame()" );

  ts = frame_timestamp();

  if( frame_number <= 0 )
  {
    LOG_ERROR( logger(),
      "seek_frame(): Given invalid frame number " << frame_number );
    return false;
  }

  if( timeout != 0 )
  {
    LOG_WARN( logger(), "seek_frame(): Timeout argument is not supported." );
  }

  try
  {
    d->video->seek( frame_number - 1 );
    ts = frame_timestamp();
    return true;
  }
  catch( std::exception const& e )
  {
    LOG_ERROR( logger(), e.what() );
    return false;
  }
}

// ----------------------------------------------------------------------------
kv::image_container_sptr
ffmpeg_video_input
::frame_image()
{
  if( !d->is_valid() )
  {
    return nullptr;
  }

  return d->video->frame->convert_image();
}

// ----------------------------------------------------------------------------
kv::video_raw_image_sptr
ffmpeg_video_input
::raw_frame_image()
{
  if( !d->is_valid() )
  {
    return nullptr;
  }

  return d->video->frame->raw_image;
}

// ----------------------------------------------------------------------------
kv::timestamp
ffmpeg_video_input
::frame_timestamp() const
{
  if( !d->is_valid() )
  {
    return {};
  }

  return d->video->timestamp();
}

// ----------------------------------------------------------------------------
kv::metadata_vector
ffmpeg_video_input
::frame_metadata()
{
  if( !d->is_valid() )
  {
    return {};
  }

  return d->video->frame->convert_metadata();
}

// ----------------------------------------------------------------------------
kv::video_raw_metadata_sptr
ffmpeg_video_input
::raw_frame_metadata()
{
  if( !d->is_valid() )
  {
    return nullptr;
  }

  for( auto& stream : d->video->klv_streams )
  {
    for( auto& packet : stream.this_frame_buffer )
    {
      ffmpeg_video_raw_metadata::packet_info info;
      info.packet = std::move( packet );
      info.stream_settings = stream.settings();
      d->video->frame->get_raw_metadata().packets
        .emplace_back( std::move( info ) );
    }
    stream.this_frame_buffer.clear();
  }
  return d->video->frame->raw_metadata;
}

// ----------------------------------------------------------------------------
kv::metadata_map_sptr
ffmpeg_video_input
::metadata_map()
{
  d->assert_open( "metadata_map()" );

  if( d->video->all_metadata )
  {
    return d->video->all_metadata;
  }

  kv::metadata_map::map_metadata_t result;
  priv::open_video_state tmp_video{ *d, d->video->path };
  while( tmp_video.advance() )
  {
    result.emplace(
      tmp_video.frame_number() + 1, tmp_video.frame->convert_metadata() );
  }

  d->video->all_metadata.reset(
    new kv::simple_metadata_map{ std::move( result ) } );
  return d->video->all_metadata;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::end_of_video() const
{
  return !d->is_open() || d->video->at_eof;
}

// ----------------------------------------------------------------------------
bool
ffmpeg_video_input
::good() const
{
  return d->is_valid();
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
  d->assert_open( "num_frames()" );

  return d->video->num_frames();
}

// ----------------------------------------------------------------------------
kv::video_settings_uptr
ffmpeg_video_input
::implementation_settings() const
{
  if( !d->is_open() )
  {
    return nullptr;
  }

  return d->video->implementation_settings();
}

} // namespace ffmpeg

} // namespace arrows

} // end namespaces
