/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file image_object_detector_trampoline.txx
 *
 * \brief trampoline for overriding virtual functions of algorithm_def<image_object_detector> and image_object_detector
 */

#ifndef IMAGE_OBJECT_DETECTOR_TRAMPOLINE_TXX
#define IMAGE_OBJECT_DETECTOR_TRAMPOLINE_TXX


#include <py_kwiver/vital/util/pybind11.h>
#include <py_kwiver/vital/algo/trampoline/algorithm_trampoline.txx>
#include <vital/algo/image_object_detector.h>
#include <vital/types/detected_object_set.h>
#include <vital/types/image_container.h>


template <class algorithm_def_iod_base=kwiver::vital::algorithm_def<kwiver::vital::algo::image_object_detector>>
class algorithm_def_iod_trampoline :
      public algorithm_trampoline<algorithm_def_iod_base>
{
  public:
    using algorithm_trampoline<algorithm_def_iod_base>::algorithm_trampoline;

    std::string type_name() const override
    {
      VITAL_PYBIND11_OVERLOAD(
        std::string,
        kwiver::vital::algorithm_def<kwiver::vital::algo::image_object_detector>,
        type_name,
      );
    }
};


template <class image_object_detector_base=kwiver::vital::algo::image_object_detector>
class image_object_detector_trampoline :
      public algorithm_def_iod_trampoline<image_object_detector_base>
{
  public:
    using algorithm_def_iod_trampoline<image_object_detector_base>::
              algorithm_def_iod_trampoline;
    kwiver::vital::detected_object_set_sptr detect(kwiver::vital::image_container_sptr image_data) const override
    {
      VITAL_PYBIND11_OVERLOAD_PURE(
        kwiver::vital::detected_object_set_sptr,
        kwiver::vital::algo::image_object_detector,
        detect,
        image_data
      );
    }
};

#endif
