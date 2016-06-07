#include "non_maximual_supression.h"

namespace kwiver {
namespace arrows {

namespace core
{

non_maximual_supression::non_maximual_supression()
:overlap_threshold_(0.3)
{
}

vital::config_block_sptr non_maximual_supression::get_configuration() const
{
   // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value("overlap_threshold", overlap_threshold_,
                    "The threshold to consider the bounding box is potentially the same object.");

  return config;
}

void non_maximual_supression::set_configuration(vital::config_block_sptr config_in)
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(config_in);
  this->overlap_threshold_ = config->get_value<double>("overlap_threshold");
}

bool non_maximual_supression::check_configuration(vital::config_block_sptr config) const
{
  return true;
}

vital::detected_object_set_sptr non_maximual_supression::filter( vital::detected_object_set_sptr input_set) const
{
  if(input_set == NULL) return NULL;
  vital::object_labels::iterator label_iter = input_set->get_labels();
  for(;!label_iter.is_end(); ++label_iter)
  {
    vital::detected_object_set::iterator class_iterator = input_set->get_iterator(label_iter.get_key(), true);
    for ( size_t i = 0; i < class_iterator.size(); ++i )
    {
      vital::object_type_sptr type_i = class_iterator[i]->get_classifications();
      vital::detected_object::bounding_box bbox_i = class_iterator[i]->get_bounding_box();
      if(type_i == NULL) continue;
      if(type_i->get_score(label_iter.get_key()) == vital::object_type::INVALID_SCORE) continue;
      double area = bbox_i.area();
      for ( size_t j = i+1; j < class_iterator.size(); ++j )
      {
        vital::object_type_sptr type_j = class_iterator[j]->get_classifications();
        vital::detected_object::bounding_box bbox_j = class_iterator[j]->get_bounding_box();
        if(type_j == NULL) continue;
        if(type_j->get_score(label_iter.get_key()) == vital::object_type::INVALID_SCORE) continue;
        vital::detected_object::bounding_box inter = bbox_i.intersection(bbox_j);
        double aj = bbox_j.area();
        double interS = inter.area();
        double t = interS / (area + aj - interS);
        if (t >= this->overlap_threshold_)
        {
          type_j->set_score(label_iter.get_key(), vital::object_type::INVALID_SCORE);
        }
      }
    }
  }
  return input_set;
}

}}}// end namespace
