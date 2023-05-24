// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of compare-klv applet.

#include <arrows/klv/applets/compare_klv.h>

#include <arrows/core/metadata_map_io_csv.h>
#include <arrows/core/metadata_stream_from_video.h>
#include <arrows/klv/klv_all.h>
#include <arrows/klv/klv_key_traits.h>
#include <arrows/klv/klv_metadata.h>

#include <vital/types/metadata_stream.h>
#include <vital/types/metadata_stream_from_map.h>
#include <vital/algo/metadata_map_io.h>
#include <vital/algo/video_input.h>
#include <vital/config/config_block_io.h>

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <list>

#include <cfloat>

using namespace kwiver::arrows;
namespace vital = kwiver::vital;
using md_map_t = vital::metadata_map::map_metadata_t;
using frame_t = vital::frame_id_t;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
class json_source : public vital::metadata_istream
{
public:
  json_source(
    std::filesystem::path const& filepath,
    vital::config_block_sptr const& config )
  {
    vital::algo::metadata_map_io_sptr importer;
    if( !config->has_value( "metadata_input:type" ) )
    {
      config->set_value( "metadata_input:type", "klv-json" );
    }
    if( filepath.extension() == ".zz" )
    {
      config->set_value( "metadata_input:klv-json:compress", true );
    }
    vital::algo::metadata_map_io::set_nested_algo_configuration(
      "metadata_input", config, importer );
    vital::algo::metadata_map_io::get_nested_algo_configuration(
      "metadata_input", config, importer );
    if( !importer )
    {
      std::cerr << "Invalid metadata_input configuration" << std::endl;
      exit( EXIT_FAILURE );
    }
    m_map = importer->load( filepath.string() )->metadata();
    m_is.emplace( m_map );
  }

  virtual ~json_source() = default;

  json_source( json_source const& ) = delete;
  json_source( json_source&& ) = delete;

  vital::frame_id_t
  frame_number() const override { return m_is->frame_number(); }
  vital::metadata_vector
  metadata() override { return m_is->metadata(); }
  bool
  next_frame() override { return m_is->next_frame(); }
  bool
  at_end() const override { return m_is->at_end(); }

private:
  std::optional< vital::metadata_istream_from_map > m_is;
  md_map_t m_map;
};

// ----------------------------------------------------------------------------
class video_source : public vital::metadata_istream
{
public:
  video_source(
    std::filesystem::path const& filepath,
    vital::config_block_sptr const& config )
  {
    vital::algo::video_input::set_nested_algo_configuration(
      "video_input", config, m_video );
    vital::algo::video_input::get_nested_algo_configuration(
      "video_input", config, m_video );

    if( !m_video )
    {
      std::cerr << "Invalid video_input configuration" << std::endl;
      exit( EXIT_FAILURE );
    }

    try
    {
      m_video->open( filepath.string() );
    }
    catch( vital::video_runtime_exception const& e )
    {
      std::cerr << e.what() << std::endl;
      exit( EXIT_FAILURE );
    }
    catch( vital::file_not_found_exception const& e )
    {
      std::cerr << e.what() << std::endl;
      exit( EXIT_FAILURE );
    }
    m_is.emplace( *m_video );
  }

  virtual ~video_source() = default;

  vital::frame_id_t
  frame_number() const override { return m_is->frame_number(); }
  vital::metadata_vector
  metadata() override { return m_is->metadata(); }
  bool
  next_frame() override { return m_is->next_frame(); }
  bool
  at_end() const override { return m_is->at_end(); }

private:
  std::optional< core::metadata_istream_from_video > m_is;
  vital::algo::video_input_sptr m_video;
};

// ----------------------------------------------------------------------------
std::unique_ptr< vital::metadata_istream >
create_metadata_istream(
  std::filesystem::path const& filepath,
  vital::config_block_sptr const& config )
{
  if( filepath.extension() == ".json" || filepath.extension() == ".zz" )
  {
    return std::make_unique< json_source >( filepath, config );
  }

  return std::make_unique< video_source >( filepath, config );
}

// ----------------------------------------------------------------------------
// Convenient information about a specific metadata packet and its istream
struct istream_data
{
  istream_data(
    vital::metadata_istream& is, vital::metadata_sptr const& metadata )
    : is{ is }, metadata{ metadata }, klv{ nullptr },
      stream_index{ INT_MIN }
  {
    auto const klv_metadata =
      dynamic_cast< klv::klv_metadata const* >( metadata.get() );
    if( klv_metadata )
    {
      klv = &klv_metadata->klv();
    }

    auto const entry =
      metadata->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX );
    if( entry )
    {
      stream_index = entry.get< int >();
    }
  }

  vital::metadata_istream& is;
  vital::metadata_sptr metadata;
  std::vector< klv::klv_packet > const* klv;
  int stream_index;
};

} // namespace <anonymous>

// ----------------------------------------------------------------------------
class compare_klv::impl
{
public:
  impl();

  void print_breadcrumbs() const;
  void print_difference( std::string const& message ) const;
  template < class T >
  bool print_if_neq(
    T const& lhs, T const& rhs, std::string const& message );

  template < class Key >
  bool compare(
    klv::klv_set< Key > const& lhs, klv::klv_set< Key > const& rhs,
    klv::klv_tag_traits_lookup const* traits );
  bool compare(
    klv::klv_value const& lhs, klv::klv_value const& rhs,
    klv::klv_tag_traits const* trait );
  bool compare(
    klv::klv_packet const& lhs, klv::klv_packet const& rhs );
  bool compare(
    std::vector< klv::klv_packet > const& lhs,
    std::vector< klv::klv_packet > const& rhs );

  bool could_be_paired(
    klv::klv_packet const& lhs, klv::klv_packet const& rhs ) const;

  template < class Key >
  std::vector< size_t > difference_score(
    klv::klv_set< Key > const& lhs, klv::klv_set< Key > const& rhs ) const;
  std::vector< size_t > difference_score(
    klv::klv_value const& lhs, klv::klv_value const& rhs ) const;

  // Tracks current "location" in KLV to inform user where the differences are
  std::list< std::string > breadcrumbs;
};

// ----------------------------------------------------------------------------
struct possible_pair
{
  istream_data& lhs_is;
  klv::klv_packet const& lhs_packet;
  istream_data& rhs_is;
  klv::klv_packet const& rhs_packet;
};

// ----------------------------------------------------------------------------
compare_klv::impl
::impl()
{}

// ----------------------------------------------------------------------------
void
compare_klv::impl
::print_breadcrumbs() const
{
  std::cout << "* ";

  bool first = true;
  for( auto const& entry : breadcrumbs )
  {
    first = first ? false : ( std::cout << " -> ", false );
    std::cout << entry;
  }
}

// ----------------------------------------------------------------------------
void
compare_klv::impl
::print_difference( std::string const& message ) const
{
  print_breadcrumbs();
  std::cout << std::fixed << std::setprecision( DBL_DIG + 1 ) << ": "
            << message << std::endl;
}

// ----------------------------------------------------------------------------
template < class T >
bool
compare_klv::impl
::print_if_neq( T const& lhs, T const& rhs, std::string const& message )
{
  if( lhs != rhs )
  {
    breadcrumbs.emplace_back( message );
    print_difference( "lhs and rhs differ:" );
    breadcrumbs.pop_back();
    std::cout << "  | lhs value: " << lhs << std::endl;
    std::cout << "  | rhs value: " << rhs << std::endl;
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
template < class Key >
bool
compare_klv::impl
::compare(
  klv::klv_set< Key > const& lhs, klv::klv_set< Key > const& rhs,
  klv::klv_tag_traits_lookup const* traits )
{
  using kt = klv::key_traits< Key >;

  auto const lhs_entries = lhs.fully_sorted();
  auto const rhs_entries = rhs.fully_sorted();
  auto lhs_it = lhs_entries.begin();
  auto rhs_it = rhs_entries.begin();
  std::vector< std::pair< const Key, klv::klv_value > const* > lhs_unmatched;
  std::vector< std::pair< const Key, klv::klv_value > const* > rhs_unmatched;
  bool equivalent = true;

  while( lhs_it != lhs_entries.end() && rhs_it != rhs_entries.end() )
  {
    auto const& lhs_entry = **lhs_it;
    auto const& rhs_entry = **rhs_it;

    // Take traits of whichever tag is lesser, since we are iterating in order
    // from lesser to greater
    auto const& prior_entry =
      ( rhs_entry.first < lhs_entry.first ) ? rhs_entry : lhs_entry;
    klv::klv_tag_traits const* trait = nullptr;
    if( traits )
    {
      trait = &kt::tag_traits_from_key( *traits, prior_entry.first );
    }

    if( lhs_entry.first == rhs_entry.first )
    {
      // Keys are equal; compare values
      equivalent &= compare( lhs_entry.second, rhs_entry.second, trait );
      ++lhs_it;
      ++rhs_it;
    }
    else if( lhs_entry.first < rhs_entry.first )
    {
      // Rhs is missing this key
      lhs_unmatched.emplace_back( &lhs_entry );
      ++lhs_it;
    }
    else
    {
      // Lhs is missing this key
      rhs_unmatched.emplace_back( &rhs_entry );
      ++rhs_it;
    }
  }

  // Register any extra entries at the end as unmatched
  while( lhs_it != lhs_entries.end() )
  {
    auto const& lhs_entry = **lhs_it;
    lhs_unmatched.emplace_back( &lhs_entry );
    ++lhs_it;
  }
  while( rhs_it != rhs_entries.end() )
  {
    auto const& rhs_entry = **rhs_it;
    rhs_unmatched.emplace_back( &rhs_entry );
    ++rhs_it;
  }

  // Report to user all unmatched tags
  if( !lhs_unmatched.empty() )
  {
    equivalent = false;
    print_difference( "unmatched tags in lhs packet:" );
    for(size_t i = 0; i < lhs_unmatched.size(); ++i)
    {
      auto const& entry = *lhs_unmatched.at( i );
      std::stringstream ss;
      ss << "  | (" << i << ") Tag " << entry.first;
      if( traits )
      {
        auto const& trait = kt::tag_traits_from_key( *traits, entry.first );
        ss << " (" << trait.name() << ")";
      }
      ss << ": " << entry.second;
    }
  }
  if( !rhs_unmatched.empty() )
  {
    equivalent = false;
    print_difference( "unmatched tags in lhs packet:" );
    for(size_t i = 0; i < rhs_unmatched.size(); ++i)
    {
      auto const& entry = *rhs_unmatched.at( i );
      std::cout << "  | (" << i << ") Tag " << entry.first;
      if( traits )
      {
        auto const& trait = kt::tag_traits_from_key( *traits, entry.first );
        std::cout << " (" << trait.name() << ")";
      }
      std::cout << ": " << entry.second << std::endl;
    }
  }

  return equivalent;
}

// ----------------------------------------------------------------------------
bool
compare_klv::impl
::compare(
  klv::klv_value const& lhs, klv::klv_value const& rhs,
  klv::klv_tag_traits const* trait )
{
  if( trait )
  {
    std::stringstream ss;
    ss << "Tag " << trait->tag() << " (" << trait->name() << ")";
    breadcrumbs.push_back( ss.str() );
  }
  auto equivalent = true;
  auto const traits = trait ? trait->subtag_lookup() : nullptr;

  // Types must be equal
  if( lhs.type() != rhs.type() )
  {
    auto const type_string =
      [ trait ]( klv::klv_value const& value ) -> std::string
      {
        if( value.empty() )
        {
          return "<none>";
        }
        if( !value.valid() )
        {
          return "<unparsed bytes>";
        }
        if( trait && value.type() == trait->type() )
        {
          return "<correct type>";
        }
        return "incorrect type: " + value.type_name();
      };
    auto const value_string =
      [ trait ]( klv::klv_value const& value ) -> std::string
      {
        return ( trait && value.type() == trait->type() )
          ? trait->format().to_string( value )
          : value.to_string();
      };
    print_difference( "types differ" );
    std::cout << "  | lhs type:  " << type_string( lhs ) << std::endl;
    std::cout << "  | rhs type:  " << type_string( rhs ) << std::endl;
    std::cout << "  | lhs value: " << value_string( lhs ) << std::endl;
    std::cout << "  | rhs value: " << value_string( rhs ) << std::endl;
    equivalent = false;
  }
  // Set-specific logic
  else if( lhs.type() == typeid( klv::klv_local_set ) )
  {
    equivalent =
      compare( lhs.get< klv::klv_local_set >(),
               rhs.get< klv::klv_local_set >(), traits );
  }
  else if( lhs.type() == typeid( klv::klv_universal_set ) )
  {
    equivalent =
      compare( lhs.get< klv::klv_universal_set >(),
               rhs.get< klv::klv_universal_set >(), traits );
  }
  // Values must be equal
  else
  {
    equivalent = print_if_neq( lhs, rhs, "value" );
  }

  if( trait )
  {
    breadcrumbs.pop_back();
  }

  return equivalent;
}

// ----------------------------------------------------------------------------
bool
compare_klv::impl
::compare( klv::klv_packet const& lhs, klv::klv_packet const& rhs )
{
  // Keys must be equal
  if( !print_if_neq( lhs.key, rhs.key, "key" ) )
  {
    return false;
  }

  auto const& trait = klv::klv_lookup_packet_traits().by_uds_key( lhs.key );
  breadcrumbs.emplace_back( trait.name() );

  // Values must be equal
  auto const equivalent = compare( lhs.value, rhs.value, &trait );

  breadcrumbs.pop_back();

  return equivalent;
}

// ----------------------------------------------------------------------------
bool
compare_klv::impl
::compare(
  std::vector< klv::klv_packet > const& lhs,
  std::vector< klv::klv_packet > const& rhs )
{
  // Just compare each pair of packets in turn
  auto equivalent = true;
  for(size_t i = 0; i < std::max( lhs.size(), rhs.size() ); ++i)
  {
    breadcrumbs.emplace_back(
      std::string{} + "klv_packet (" + std::to_string( i ) + ")" );

    if( i >= lhs.size() )
    {
      print_difference( "lhs is missing this packet" );
      equivalent = false;
      continue;
    }
    if( i >= rhs.size() )
    {
      print_difference( "rhs is missing this packet" );
      equivalent = false;
      continue;
    }

    equivalent &= compare( lhs.at( i ), rhs.at( i ) );

    breadcrumbs.pop_back();
  }

  return equivalent;
}

// ----------------------------------------------------------------------------
bool
compare_klv::impl
::could_be_paired(
  klv::klv_packet const& lhs, klv::klv_packet const& rhs ) const
{
  return lhs.key == rhs.key;
}

// ----------------------------------------------------------------------------
template < class Key >
std::vector< size_t >
compare_klv::impl
::difference_score(
  klv::klv_set< Key > const& lhs, klv::klv_set< Key > const& rhs ) const
{
  std::vector< size_t > result{ 0 };
  auto const lhs_entries = lhs.fully_sorted();
  auto const rhs_entries = rhs.fully_sorted();
  auto lhs_it = lhs_entries.begin();
  auto rhs_it = rhs_entries.begin();

  // Loop through entries
  while( lhs_it != lhs_entries.end() && rhs_it != rhs_entries.end() )
  {
    auto const& lhs_entry = **lhs_it;
    auto const& rhs_entry = **rhs_it;

    if( lhs_entry.first == rhs_entry.first )
    {
      // Keys are the same: recurse on the values
      auto const subscore =
        difference_score( lhs_entry.second, rhs_entry.second );

      // Allocate more difference sub-levels, if necessary
      result.resize( std::max( result.size(), subscore.size() + 1 ), 0 );

      // Add scores at each sub-level, maxing out at SIZE_MAX to avoid overflow
      for(size_t i = 0; i < subscore.size(); ++i)
      {
        auto const sum = result.at( i + 1 ) + subscore.at( i );
        result.at( i + 1 ) = ( sum < result.at( i + 1 ) ) ? SIZE_MAX : sum;
      }

      ++lhs_it;
      ++rhs_it;
    }
    else if( lhs_entry.first < rhs_entry.first )
    {
      // Rhs is missing this key
      ++result.at( 0 );
      ++lhs_it;
    }
    else
    {
      // Lhs is missing this key
      ++result.at( 0 );
      ++rhs_it;
    }
  }

  // One difference for each extra entry
  result.at( 0 ) += std::distance( lhs_it, lhs_entries.end() ) +
                    std::distance( rhs_it, rhs_entries.end() );

  return result;
}

// ----------------------------------------------------------------------------
std::vector< size_t >
compare_klv::impl
::difference_score(
  klv::klv_value const& lhs, klv::klv_value const& rhs ) const
{
  if( lhs.type() != rhs.type() )
  {
    // Maximally different
    return { SIZE_MAX };
  }

  // Reroute to set-specific logic
  if( lhs.type() == typeid( klv::klv_local_set ) )
  {
    return difference_score( lhs.get< klv::klv_local_set >(),
                             rhs.get< klv::klv_local_set >() );
  }
  if( lhs.type() == typeid( klv::klv_universal_set ) )
  {
    return difference_score( lhs.get< klv::klv_universal_set >(),
                             rhs.get< klv::klv_universal_set >() );
  }

  if( lhs != rhs )
  {
    // Standard difference
    return { 1 };
  }

  // No difference
  return { 0 };
}

// ----------------------------------------------------------------------------
compare_klv
::compare_klv()
  : d{ new impl }
{}

// ----------------------------------------------------------------------------
int
compare_klv
::run()
{
  // Parse command line
  auto& args = command_args();

  // Display help info and exit
  if( args[ "help" ].as< bool >() )
  {
    std::cerr << m_cmd_options->help();
    return 0;
  }

  // Load configuration
  auto config = find_configuration( "applets/compare_klv.conf" );
  if( args.count( "config" ) )
  {
    config->merge_config(
      vital::read_config_file( args[ "config" ].as< std::string >() ) );
  }

  // Determine which files to compare
  if( !args.count( "lhs-file" ) || !args.count( "rhs-file" ) )
  {
    std::cerr << "Please provide two files to compare" << std::endl;
    std::cerr << m_cmd_options->help();
    return -1;
  }

  std::filesystem::path lhs_path = args[ "lhs-file" ].as< std::string >();
  std::filesystem::path rhs_path = args[ "rhs-file" ].as< std::string >();

  // Open both files
  auto const lhs_is = create_metadata_istream( lhs_path, config );
  auto const rhs_is = create_metadata_istream( rhs_path, config );

  // Track if all packets in lhs and rhs have a match and are equal to that
  // match
  auto equivalent = true;

  // Loop through frames
  while( !lhs_is->at_end() || !rhs_is->at_end() )
  {
    // Extract information about this frame's KLV
    auto const build_data =
      []( vital::metadata_istream& is ) -> std::vector< istream_data > {
        if( is.at_end() )
        {
          return {};
        }

        std::vector< istream_data > data;
        for( size_t i = 0; i < is.metadata().size(); ++i )
        {
          data.emplace_back( is, is.metadata()[ i ] );
        }
        return data;
      };
    auto lhs_data = build_data( *lhs_is );
    auto rhs_data = build_data( *rhs_is );

    // Determine frame numbers
    auto const lhs_frame_number =
      lhs_is->at_end() ? 0 : lhs_is->frame_number();
    auto const rhs_frame_number =
      rhs_is->at_end() ? 0 : rhs_is->frame_number();
    auto const frame_number = std::max( lhs_frame_number, rhs_frame_number );
    d->breadcrumbs.emplace_back(
        std::string{} + "frame (" + std::to_string( frame_number ) + ")" );

    // Score each possible pair of packets on their similarity
    std::multimap< std::vector< size_t >, possible_pair > ranked_pairs;
    for( auto& lhs : lhs_data )
    {
      if( !lhs.klv )
      {
        continue;
      }
      for( auto& lhs_packet : *lhs.klv )
      {
        for( auto& rhs : rhs_data )
        {
          if( !rhs.klv )
          {
            continue;
          }
          for( auto& rhs_packet : *rhs.klv )
          {
            if( !d->could_be_paired( lhs_packet, rhs_packet ) )
            {
              continue;
            }

            auto const score =
              d->difference_score( lhs_packet.value, rhs_packet.value );
            ranked_pairs.emplace(
                score, possible_pair{ lhs, lhs_packet, rhs, rhs_packet } );
          }
        }
      }
    }

    // Match up the packets from the two istreams based on their similarity
    // scores
    std::vector< possible_pair > confirmed_pairs;
    for( auto& entry : ranked_pairs )
    {
      auto& pair = entry.second;
      auto redundant = false;
      for( auto const& confirmed_pair : confirmed_pairs )
      {
        if( &pair.lhs_packet == &confirmed_pair.lhs_packet ||
            &pair.rhs_packet == &confirmed_pair.rhs_packet )
        {
          redundant = true;
          break;
        }
      }
      if( redundant )
      {
        continue;
      }
      confirmed_pairs.emplace_back( pair );

      // Report if the matched packets are equal
      equivalent &= d->compare( pair.lhs_packet, pair.rhs_packet );
    }

    // Report any lhs packets that have no match on the rhs
    std::vector< klv::klv_packet const* > unmatched_lhs;
    for( auto& lhs : lhs_data )
    {
      if( !lhs.klv )
      {
        continue;
      }

      for( auto& packet : *lhs.klv )
      {
        bool found = false;
        for( auto const& confirmed_pair : confirmed_pairs )
        {
          if( &packet == &confirmed_pair.lhs_packet )
          {
            found = true;
            break;
          }
        }
        if( !found )
        {
          unmatched_lhs.emplace_back( &packet );
        }
      }
    }
    if( !unmatched_lhs.empty() )
    {
      d->print_difference( "unmatched packets in lhs stream:" );
      for( size_t i = 0; i < unmatched_lhs.size(); ++i )
      {
        std::cout << "  | (" << i << ") " << *unmatched_lhs.at( i ) <<
        std::endl;
      }
    }

    // Report any rhs packets that have no match on the lhs
    std::vector< klv::klv_packet const* > unmatched_rhs;
    for( auto& rhs : rhs_data )
    {
      if( !rhs.klv )
      {
        continue;
      }

      for( auto& packet : *rhs.klv )
      {
        bool found = false;
        for( auto const& confirmed_pair : confirmed_pairs )
        {
          if( &packet == &confirmed_pair.rhs_packet )
          {
            found = true;
            break;
          }
        }
        if( !found )
        {
          unmatched_rhs.emplace_back( &packet );
        }
      }
    }
    if( !unmatched_rhs.empty() )
    {
      d->print_difference( "unmatched packets in rhs stream:" );
      for( size_t i = 0; i < unmatched_rhs.size(); ++i )
      {
        std::cout << "  | (" << i << ") " << *unmatched_rhs.at( i ) <<
        std::endl;
      }
    }

    // Next frame
    d->breadcrumbs.pop_back();
    if( !lhs_is->at_end() )
    {
      lhs_is->next_frame();
    }
    if( !rhs_is->at_end() )
    {
      rhs_is->next_frame();
    }
  }

  return !equivalent;
}

// ----------------------------------------------------------------------------
void
compare_klv
::add_command_options()
{
  m_cmd_options->custom_help(
    "[options] lhs-file rhs-file\n"
    "This program prints differences found between the KLV in two files "
    "(video or JSON).\n" );

  m_cmd_options->positional_help(
    "\n  lhs-file: Left-hand-side video or JSON file for comparison."
    "\n  rhs-file: Right-hand-side video or JSON file for comparison." );

  m_cmd_options->add_options()( "h,help", "Display applet usage." )
    ( "c,config", "Provide configuration file.",
      cxxopts::value< std::string >(), "filename" )
    ( "lhs-file", "Left-hand-side video or JSON file for comparison.",
    cxxopts::value< std::string >() )
    ( "rhs-file", "Right-hand-side video or JSON file for comparison.",
    cxxopts::value< std::string >() )
  ;

  m_cmd_options->parse_positional( { "lhs-file", "rhs-file" } );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
