#ifndef NON_MAXIMUAL_SUPRESSION_H_
#define NON_MAXIMUAL_SUPRESSION_H_

#include <vital/vital_config.h>
#include <arrows/algorithms/core/algorithms_core_export.h>

#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/object_labels.h>

#include <vital/algo/detected_object_filter.h>

#include <vital/config/config_block.h>

#include <opencv2/core/core.hpp>

#include <utility>

namespace kwiver {
namespace arrows {

namespace core
{

class ALGORITHMS_CORE_EXPORT non_maximual_supression
  : public vital::algorithm_impl<non_maximual_supression, vital::algo::detected_object_filter>
{
public:

  non_maximual_supression();

  virtual ~non_maximual_supression(){}

  virtual std::string impl_name() const { return "non_maximual_supression"; }

  virtual vital::config_block_sptr get_configuration() const;

  virtual void set_configuration(vital::config_block_sptr config);

  virtual bool check_configuration(vital::config_block_sptr config) const;

  virtual vital::detected_object_set_sptr filter( vital::detected_object_set_sptr input_set) const;

private:

  double overlap_threshold_;

};
}}}//End namespace


#endif // NON_MAXIMUAL_SUPRESSION_H_
