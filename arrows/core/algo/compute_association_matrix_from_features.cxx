// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of compute_association_matrix_from_features

#include "compute_association_matrix_from_features.h"

#include <vital/algo/detected_object_filter.h>
#include <vital/exceptions/algorithm.h>
#include <vital/types/object_track_set.h>
#include <vital/vital_config.h>

#include <algorithm>
#include <atomic>
#include <string>
#include <vector>

namespace kwiver {

namespace arrows {

namespace core {

using namespace kwiver::vital;

/// Private implementation class
class compute_association_matrix_from_features::priv
{
public:
  priv( compute_association_matrix_from_features& parent )
    : parent( parent )
  {}

  compute_association_matrix_from_features& parent;

  /// Raw pixel distance threshold
  double c_max_distance() { return parent.c_max_distance; }

  /// The feature matching algorithm to use
  vital::algo::detected_object_filter_sptr c_filter()
  { return parent.c_filter; }
};

// ----------------------------------------------------------------------------
void
compute_association_matrix_from_features
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.core.compute_association_matrix_from_features" );
}

/// Destructor
compute_association_matrix_from_features
::~compute_association_matrix_from_features() noexcept
{}

bool
compute_association_matrix_from_features
::check_configuration( vital::config_block_sptr config ) const
{
  return (
    check_nested_algo_configuration< algo::detected_object_filter >(
      "filter",
      config )
  );
}

/// Compute an association matrix given detections and tracks
bool
compute_association_matrix_from_features
::compute(
  VITAL_UNUSED kwiver::vital::timestamp ts,
  VITAL_UNUSED kwiver::vital::image_container_sptr image,
  kwiver::vital::object_track_set_sptr tracks,
  kwiver::vital::detected_object_set_sptr detections,
  kwiver::vital::matrix_d& matrix,
  kwiver::vital::detected_object_set_sptr& considered ) const
{
  considered = d_->c_filter()->filter( detections );

  auto filtered_dets = considered;
  auto filtered_tracks = tracks->tracks();

  const double invalid_value = std::numeric_limits< double >::max();

  if( filtered_tracks.empty() || filtered_dets->empty() )
  {
    matrix = kwiver::vital::matrix_d();
  }
  else
  {
    matrix = kwiver::vital::matrix_d(
      filtered_tracks.size(),
      filtered_dets->size() );

    for( size_t t = 0; t < filtered_tracks.size(); ++t )
    {
      for( size_t d = 0; d < filtered_dets->size(); ++d )
      {
        track_sptr trk = filtered_tracks[ t ];
        detected_object_sptr det = filtered_dets->at( d );

        auto det_features = det->descriptor();
        auto trk_features = decltype( det_features ) {};

        if( !trk->empty() )
        {
          object_track_state* trk_state =
            dynamic_cast< object_track_state* >( trk->back().get() );

          if( trk_state->detection() )
          {
            double dist = d_->c_max_distance();

            if( d_->c_max_distance() > 0.0 )
            {
              auto center1 = trk_state->detection()->bounding_box().center();
              auto center2 = det->bounding_box().center();

              dist = ( center1[ 0 ] - center2[ 0 ] ) *
                     ( center1[ 0 ] - center2[ 0 ] );
              dist += ( ( center1[ 1 ] - center2[ 1 ] ) *
                        ( center1[ 1 ] - center2[ 1 ] ) );
              dist = std::sqrt( dist );
            }

            if( d_->c_max_distance() <= 0.0 || dist < d_->c_max_distance() )
            {
              trk_features = trk_state->detection()->descriptor();
            }
          }
        }

        if( det_features && trk_features )
        {
          if( det_features->size() != trk_features->size() )
          {
            throw std::runtime_error( "Invalid descriptor dimensions" );
          }

          double sum_sqr = 0.0;

          for( auto pos1 = det_features->raw_data(),
               pos2 = trk_features->raw_data(),
               end = pos1 + det_features->size();
               pos1 != end; ++pos1, ++pos2 )
          {
            sum_sqr += ( ( *pos1 - *pos2 ) * ( *pos1 - *pos2 ) );
          }

          matrix( t, d ) = std::sqrt( sum_sqr );
        }
        else
        {
          matrix( t, d ) = invalid_value;
        }
      }
    }
  }

  considered = detections;
  return ( matrix.size() > 0 );
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
