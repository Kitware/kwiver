/*ckwg +29
 * Copyright 2014-2017 by Kitware, Inc.
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

#include <test_eigen.h>
#include <test_random_point.h>

#include <arrows/vxl/estimate_similarity_transform.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <vital/exceptions.h>
#include <vital/types/rotation.h>
#include <vital/types/similarity.h>
#include <vital/types/vector.h>

#include <iostream>
#include <vector>

using namespace kwiver::vital;

using std::cerr;
using std::endl;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(estimate_similarity, create)
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, algo::estimate_similarity_transform::create("vxl") );
}

// ----------------------------------------------------------------------------
TEST(estimate_similarity, not_enough_points)
{
  kwiver::arrows::vxl::estimate_similarity_transform est_ST;
  std::vector<vector_3d> from, to;
  EXPECT_THROW(
    est_ST.estimate_transform(from, to),
    algorithm_exception)
    << "Estimating with zero points";
}

// ----------------------------------------------------------------------------
TEST(estimate_similarity, uneven_sets)
{
  kwiver::arrows::vxl::estimate_similarity_transform est_ST;
  std::vector<vector_3d> from, to;
  vector_3d dummy_vec(0,0,0);

  from.push_back(dummy_vec);
  from.push_back(dummy_vec);
  to.push_back(dummy_vec);

  EXPECT_THROW(
    est_ST.estimate_transform(from, to),
    algorithm_exception)
    << "Estimating with uneven sets";
}

// ----------------------------------------------------------------------------
TEST(estimate_similarity, reprojection_100pts)
{
  // Process:
  //  1) generated points
  //  2) make up similarity transform and transform points
  //  3) generate similarity transform via estimation between two points sets
  //  4) check difference between crafted and estimated similarity transforms

  cerr << "Constructing 100 original, random points (std dev: 1.0)" << endl;
  std::vector<vector_3d> original_points;
  for (int i=0; i < 100; ++i)
  {
    original_points.push_back(kwiver::testing::random_point3d(1.0));
  }

  cerr << "Constructing crafted similarity transformation" << endl;
  similarity_d m_sim(5.623,
                     rotation_d(vector_3d(-1.4, 0.23, 1.7)),
                     vector_3d(2.24, 1.51, 4.23));

  cerr << "Transforming original points by crafted transformation" << endl;
  std::vector<vector_3d> transformed_points;
  for (vector_3d o_vec : original_points)
  {
    transformed_points.push_back(m_sim * o_vec);
  }

  cerr << "Estimating similarity transformation between point sets" << endl;
  kwiver::arrows::vxl::estimate_similarity_transform est_ST;
  similarity_d e_sim = est_ST.estimate_transform(original_points,
                                                 transformed_points);

  cerr << "Original Transform : " << m_sim << endl
       << "Estimated Transform: " << e_sim << endl
       << "Euclidian norm     : " << (m_sim.matrix() - e_sim.matrix()).norm() << endl;

  EXPECT_MATRIX_NEAR( m_sim.matrix(), e_sim.matrix(), 1e-12 )
    << "Crafted and estimated similarity transforms should match";

  cerr << "Constructing crafted similarity transformation WITH ZERO TRANSLATION" << endl;
  m_sim = similarity_d(5.623,
                       rotation_d(vector_3d(-1.4, 0.23, 1.7)),
                       vector_3d(0, 0, 0));

  cerr << "Transforming original points by crafted transformation" << endl;
  transformed_points.clear();
  for (vector_3d o_vec : original_points)
  {
    transformed_points.push_back(m_sim * o_vec);
  }

  cerr << "Estimating similarity transformation between point sets" << endl;
  e_sim = est_ST.estimate_transform(original_points, transformed_points);

  cerr << "Original Transform : " << m_sim << endl
       << "Estimated Transform: " << e_sim << endl
       << "Euclidian norm     : " << (m_sim.matrix() - e_sim.matrix()).norm() << endl;

  EXPECT_MATRIX_NEAR( m_sim.matrix(), e_sim.matrix(), 1e-12 )
    << "Crafted and estimated similarity transforms should match";
}

// ----------------------------------------------------------------------------
TEST(estimate_similarity, reprojection_4pts)
{
  // Process:
  //  1) generated points
  //  2) make up similarity transform and transform points
  //  3) generate similarity transform via estimation between two points sets
  //  4) check difference between crafted and estimated similarity transforms

  cerr << "Constructing 4 original, random points (std dev: 1.0)" << endl;
  std::vector<vector_3d> original_points;
  cerr << "Random points:" << endl;
  for (int i=0; i < 4; ++i)
  {
    original_points.push_back(kwiver::testing::random_point3d(1.0));
    cerr << "\t" << original_points.back() << endl;
  }

  cerr << "Constructing crafted similarity transformation" << endl;
  similarity_d m_sim(5.623,
                     rotation_d(vector_3d(-1.4, 0.23, 1.7)),
                     vector_3d(2.24, 1.51, 4.23));

  cerr << "Transforming original points by crafted transformation" << endl;
  std::vector<vector_3d> transformed_points;
  for (vector_3d o_vec : original_points)
  {
    transformed_points.push_back(m_sim * o_vec);
  }

  cerr << "Estimating similarity transformation between point sets" << endl;
  kwiver::arrows::vxl::estimate_similarity_transform est_ST;
  similarity_d e_sim = est_ST.estimate_transform(original_points,
                                                 transformed_points);

  cerr << "Original Transform : " << m_sim << endl
       << "Estimated Transform: " << e_sim << endl
       << "Euclidian norm     : " << (m_sim.matrix() - e_sim.matrix()).norm() << endl;

  EXPECT_MATRIX_NEAR( m_sim.matrix(), e_sim.matrix(), 1e-12 )
    << "Crafted and estimated similarity transforms should match";
}

// ----------------------------------------------------------------------------
TEST(estimate_similarity, reprojection_3pts)
{
  // Process:
  //  1) generated points
  //  2) make up similarity transform and transform points
  //  3) generate similarity transform via estimation between two points sets
  //  4) check difference between crafted and estimated similarity transforms

  cerr << "Constructing 3 original, random points (std dev: 1.0)" << endl;
  std::vector<vector_3d> original_points;
  cerr << "Random points:" << endl;
  for (int i=0; i < 3; ++i)
  {
    original_points.push_back(kwiver::testing::random_point3d(1.0));
    cerr << "\t" << original_points.back() << endl;
  }

  cerr << "Constructing crafted similarity transformation" << endl;
  similarity_d m_sim(5.623,
                     rotation_d(vector_3d(-1.4, 0.23, 1.7)),
                     vector_3d(2.24, 1.51, 4.23));

  cerr << "Transforming original points by crafted transformation" << endl;
  std::vector<vector_3d> transformed_points;
  for (vector_3d o_vec : original_points)
  {
    transformed_points.push_back(m_sim * o_vec);
  }

  cerr << "Estimating similarity transformation between point sets" << endl;
  kwiver::arrows::vxl::estimate_similarity_transform est_ST;
  similarity_d e_sim = est_ST.estimate_transform(original_points,
                                                 transformed_points);

  cerr << "Original Transform : " << m_sim << endl
       << "Estimated Transform: " << e_sim << endl
       << "Euclidian norm     : " << (m_sim.matrix() - e_sim.matrix()).norm() << endl;

  EXPECT_MATRIX_NEAR( m_sim.matrix(), e_sim.matrix(), 1e-12 )
    << "Crafted and estimated similarity transforms should match";
}

// ----------------------------------------------------------------------------
TEST(estimate_similarity, reprojection_100pts_noisy)
{
  // Process:
  //  1) generated points
  //  2) make up similarity transform and transform points
  //  3) generate similarity transform via estimation between two points sets
  //  4) check difference between crafted and estimated similarity transforms

  cerr << "Constructing 100 original, random points (std dev: 1.0)" << endl;
  std::vector<vector_3d> original_points;
  for (int i=0; i < 100; ++i)
  {
    original_points.push_back(kwiver::testing::random_point3d(1.0));
  }

  cerr << "Constructing crafted similarity transformation" << endl;
  similarity_d m_sim(5.623,
                     rotation_d(vector_3d(-1.4, 0.23, 1.7)),
                     vector_3d(2.24, 1.51, 4.23));

  cerr << "Transforming original points by crafted transformation" << endl;
  std::vector<vector_3d> transformed_points;
  for (vector_3d o_vec : original_points)
  {
    transformed_points.push_back((m_sim * o_vec) + kwiver::testing::random_point3d(0.01));
  }

  cerr << "Estimating similarity transformation between point sets" << endl;
  kwiver::arrows::vxl::estimate_similarity_transform est_ST;
  similarity_d e_sim = est_ST.estimate_transform(original_points,
                                                 transformed_points);

  cerr << "Original Transform : " << m_sim << endl
       << "Estimated Transform: " << e_sim << endl
       << "Euclidian norm     : " << (m_sim.matrix() - e_sim.matrix()).norm() << endl;

  EXPECT_MATRIX_NEAR( m_sim.matrix(), e_sim.matrix(), 1e-2 )
    << "Crafted and estimated similarity transforms should match";
}
