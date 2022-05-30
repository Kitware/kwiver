// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "merge_detections_suppress_in_regions.h"

namespace kwiver {
namespace arrows {
namespace core {

/// Private implementation class
class merge_detections_suppress_in_regions::priv
{
public:

  /// Constructor
  priv()
  : suppression_class( "" ),
    borderline_class( "" ),
    borderline_scale_factor( 0.5 ),
    min_overlap( 0.5 ),
    output_region_classes( true )
  {}

  /// Destructor
  ~priv() {}

  /// Parameters
  std::string suppression_class;
  std::string borderline_class;
  double borderline_scale_factor;
  double min_overlap;
  bool output_region_classes;
};


/// Constructor
merge_detections_suppress_in_regions
::merge_detections_suppress_in_regions()
: d( new priv() )
{
}


/// Destructor
merge_detections_suppress_in_regions
::~merge_detections_suppress_in_regions()
{
}


/// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
merge_detections_suppress_in_regions
::get_configuration() const
{
  vital::config_block_sptr config = vital::algo::merge_detections::get_configuration();

  config->set_value( "suppression_class", d->suppression_class,
    "Suppression region class IDs, will eliminate any detections overlapping with "
    "this class entirely." );

  config->set_value( "borderline_class", d->borderline_class,
    "Borderline region class IDs, will reduce the probability of any detections "
    "overlapping with the class by some fixed scale factor." );

  config->set_value( "borderline_scale_factor", d->borderline_scale_factor,
    "The factor by which the detections are scaled when overlapping with borderline "
    "regions." );

  config->set_value( "min_overlap", d->min_overlap,
    "The minimum percent a detection can overlap with a suppression category before "
    "it's discarded or reduced. Range [0.0,1.0]." );

  config->set_value( "output_region_classes", d->output_region_classes,
    "Add suppression and borderline classes to output" );

  return config;
}


/// Set this algorithm's properties via a config block
void
merge_detections_suppress_in_regions
::set_configuration( vital::config_block_sptr in_config )
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  d->suppression_class = config->get_value< std::string >( "suppression_class" );
  d->borderline_class = config->get_value< std::string >( "borderline_class" );
  d->borderline_scale_factor = config->get_value< double >( "borderline_scale_factor" );
  d->min_overlap = config->get_value< double >( "min_overlap" );
  d->output_region_classes = config->get_value< double >( "output_region_classes" );
}

/// Check that the algorithm's currently configuration is valid
bool
merge_detections_suppress_in_regions
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// -----------------------------------------------------------------------------
vital::detected_object_set_sptr
merge_detections_suppress_in_regions
::merge( std::vector< kwiver::vital::detected_object_set_sptr > const& sets ) const
{
  // Returns detections sorted by confidence threshold
  vital::detected_object_set_sptr output( new vital::detected_object_set() );

  if( sets.empty() || !sets[0] )
  {
    return output;
  }
  if( sets.size() == 1 )
  {
    return sets[0];
  }

  vital::detected_object_set_sptr region_set = sets[0];

  for( unsigned i = 1; i < sets.size(); i++ )
  {
    auto test_set = sets[i];

    for( auto det : *test_set )
    {
      bool should_add = true, should_adjust = false;
      const auto det_bbox = det->bounding_box();

      for( auto region : *region_set )
      {
        const auto reg_bbox = region->bounding_box();
        const auto overlap = kwiver::vital::intersection( det_bbox, reg_bbox );

        // Check how much they overlap. Only keep if the overlapped percent isn't too high
        if( overlap.min_x() < overlap.max_x() && overlap.min_y() < overlap.max_y() &&
            ( overlap.area() / det_bbox.area() ) >= d->min_overlap )
        {
          std::string reg_class;

          if( region->type() )
          {
            region->type()->get_most_likely( reg_class );
          }

          if( d->suppression_class == reg_class ||
              ( d->suppression_class.empty() && d->borderline_class.empty() ) )
          {
            should_add = false;
          }
          else if( !d->borderline_class.empty() && d->borderline_class == reg_class )
          {
            should_adjust = true;
          }
        }
      }
      if( should_add ) // It doesn't overlap too much, add it in
      {
        if( should_adjust )
        {
          auto new_det = det->clone();
          auto adj_type = new_det->type();

          for( auto cls_pair : *adj_type )
          {
            cls_pair.second = cls_pair.second * d->borderline_scale_factor;
          }

          new_det->set_type( adj_type );
          output->add( new_det );
        }
        else
        {
          output->add( det );
        }
      }
    }
  }

  if( d->output_region_classes )
  {
    for( auto region : *region_set )
    {
      output->add( region );
    }
  }

  return output;
}

}}} // end namespace kwiver
