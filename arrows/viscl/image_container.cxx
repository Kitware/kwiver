// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "image_container.h"

#include <vil/vil_image_view.h>
#include <vil/vil_save.h>

#include <viscl/core/manager.h>
#include <viscl/tasks/gaussian_smooth.h>
#include <viscl/vxl/transfer.h>

#include <boost/shared_ptr.hpp>

#include <memory>

namespace kwiver {
namespace arrows {
namespace vcl {

/// Constructor - convert base image container to a VisCL image
image_container
::image_container(const vital::image_container& image_cont)
: data_(image_container_to_viscl(image_cont))
{
}

/// The size of the image data in bytes
size_t
image_container
::size() const
{
  return data_.mem_size();
}

/// Convert a VisCL image to a VITAL image
vital::image
image_container
::viscl_to_vital(const viscl::image& img_cl)
{
  size_t width = img_cl.width();
  size_t height = img_cl.height();
  vital::image img(width, height);

  cl::size_t<3> origin;
  // Defaults are already 0 on initialization, setting is redundant

  cl::size_t<3> region;
  region[0] = width;
  region[1] = height;
  region[2] = 1;

  viscl::cl_queue_t q = viscl::manager::inst()->create_queue();
  q->enqueueReadImage(*img_cl().get(), CL_TRUE, origin, region,
                      0, 0, img.memory()->data());

  return img;
}

/// Convert a VITAL image to a VisCL image
viscl::image
image_container
::vital_to_viscl(const vital::image& img)
{
  cl::ImageFormat img_fmt;
  img_fmt = cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8);

  // viscl::image is only able to display single channel images at the moment
  // it also only supports byte and float images below only byte are supported
  if( img.depth() == 1 )
  {
    return viscl::image(boost::make_shared<cl::Image2D>(
                          viscl::manager::inst()->get_context(),
                          CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                          img_fmt,
                          img.width(),
                          img.height(),
                          0,
                          (void *)img.first_pixel()));
  }

  if (img.depth() == 3)
  {
    //Convert color image to a grey scale image and upload it.
    unsigned char *grey = new unsigned char [img.width() * img.height()];
    for (unsigned int j = 0; j < img.height(); j++)
    {
      for (unsigned int i = 0; i < img.width(); i++)
      {
        double value = 0.2125 * img(i,j,0) + 0.7154 * img(i,j,1) + 0.0721 * img(i,j,2);
        grey[j * img.width() + i] = static_cast<unsigned char>(value);
      }
    }

    viscl::image image = viscl::image(boost::make_shared<cl::Image2D>(
                                        viscl::manager::inst()->get_context(),
                                        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                        img_fmt,
                                        img.width(),
                                        img.height(),
                                        0,
                                        grey));

    delete [] grey;

    return image;
  }

  //TODO: Throw exception
  return viscl::image();
}

/// Extract a VisCL image from any image container
viscl::image
image_container_to_viscl(const vital::image_container& img)
{
  if( const image_container* c =
          dynamic_cast<const image_container*>(&img) )
  {
    return c->get_viscl_image();
  }
  return image_container::vital_to_viscl(img.get_image());
}

} // end namespace vcl
} // end namespace arrows
} // end namespace kwiver
