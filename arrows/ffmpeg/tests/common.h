// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/ffmpeg/ffmpeg_video_uninterpreted_data.h>

#include <vital/range/iota.h>

namespace ffmpeg = kwiver::arrows::ffmpeg;
namespace klv = kwiver::arrows::klv;
namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

// ----------------------------------------------------------------------------
// Verify the average difference between pixels is not too high. Some
// difference is expected due to compression artifacts, but we need to make
// sure the frame images we get out are generally the same as what we put in.
void
expect_eq_images( kv::image const& src_image,
                  kv::image const& tmp_image,
                  double epsilon )
{
  auto error = 0.0;

  ASSERT_TRUE( src_image.width() == tmp_image.width() );
  ASSERT_TRUE( src_image.height() == tmp_image.height() );
  ASSERT_TRUE( src_image.depth() == tmp_image.depth() );

  for( auto const i : kvr::iota( src_image.width() ) )
  {
    for( auto const j : kvr::iota( src_image.height() ) )
    {
      for( auto const k : kvr::iota( src_image.depth() ) )
      {
        error += std::abs(
          static_cast< double >( src_image.at< uint8_t >( i, j, k ) ) -
          static_cast< double >( tmp_image.at< uint8_t >( i, j, k ) ) );
      }
    }
  }
  error /= src_image.width() * src_image.height() * src_image.depth();

  EXPECT_LE( error, epsilon );
}

// ----------------------------------------------------------------------------
void
expect_eq_audio( kv::video_uninterpreted_data_sptr const& src_data,
                 kv::video_uninterpreted_data_sptr const& tmp_data )
{
  ASSERT_EQ( src_data == nullptr, tmp_data == nullptr );
  if( !src_data )
  {
    return;
  }

  auto const& src_packets =
    dynamic_cast< ffmpeg::ffmpeg_video_uninterpreted_data const& >( *src_data )
    .audio_packets;
  auto const& tmp_packets =
    dynamic_cast< ffmpeg::ffmpeg_video_uninterpreted_data const& >( *tmp_data )
    .audio_packets;
  ASSERT_EQ( src_packets.size(), tmp_packets.size() );

  auto src_it = src_packets.begin();
  auto tmp_it = tmp_packets.begin();
  while( src_it != src_packets.begin() && tmp_it != tmp_packets.begin() )
  {
    ASSERT_EQ( ( *src_it )->size, ( *tmp_it )->size );
    EXPECT_TRUE(
      std::equal( ( *src_it )->data, ( *src_it )->data + ( *src_it )->size,
                  ( *tmp_it )->data ) );
    ++src_it;
    ++tmp_it;
  }
}

// ----------------------------------------------------------------------------
void
expect_eq_videos(
  kv::algo::video_input& src_is, kv::algo::video_input& tmp_is,
  double image_epsilon = 0.0, kv::frame_id_t frame_offset = 0,
  kv::time_usec_t usec_offset = 0, bool allow_different_lengths = false )
{
  kv::timestamp src_ts;
  kv::timestamp tmp_ts;

  // Check each pair of frames for equality
  for( src_is.next_frame( src_ts ), tmp_is.next_frame( tmp_ts );
       !src_is.end_of_video() && !tmp_is.end_of_video();
       src_is.next_frame( src_ts ), tmp_is.next_frame( tmp_ts ) )
  {
    SCOPED_TRACE(
      std::string{ "Frame: " } +
      std::to_string( src_ts.get_frame() ) + " | " +
      std::to_string( tmp_ts.get_frame() ) );

    EXPECT_EQ( src_ts.get_frame() + frame_offset, tmp_ts.get_frame() );
    EXPECT_NEAR(
      src_ts.get_time_usec() + usec_offset, tmp_ts.get_time_usec(), 1 );

    auto const src_data = src_is.uninterpreted_frame_data();
    auto const tmp_data = tmp_is.uninterpreted_frame_data();
    expect_eq_audio( src_data, tmp_data );

    auto const src_image = src_is.frame_image()->get_image();
    auto const tmp_image = tmp_is.frame_image()->get_image();
    expect_eq_images( src_image, tmp_image, image_epsilon );
  }
  if( !allow_different_lengths )
  {
    EXPECT_TRUE( src_is.end_of_video() );
    EXPECT_TRUE( tmp_is.end_of_video() );
  }
}

// ----------------------------------------------------------------------------
void
expect_eq_videos(
  std::string const& src_path, std::string const& tmp_path,
  double image_epsilon = 0.0, kv::frame_id_t frame_offset = 0,
  kv::time_usec_t usec_offset = 0, bool allow_different_lengths = false )
{
  ASSERT_GE( frame_offset, 0 );
  ASSERT_GE( usec_offset, 0 );

  ffmpeg::ffmpeg_video_input src_is;
  ffmpeg::ffmpeg_video_input tmp_is;
  src_is.open( src_path );
  tmp_is.open( tmp_path );

  kv::timestamp ts;
  for( kv::frame_id_t i = 0; i < frame_offset; ++i )
  {
    src_is.next_frame( ts );
  }

  expect_eq_videos( src_is, tmp_is, image_epsilon );

  src_is.close();
  tmp_is.close();
}
