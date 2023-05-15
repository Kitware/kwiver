// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of merge_metadata_streams filter.

#include <arrows/core/merge_metadata_streams.h>

#include <algorithm>
#include <optional>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
merge_metadata_streams
::merge_metadata_streams()
{
  this->set_capability( CAN_USE_FRAME_IMAGE, false );
}

// ----------------------------------------------------------------------------
merge_metadata_streams
::~merge_metadata_streams()
{}

// ----------------------------------------------------------------------------
vital::config_block_sptr
merge_metadata_streams
::get_configuration() const
{
  return metadata_filter::get_configuration();
}

// ----------------------------------------------------------------------------
void
merge_metadata_streams
::set_configuration( vital::config_block_sptr )
{}

// ----------------------------------------------------------------------------
bool
merge_metadata_streams
::check_configuration( vital::config_block_sptr ) const
{
  return true;
}

// ----------------------------------------------------------------------------
vital::metadata_vector
merge_metadata_streams
::filter(
  vital::metadata_vector const& input_metadata,
  vital::image_container_scptr const& input_image )
{
  auto const result = std::make_shared< vital::metadata >();

  // Remove any null objects
  vital::metadata_vector sorted_metadata;
  for( auto const metadata : input_metadata )
  {
    if( metadata )
    {
      sorted_metadata.emplace_back( metadata );
    }
  }

  // Sort the incoming objects in order from most to least preferred.
  // Synchronous streams are preferred first, then remaining streams
  // in order of index.
  auto const tuplize =
    []( vital::metadata_sptr const& metadata ) {
      std::optional< bool > is_async;
      if( auto const entry =
            metadata->find( vital::VITAL_META_VIDEO_DATA_STREAM_SYNCHRONOUS );
          entry.is_valid() )
      {
        is_async = !entry.get< bool >();
      }

      std::optional< int > index;
      if( auto const entry =
            metadata->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX );
          entry.is_valid() )
      {
        index = entry.get< int >();
      }

      return std::make_tuple(
        !is_async.has_value(), is_async,
        !index.has_value(), index );
    };

  auto const cmp =
    [ &tuplize ]( vital::metadata_sptr const& lhs,
                  vital::metadata_sptr const& rhs ) {
      return tuplize( lhs ) < tuplize( rhs );
    };

  std::sort( sorted_metadata.begin(), sorted_metadata.end(), cmp );

  // Import each tag, pulling from preferred streams first
  for( size_t i = 0; i < vital::VITAL_META_LAST_TAG; ++i )
  {
    auto const tag = static_cast< vital::vital_metadata_tag >( i );

    // Since we're merging streams, we need to drop stream-specific tags
    if( tag == vital::VITAL_META_VIDEO_DATA_STREAM_INDEX ||
        tag == vital::VITAL_META_VIDEO_DATA_STREAM_SYNCHRONOUS )
    {
      continue;
    }

    for( auto const& metadata : sorted_metadata )
    {
      if( auto const entry = metadata->find( tag ); entry.is_valid() )
      {
        result->add( entry.tag(), entry.data() );
        break;
      }
    }
  }

  return { result };
}

} // namespace core

} // namespace arrows

} // namespace kwiver
