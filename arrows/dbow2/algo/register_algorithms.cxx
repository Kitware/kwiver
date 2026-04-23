/*ckwg +29
 * Copyright 2017, 2020 by Kitware, Inc.
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
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be
 * used
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
 * \file
 * \brief OpenCV algorithm registration implementation
 */

#include <arrows/dbow2/kwiver_algo_dbow2_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

#include <opencv2/opencv_modules.hpp>
#ifdef HAVE_OPENCV_NONFREE
#include <opencv2/nonfree/nonfree.hpp>
#endif

#include <arrows/dbow2/algo/match_descriptor_sets.h>

namespace kwiver {

namespace arrows {

namespace dbow2 {

extern "C"
KWIVER_ALGO_DBOW2_PLUGIN_EXPORT
void
register_factories( ::kwiver::vital::plugin_loader& vpm )
{
  using xvpf = ::kwiver::vital::plugin_factory;

#if defined( HAVE_OPENCV_NONFREE )
  cv::initModule_nonfree();
#endif

  auto fact = vpm.add_factory< vital::algo::match_descriptor_sets,
    match_descriptor_sets >( "dbow2" );
  fact->add_attribute( xvpf::PLUGIN_MODULE_NAME, "arrows.dbow2" );
}

} // end namespace dbow2

} // end namespace arrows

} // end namespace kwiver
