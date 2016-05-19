
#include "vidtk_change_detector.h"

#include <object_detectors/detector_factory.h>

#include <arrows/algorithms/vxl/image_container.h>

#include <vital/types/vector.h>
#include <vital/io/eigen_io.h>
#include <vital/util/cpu_timer.h>
#include <vital/logger/logger.h>
#include <kwiversys/SystemTools.hxx>

#include <arrows/algorithms/ocv/image_container.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <math.h>

using namespace vidtk;

namespace kwiver {
namespace arrows {
namespace vidtk {

class vidtk_change_detector::priv
{
public:

// =====================================================================
  priv() : m_factory("detector_factory"), m_frame_number(0)
  {
  }


  priv(priv const& other) : m_factory(other.m_factory), m_detector(other.m_detector), m_config_filename(other.m_config_filename),
  m_frame_number(other.m_frame_number), m_labels(other.m_labels)
  {
  }


  ~priv()
  {
  }

  detector_factory<vxl_byte> m_factory;
  process_smart_pointer< detector_super_process< vxl_byte > > m_detector;
  std::string m_config_filename;
  unsigned int m_frame_number;
  vital::object_labels_sptr m_labels;
};

vidtk_change_detector::vidtk_change_detector()
: d(new priv())
{
}

vidtk_change_detector::vidtk_change_detector(vidtk_change_detector const& frd)
: d(new priv(*frd.d))
{
}

vidtk_change_detector::~vidtk_change_detector()
{
}

// --------------------------------------------------------------------
vital::config_block_sptr 
vidtk_change_detector::
get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();
  config->set_value("config_file", d->m_config_filename,  "config file for vidtk");
  return config;
}

// --------------------------------------------------------------------
void vidtk_change_detector::
set_configuration(vital::config_block_sptr config_in)
{
  config_block vidtkConfig = d->m_factory.params();
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(config_in);

  this->d->m_config_filename  = config->get_value<std::string>("config_file");
  if(this->d->m_config_filename.empty()) return;
  std::cout << this->d->m_config_filename << "    " << !(this->d->m_config_filename.empty()) << std::endl;
  vidtkConfig.parse(this->d->m_config_filename);
  config_block block;
  block.add_subblock(vidtkConfig, "detector_factory");
  d->m_detector = d->m_factory.create_detector(block);
  d->m_detector->set_params(vidtkConfig);
  d->m_detector->initialize();
  this->d->m_labels = vital::object_labels_sptr(new vital::object_labels(std::vector<std::string>(1,"motion")));
}


// --------------------------------------------------------------------
bool vidtk_change_detector::
check_configuration(vital::config_block_sptr config) const
{
  std::string fname = config->get_value<std::string>("config_file");
  std::cout << fname << "    " << !fname.empty() << std::endl;
  return !fname.empty();
}


// --------------------------------------------------------------------
vital::detected_object_set_sptr 
vidtk_change_detector::
detect( vital::image_container_sptr image_data) const
{
  if(image_data == NULL) return NULL;
  vil_image_view<vxl_byte> img =   kwiver::arrows::vxl::image_container::vital_to_vxl(image_data->get_image());
  timestamp ts(d->m_frame_number*0.1, d->m_frame_number);
  d->m_frame_number++;
  vil_image_view< bool > mask;
  image_to_image_homography i2i;
  i2i.set_identity(true);
  image_to_plane_homography i2p;
  image_to_utm_homography i2u;
  plane_to_image_homography p2i;
  plane_to_utm_homography p2u;
  image_to_plane_homography i2p2;
  video_modality modality;
  shot_break_flags sbf;
  gui_frame_info gui;
  d->m_detector->input_image(img);
  d->m_detector->input_timestamp(ts);
  d->m_detector->input_mask_image(mask);
  d->m_detector->input_src_to_ref_homography(i2i);
  d->m_detector->input_src_to_wld_homography(i2p);
  d->m_detector->input_src_to_utm_homography(i2u);
  d->m_detector->input_wld_to_src_homography(p2i);
  d->m_detector->input_wld_to_utm_homography(p2u);
  d->m_detector->input_ref_to_wld_homography(i2p2);
  d->m_detector->input_world_units_per_pixel(0.5);
  d->m_detector->input_video_modality(modality);
  d->m_detector->input_shot_break_flags(sbf);
  d->m_detector->input_gui_feedback(gui);
  d->m_detector->step2();
  std::vector< image_object_sptr > image_objects = d->m_detector->output_image_objects();

  std::vector<vital::detected_object_sptr> detected_objects;
  for(unsigned i = 0; i < image_objects.size(); ++i)
  {
     vgl_box_2d< unsigned > const& bbox = image_objects[i]->get_bbox();
    vital::detected_object::bounding_box pbox(vital::vector_2d(bbox.min_x(), bbox.min_y()),vital::vector_2d(bbox.max_x(), bbox.max_y()));
    vital::object_type_sptr classification( new  vital::object_type(this->d->m_labels, std::vector<double>(1,1)));
    detected_objects.push_back(vital::detected_object_sptr(new vital::detected_object(pbox, 1.0, classification)));
  }
  return vital::detected_object_set_sptr(new vital::detected_object_set(detected_objects, this->d->m_labels));;
}

} } } //end namespace
