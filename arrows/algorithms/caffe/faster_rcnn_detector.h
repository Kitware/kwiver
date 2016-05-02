#ifndef FASTER_RCNN_DETECTOR_H_
#define FASTER_RCNN_DETECTOR_H_

#include <arrows/algorithms/caffe/algorithms_caffe_export.h>

#include <vital/vital_config.h>

#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/object_labels.h>

#include <vital/algo/image_object_detector.h>

#include <vital/config/config_block.h>

#include <opencv2/core/core.hpp>

#include <caffe/blob.hpp>
#include <caffe/net.hpp>

#include <utility>

namespace kwiver {
namespace arrows {
namespace caffe {

class ALGORITHMS_CAFFE_EXPORT faster_rcnn_detector
  : public vital::algorithm_impl<faster_rcnn_detector, vital::algo::image_object_detector>
{
public:

  faster_rcnn_detector();
  faster_rcnn_detector(faster_rcnn_detector const& frd);

  virtual ~faster_rcnn_detector();

  virtual std::string impl_name() const { return "faster_rcnn_detector"; }

  virtual vital::config_block_sptr get_configuration() const;

  virtual void set_configuration(vital::config_block_sptr config);

  virtual bool check_configuration(vital::config_block_sptr config) const;

  virtual vital::detected_object_set_sptr detect( vital::image_container_sptr image_data) const;

private:

  class priv;
  const std::unique_ptr<priv> d;

};
}}}

#endif // FASTER_RCNN_DETECTOR_H_
