#ifndef VIDTK_CHANGE_DETECTOR_H_
#define VIDTK_CHANGE_DETECTOR_H_

#include <arrows/algorithms/vidtk/algorithms_vidtk_export.h>

#include <vital/vital_config.h>

#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/object_labels.h>

#include <vital/algo/image_object_detector.h>

#include <vital/config/config_block.h>

#include <opencv2/core/core.hpp>


#include <utility>

namespace kwiver {
namespace arrows {
namespace vidtk {

class ALGORITHMS_VIDTK_EXPORT vidtk_change_detector
  : public vital::algorithm_impl<vidtk_change_detector, vital::algo::image_object_detector>
{
public:

  vidtk_change_detector();
  vidtk_change_detector(vidtk_change_detector const& frd);

  virtual ~vidtk_change_detector();

  virtual std::string impl_name() const { return "vidtk_change_detector"; }

  virtual vital::config_block_sptr get_configuration() const;

  virtual void set_configuration(vital::config_block_sptr config);

  virtual bool check_configuration(vital::config_block_sptr config) const;

  virtual vital::detected_object_set_sptr detect( vital::image_container_sptr image_data) const;

private:

  class priv;
  const std::unique_ptr<priv> d;

};
}}}

#endif // VIDTK_CHANGE_DETECTOR_H_
