#ifndef CLASS_PROBABLITY_FILTER_H_
#define CLASS_PROBABLITY_FILTER_H_

#include <vital/vital_config.h>
#include <arrows/algorithms/core/algorithms_core_export.h>

#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/object_labels.h>

#include <vital/algo/detected_object_filter.h>

#include <vital/config/config_block.h>

#include <opencv2/core/core.hpp>

#include <utility>
#include <set>

namespace kwiver {
namespace arrows {

namespace core
{

class ALGORITHMS_CORE_EXPORT class_probablity_filter
  : public vital::algorithm_impl<class_probablity_filter, vital::algo::detected_object_filter>
{
public:

  class_probablity_filter();

  virtual ~class_probablity_filter(){}

  virtual std::string impl_name() const { return "class_probablity_filter"; }

  virtual vital::config_block_sptr get_configuration() const;

  virtual void set_configuration(vital::config_block_sptr config);

  virtual bool check_configuration(vital::config_block_sptr config) const;

  virtual vital::detected_object_set_sptr filter( vital::detected_object_set_sptr input_set) const;

private:

  bool m_keep_all_classes;
  std::set<std::string> m_keep_classes;
  double m_threshold;

};
}}}//End namespace


#endif // CLASS_PROBABLITY_FILTER_H_
