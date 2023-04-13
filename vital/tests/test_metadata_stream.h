// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test utilities for metadata_[io]stream implementations.

#include <vital/types/metadata_stream.h>

#include <tests/test_gtest.h>

using namespace kwiver::vital;

namespace {

// ----------------------------------------------------------------------------
void
test_istream_frame(
  metadata_istream& is, frame_id_t frame, metadata_vector const& md )
{
  ASSERT_FALSE( is.at_end() );
  EXPECT_EQ( frame, is.frame_number() );
  EXPECT_EQ( md, is.metadata() );
}

// ----------------------------------------------------------------------------
void
test_istream_at_end( metadata_istream& is )
{
  // Repeat in case next_frame() changes state
  for( size_t i = 0; i < 2; ++i )
  {
    ASSERT_TRUE( is.at_end() );
    EXPECT_THROW( is.frame_number(), std::invalid_argument );
    EXPECT_THROW( is.metadata(), std::invalid_argument );
    EXPECT_FALSE( is.next_frame() );
  }
}

// ----------------------------------------------------------------------------
void
test_ostream_frame(
  metadata_ostream& os, frame_id_t frame, metadata_vector const& md )
{
  ASSERT_FALSE( os.at_end() );
  ASSERT_NO_THROW( ASSERT_TRUE( os.write_frame( frame, md ) ) );
}

// ----------------------------------------------------------------------------
void
test_ostream_at_end( metadata_ostream& os )
{
  ASSERT_TRUE( os.at_end() );
  EXPECT_THROW( os.write_frame( 1024, {} ), std::invalid_argument );
  ASSERT_TRUE( os.at_end() );
  EXPECT_NO_THROW( os.write_end() );
  ASSERT_TRUE( os.at_end() );
}

} // namespace <anonymous>
