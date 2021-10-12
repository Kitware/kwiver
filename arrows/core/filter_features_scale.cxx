/*ckwg +29
 * Copyright 2017-2018 by Kitware, Inc.
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
 * \brief Implementation of filter_features_scale algorithm
 */
#include "filter_features_scale.h"

#include <algorithm>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace core {

//Helper struct for the filter function
struct feature_at_index_is_greater
{
  bool operator()(std::pair<unsigned int, double> const &l,
                  std::pair<unsigned int, double> const &r)
  {
    return l.second > r.second;
  }
};


// Private implementation class
class filter_features_scale::priv
{
public:
  // Constructor
  priv()
    : top_fraction(0.2),
      min_features(100),
      max_features(1000)
  {
  }


// ----------------------------------------------------------------------------
  feature_set_sptr
  filter(feature_set_sptr feat, std::vector<unsigned int> &ind) const
  {
    const std::vector<feature_sptr> &feat_vec = feat->features();
    ind.clear();
    if (feat_vec.size() <= min_features)
    {
      ind.resize(feat_vec.size());
      for (unsigned int i=0; i<ind.size(); ++i)
      {
        ind[i] = i;
      }
      return feat;
    }

    //  Create a new vector with the index and scale for faster sorting
    std::vector<std::pair<unsigned int, double> > indices;
    indices.reserve(feat_vec.size());
    for (unsigned int i = 0; i < feat_vec.size(); i++)
    {
      indices.push_back(std::make_pair(i, feat_vec[i]->scale()));
    }

    // compute threshold
    unsigned int cutoff = std::max(min_features,
        static_cast<unsigned int>(top_fraction * indices.size()));
    if (max_features > 0 )
    {
      cutoff = std::min(cutoff, max_features);
    }

    // partially sort on descending feature scale
    std::nth_element(indices.begin(), indices.begin()+cutoff, indices.end(),
                     feature_at_index_is_greater());

    std::vector<feature_sptr> filtered(cutoff);
    ind.resize(cutoff);
    for (unsigned int i = 0; i < cutoff; i++)
    {
      unsigned int index = indices[i].first;
      ind[i] = index;
      filtered[i] = feat_vec[index];
    }

    LOG_INFO( m_logger,
              "Reduced " << feat_vec.size() << " features to " <<
              filtered.size() << " features.");

    return std::make_shared<vital::simple_feature_set>(
        vital::simple_feature_set(filtered));
  }

  double top_fraction;
  unsigned int min_features;
  unsigned int max_features;
  vital::logger_handle_t m_logger;
};


// ----------------------------------------------------------------------------
// Constructor
filter_features_scale
::filter_features_scale()
: d_(new priv)
{
  attach_logger( "arrows.core.filter_features_scale" );
  d_->m_logger = logger();
}


// Destructor
filter_features_scale
::~filter_features_scale()
{
}


// ----------------------------------------------------------------------------
// Get this algorithm's \link vital::config_block configuration block \endlink
  vital::config_block_sptr
filter_features_scale
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config =
      vital::algo::filter_features::get_configuration();

  config->set_value("top_fraction", d_->top_fraction,
                    "Fraction of largest scale keypoints to keep, range (0.0, 1.0]");

  config->set_value("min_features", d_->min_features,
                    "Minimum number of features to keep");

  config->set_value("max_features", d_->max_features,
                    "Maximum number of features to keep, use 0 for unlimited");

  return config;
}


// ----------------------------------------------------------------------------
// Set this algorithm's properties via a config block
void
filter_features_scale
::set_configuration(vital::config_block_sptr config)
{
  d_->top_fraction = config->get_value<double>("top_fraction", d_->top_fraction);
  d_->min_features = config->get_value<unsigned int>("min_features", d_->min_features);
  d_->max_features = config->get_value<unsigned int>("max_features", d_->max_features);
}


// Check that the algorithm's configuration vital::config_block is valid
bool
filter_features_scale
::check_configuration(vital::config_block_sptr config) const
{
  double top_fraction = config->get_value<double>("top_fraction", d_->top_fraction);
  if( top_fraction <= 0.0 || top_fraction > 1.0 )
  {
    LOG_ERROR( logger(),
             "top_fraction parameter is " << top_fraction << ", needs to be in (0.0, 1.0].");
    return false;
  }

  unsigned int min_features =
    config->get_value<unsigned int>("min_features", d_->min_features);
  unsigned int max_features =
    config->get_value<unsigned int>("max_features", d_->max_features);
  if( max_features > 0 && max_features < min_features )
  {
    LOG_ERROR( logger(), "max_features (" << max_features
                             << ") must be zero or greater than min_features ("
                             << min_features << ")" );
    return false;
  }
  return true;
}


// ----------------------------------------------------------------------------
// Filter feature set
vital::feature_set_sptr
filter_features_scale
::filter(vital::feature_set_sptr feat, std::vector<unsigned int> &indices) const
{
  return d_->filter(feat, indices);
}

} // end namespace core
} // end namespace arrows
} // end namespace kwiver
