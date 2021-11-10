// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/serialize/json/load_save.h>

#include <vital/types/metadata.h>
#include <vital/types/metadata_map.h>
#include <vital/types/metadata_traits.h>
#include <vital/types/geo_point.h>
#include <vital/types/geo_polygon.h>
#include <vital/types/polygon.h>

#include <vital/internal/cereal/cereal.hpp>
#include <vital/internal/cereal/archives/json.hpp>
#include <vital/internal/cereal/types/vector.hpp>
#include <vital/internal/cereal/types/map.hpp>
#include <vital/internal/cereal/types/utility.hpp>

namespace kv = kwiver::vital;

namespace {

// ----------------------------------------------------------------------------
struct meta_item
{
  meta_item() : m_tag{ kv::VITAL_META_UNKNOWN }, m_value{ 0 } {}

  meta_item( kv::metadata_item const& item )
    : m_tag{ item.tag() }, m_value{ item.data() } {}

  template< class Archive >
  struct save_visitor {
    template< class T >
    void operator()( T const& value ) const
    {
      archive( CEREAL_NVP( value ) );
    }

    Archive& archive;
  };

  template< class Archive >
  struct load_visitor {
    template< class T >
    void operator()() const {
      T value;
      archive( CEREAL_NVP( value ) );
      data = value;
    }

    Archive& archive;
    kv::metadata_value& data;
  };

  // ---------------------------------------------
  /// Save a single metadata item
  template < class Archive >
  void save( Archive& archive ) const
  {
    auto const& trait = kwiver::vital::tag_traits_by_tag( m_tag );

    archive( ::cereal::make_nvp( "tag", m_tag ) );

    kv::visit( save_visitor< Archive >{ archive }, m_value );

    // These two items are included to increase readability of the
    // serialized form and are not used when deserializing.
    archive( ::cereal::make_nvp( "name", trait.name() ) );
    archive( ::cereal::make_nvp( "type", trait.type_name() ) );
  }

  // -------------------------------------------------
  /*
   * Load a single metadata element
   */
  template< class Archive >
  void load( Archive& archive )
  {
    // Get the tag value
    archive( ::cereal::make_nvp( "tag", m_tag ) );

    // Get associated traits to assist with decoding the data portion
    auto const& trait = kv::tag_traits_by_tag( m_tag );

    kv::visit_metadata_types( load_visitor< Archive >{ archive, m_value },
                              trait.type() );
  }

  kv::vital_metadata_tag m_tag;
  kv::metadata_value m_value;
};

using meta_vect_t = std::vector< meta_item >;

} // namespace <anonymous>

namespace cereal {

// ----------------------------------------------------------------------------
void save( ::cereal::JSONOutputArchive& archive,
           kwiver::vital::metadata_vector const& meta_packets )
{
  std::vector< kwiver::vital::metadata > meta_packets_dereferenced;
  for( auto const& packet : meta_packets )
  {
    meta_packets_dereferenced.push_back( *packet );
  }
  save( archive, meta_packets_dereferenced );
}

// ----------------------------------------------------------------------------
void load( ::cereal::JSONInputArchive& archive,
           kwiver::vital::metadata_vector& meta )
{
  std::vector< kwiver::vital::metadata > meta_packets_dereferenced;
  load( archive, meta_packets_dereferenced );

  for( auto const& meta_packet : meta_packets_dereferenced )
  {
    meta.push_back(
      std::make_shared< kwiver::vital::metadata >( meta_packet ) );
  }
}

// ----------------------------------------------------------------------------
void save( ::cereal::JSONOutputArchive& archive,
           kwiver::vital::metadata const& packet_map )
{
  meta_vect_t packet_vec;

  // Serialize one metadata collection
  for( auto const& item : packet_map )
  {
    packet_vec.emplace_back( *item.second );
  }

  save( archive, packet_vec );
}

// ----------------------------------------------------------------------------
void load( ::cereal::JSONInputArchive& archive,
           kwiver::vital::metadata& packet_map )
{
  meta_vect_t meta_vect; // intermediate form

  // Deserialize the list of elements for one metadata collection
  load( archive, meta_vect );

  // Convert the intermediate form back to a real metadata collection
  for( auto const& it : meta_vect )
  {
    packet_map.add( it.m_tag, it.m_value );
  }
}

// ----------------------------------------------------------------------------
void save( ::cereal::JSONOutputArchive& archive,
           kwiver::vital::metadata_map::map_metadata_t const& meta_map )
{
  archive( make_size_tag( static_cast< size_type >( meta_map.size() ) ) );

  for ( auto const& meta_vec : meta_map )
  {
    archive( make_map_item( meta_vec.first, meta_vec.second ) );
  }
}

// ----------------------------------------------------------------------------
void load( ::cereal::JSONInputArchive& archive,
           kwiver::vital::metadata_map::map_metadata_t& meta_map )
{
  size_type size;
  archive( make_size_tag( size ) );

  meta_map.clear();

  auto hint = meta_map.begin();
  for( size_t i = 0; i < size; ++i )
  {
    kwiver::vital::frame_id_t key;
    kwiver::vital::metadata_vector value;

    archive( make_map_item(key, value) );
    hint = meta_map.emplace_hint( hint, std::move( key ), std::move( value ) );
  }
}

} // namespace cereal
