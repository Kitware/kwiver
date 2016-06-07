#include "class_probablity_filter.h"

#include <sstream>

namespace kwiver {
namespace arrows {

namespace core
{

class_probablity_filter::class_probablity_filter()
:m_keep_all_classes(true), m_threshold(0.0)
{
}

vital::config_block_sptr class_probablity_filter::get_configuration() const
{
   // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value("threshold", m_threshold,
                    "The threshold to keep a detection.");
  std::string list_of_classes;
  for(std::set<std::string>::const_iterator i = m_keep_classes.begin(); i != m_keep_classes.end(); ++i)
  {
    list_of_classes += (list_of_classes.empty())?"":";" + *i;
  }
  config->set_value("keep_classes", list_of_classes,
                    "What detection classes to keep.");
  config->set_value("keep_all_classes", m_keep_all_classes,
                    "Keeps all the classes");

  return config;
}

void class_probablity_filter::set_configuration(vital::config_block_sptr config_in)
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(config_in);
  this->m_threshold = config->get_value<double>("threshold");
  std::string list = config->get_value<std::string>("keep_classes");
  std::string parsed;
  {
    std::stringstream ss(list);

    while ( std::getline( ss, parsed, ';' ) )
    {
      if ( ! parsed.empty() )
      {
        m_keep_classes.insert( parsed );
      }
    }
  }
  m_keep_all_classes = config->get_value<bool>("keep_all_classes");
}

bool class_probablity_filter::check_configuration(vital::config_block_sptr config) const
{
  return true;
}

vital::detected_object_set_sptr class_probablity_filter::filter( vital::detected_object_set_sptr input_set) const
{
  if(input_set == NULL) return NULL;
  vital::object_labels::iterator label_iter = input_set->get_labels();
  std::map<vital::detected_object_sptr, vital::object_type_sptr> filtered_map;
  vital::object_labels_sptr ols = NULL;

  for(;!label_iter.is_end(); ++label_iter)
  {
    if(!m_keep_all_classes && m_keep_classes.find(label_iter.get_label()) == m_keep_classes.end())
    {
      continue;
    }
    vital::detected_object_set::iterator class_iterator = input_set->get_iterator(label_iter.get_key(), false, this->m_threshold);
    for ( ; !class_iterator.is_end(); ++class_iterator )
    {
      vital::detected_object_sptr dos = class_iterator.get_object();
      std::map<vital::detected_object_sptr, vital::object_type_sptr>::iterator at = filtered_map.find(dos);
      vital::object_type_sptr types = dos->get_classifications();
      if(at != filtered_map.end())
      {
        at->second->set_score(label_iter.get_key(), types->get_score(label_iter.get_key()));
      }
      else
      {
        std::vector<double> tmpv(types->labels()->get_number_of_labels(), vital::object_type::INVALID_SCORE);
        tmpv[label_iter.get_key()] = types->get_score(label_iter.get_key());
        filtered_map[dos] = vital::object_type_sptr( new vital::object_type(types->labels(), tmpv));
      }
    }
  }
  std::vector<vital::detected_object_sptr> tmp;
  for(std::map<vital::detected_object_sptr, vital::object_type_sptr>::const_iterator iter = filtered_map.begin(); iter != filtered_map.end(); ++iter)
  {
    tmp.push_back(vital::detected_object_sptr(new vital::detected_object(iter->first->get_bounding_box(), iter->first->get_confidence(), iter->second)));
  }
  return vital::detected_object_set_sptr(new vital::detected_object_set(tmp, input_set->get_object_labels()));
}

}}}// end namespace
