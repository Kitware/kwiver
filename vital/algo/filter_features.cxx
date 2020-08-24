/*ckwg +29
 * Copyright 2015 by Kitware, Inc.
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
 * \file
 * \brief Instantiation of \link kwiver::vital::algo::algorithm_def algorithm_def<T>
 *        \endlink for \link kwiver::vital::algo::filter_features
 *        filter_features \endlink
 */


#include <vital/algo/filter_features.h>
#include <vital/algo/algorithm.txx>

namespace kwiver {
namespace vital {
namespace algo {


filter_features
::filter_features()
{
  attach_logger( "algo.filter_features" );
}


feature_set_sptr
filter_features
::filter(feature_set_sptr feat) const
{
  std::vector<unsigned int> indices;
  return filter(feat, indices);
}


std::pair<feature_set_sptr, descriptor_set_sptr>
filter_features
::filter( feature_set_sptr feat, descriptor_set_sptr descr) const
{
  std::vector<unsigned int> indices;
  feature_set_sptr filt_feat = filter(feat, indices);

  // Iterate through descriptor sptrs, keeping those in the same index as the
  // kept features.
  std::vector<descriptor_sptr> filtered_descr;
  filtered_descr.reserve(indices.size());

  for (unsigned int i = 0; i < indices.size(); i++)
  {
    filtered_descr.push_back( descr->at( indices[i] ) );
  }

  descriptor_set_sptr filt_descr = std::make_shared<kwiver::vital::simple_descriptor_set>(
                                     kwiver::vital::simple_descriptor_set(filtered_descr));

  return std::make_pair(filt_feat, filt_descr);
}

} } } // end namespace

INSTANTIATE_ALGORITHM_DEF(kwiver::vital::algo::filter_features);
