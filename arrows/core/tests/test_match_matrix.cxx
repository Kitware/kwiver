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

#include <test_tracks.h>

#include <arrows/core/match_matrix.h>
#include <vital/tests/test_track_set.h>

using namespace kwiver::vital;

// This can be removed prior to merging with source code
void view_set_matrix();

// ----------------------------------------------------------------------------
int main( int argc, char** argv )
{
  // This can be removed prior to merging with source code
  view_set_matrix();

  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

namespace {

// ----------------------------------------------------------------------------
// Helper function to generate deterministic track set
kwiver::vital::track_set_sptr
gen_set_tracks( unsigned frames=100,
                 unsigned max_tracks_per_frame=1000,
                 unsigned min_tracks_per_frame=500,
                 double termination_fraction = 0.01,
                 double skip_fraction = 0.01,
                 double frame_drop_fraction = 0.01 )
{

  // Manually terminate tracks on frames 0, 1 and 3
  track_id_t track_id=0;
  std::vector< track_sptr > all_tracks, active_tracks;
  for( unsigned f=0; f<frames; ++f )
  {

    // Create tracks as needed to get enough on this frame
    while( active_tracks.size() < max_tracks_per_frame )
    {
      auto t = track::create();
      t->set_id(track_id++);
      active_tracks.push_back(t);
      all_tracks.push_back(t);
    }

    // Add a state for each track to this frame
    for( auto t : active_tracks ) 
      {
        t->append( std::make_shared<track_state>( f ) );
      }

    if(f==0)
    {
      // Terminate tracks 0 and 3 on frame 0
      std::vector< track_sptr > next_tracks;
      for( auto t : active_tracks )
      {   
        if( t->id() != 0 && t->id() != 3 )
        {
          next_tracks.push_back( t );
        }
      }
      active_tracks.swap( next_tracks );
    }

    if(f==1)
    {
      // Terminate tracks 2 and 7 on frame 1
      std::vector< track_sptr > next_tracks;
      for( auto t : active_tracks )
      {
        if( t->id() != 2 && t->id() != 7 )
        {
          next_tracks.push_back( t );
        }
      }
      active_tracks.swap( next_tracks );
    }

    if(f==3)
    {
      // Terminate tracks 5 and 9 on frame 3
      std::vector< track_sptr > next_tracks;
      for( auto t : active_tracks )
      {
        if( t->id() != 5 && t->id() != 9 )
        {
          next_tracks.push_back( t );
        }
      }
      active_tracks.swap( next_tracks );
    }
  }
  return std::make_shared<track_set>( all_tracks );
}

// ----------------------------------------------------------------------------
// Function to generate match matrix with known values
Eigen::SparseMatrix<unsigned int> gen_test_matrix() 
{
  Eigen::Matrix<unsigned int, 5, 5> dense_matrix;

  // Manually calculated matrix from gen_set_tracks()
  dense_matrix << 8, 6, 4, 4, 3,
                  6, 8, 6, 6, 4,
                  4, 6, 8, 8, 6,
                  4, 6, 8, 8, 6,
                  3, 4, 6, 6, 8;

  // Convert the dense matrix to a sparse matrix for unit test comparison
  Eigen::SparseMatrix<unsigned int> test_matrix = dense_matrix.sparseView();

  return test_matrix;
}

// ----------------------------------------------------------------------------
// Function to calculate the max possible importance score
double gen_max_score(Eigen::SparseMatrix<unsigned int> matrix) 
{  
  double sum = 0.0;

  for (int row = 0; row < matrix.rows(); ++row) 
  {
      for (int col = 0; col <= row; ++col) {
          unsigned int value = matrix.coeff(row, col);
          if (value != 0) {
            sum += 1.0 / static_cast<double>(value);
          }
      }
  }
  return sum;
}

// ----------------------------------------------------------------------------
// Function to generate importance scores from known values for comparison
std::vector<double> gen_set_scores() 
{
  std::vector<double> set_scores;

  // Manually calculated for the 'set_tracks' and 'set_matrix'
  set_scores = {1.0/8, 8.0/3, 5.0/12, 1.0/8, 8.0/3, 1.625, 8.0/3,
                    5.0/12, 37.0/24, 5.0/6, 5.0/6, 5.0/6, 1.0/8, 1.0/8};

  return set_scores;
}

// ----------------------------------------------------------------------------
// Function to check range of elements in match matrix 
bool matrix_values(const Eigen::SparseMatrix<unsigned int>& matrix, 
                        unsigned int max_tracks) 
{
    for (int i = 0; i < matrix.rows(); ++i) {
        for (int k = 0; k < matrix.cols(); ++k) {
            unsigned int value = matrix.coeff(i, k);
            if (value < 0 || value > max_tracks) {
                return false;
            }
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// Establish constants and create variables for test_tracks

// These parameters can be varied for further testing
const unsigned int num_frames = 100;
const unsigned int max_tracks = 1000;

track_set_sptr test_tracks = 
  kwiver::testing::generate_tracks(num_frames, max_tracks);

const auto trks = test_tracks->tracks();

std::set<frame_id_t> frame_ids = test_tracks->all_frame_ids();
std::vector<frame_id_t> frames = 
  std::vector<frame_id_t>(frame_ids.begin(), frame_ids.end());

// Frames might dropped in track set generation
int actual_num_frames = test_tracks->all_frame_ids().size();
  
Eigen::SparseMatrix<unsigned int> matched_matrix = 
  kwiver::arrows::match_matrix(test_tracks, frames);

// ----------------------------------------------------------------------------
// Establish constants and create variables for set_tracks

// DO NOT EDIT these two constants, might cause unit tests to fail
const unsigned int set_num_frames = 5;
const unsigned int set_max_tracks = 8;

track_set_sptr set_tracks = 
  gen_set_tracks(set_num_frames, set_max_tracks);

const auto set_trks = set_tracks->tracks();

std::set<frame_id_t> set_frame_ids = set_tracks->all_frame_ids();
std::vector<frame_id_t> set_frames = 
  std::vector<frame_id_t>(set_frame_ids.begin(), set_frame_ids.end());

Eigen::SparseMatrix<unsigned int> set_matrix = 
  kwiver::arrows::match_matrix(set_tracks, set_frames);

std::map<track_id_t, double> set_importance_scores = 
  kwiver::arrows::match_matrix_track_importance(set_tracks, 
                                                set_frames, set_matrix);

} // end namespace anonymous

// ----------------------------------------------------------------------------
TEST(match_matrix, matrix_dimensions) 
{
  int num_rows = matched_matrix.rows();
  int num_cols = matched_matrix.cols();

  ASSERT_EQ(num_rows, actual_num_frames);
  ASSERT_EQ(num_cols, actual_num_frames);
}

// ----------------------------------------------------------------------------
// Test range of matrix values and symmetry
TEST(match_matrix, matrix_values) 
{
  EXPECT_TRUE(matrix_values(matched_matrix, max_tracks));
  EXPECT_TRUE(matched_matrix.isApprox(matched_matrix.transpose()));
}

// ----------------------------------------------------------------------------
// Test matrix diagonal values match the number of tracks in each frame
TEST(match_matrix, diagonal_values) 
{
  std::vector<unsigned int> tracks_in_frame(actual_num_frames, 0);

  for (const auto& t : trks) {
    std::set<frame_id_t> t_frames = t->all_frame_ids();
    for (const auto& fid : t_frames) {
        tracks_in_frame[fid]++;
    }
  }
  
  std::vector<unsigned int> diag_elements(actual_num_frames, 0);

  for (Eigen::Index i = 0; i < matched_matrix.rows(); ++i) {
    diag_elements[i] =(matched_matrix.coeff(i, i));
  }

  EXPECT_EQ(diag_elements, tracks_in_frame);
}

// ----------------------------------------------------------------------------
// Test that match_matrix() function is equivalent to calculated matrix
TEST(match_matrix, test_matrix) 
{
  Eigen::SparseMatrix<unsigned int> test_matrix = gen_test_matrix();

  ASSERT_TRUE(set_matrix.isApprox(test_matrix));
}

// ----------------------------------------------------------------------------
TEST(importance_score, vector_size) 
{
  std::map<track_id_t, double> importance_scores = 
  kwiver::arrows::match_matrix_track_importance(test_tracks, 
                                                frames, matched_matrix);

  double max_score = gen_max_score(matched_matrix);

  std::vector<double> score_values;
  for (const auto& entry : importance_scores) 
  {
    score_values.push_back(entry.second);
  }

  double largest_score = *std::max_element(score_values.begin(),
                                            score_values.end());

  EXPECT_EQ(test_tracks->size(), importance_scores.size());
  EXPECT_LE(largest_score, max_score);
}

// ----------------------------------------------------------------------------
// Test importance score function against pre-determined result 
TEST(importance_score, score_values) 
{
    // invoke the importance scores that were manually calculated
    auto set_scores = gen_set_scores();

    std::vector<double> score_values;
    for (const auto& entry : set_importance_scores) 
    {
        score_values.push_back(entry.second);
    }
  
    const double tolerance = 1e-5;

    for (size_t i = 0; i < set_scores.size(); ++i) 
    {
        EXPECT_NEAR(set_scores[i], score_values[i], tolerance);
    }
}

// ----------------------------------------------------------------------------
// Function to view results for a small track set
// Used for visual inspection, manual calculations and de-bugging
// Can be removed before merging with source code
void view_set_matrix() 
{
    std::cout << "Deterministic track set"<<std::endl;

    // Frames might be dropped in track set generation
    int actual_set_num_frames = set_tracks->all_frame_ids().size();

    // View each frame and associated tracks
    for (frame_id_t f_id = 0; f_id < actual_set_num_frames; ++f_id) {

        std::cout << "Frame " << f_id << " - Tracks: ";

        for (const auto& t : set_trks) {
            // Get all frames covered by this track
            std::set<frame_id_t> t_frames = t->all_frame_ids();

            // Check if the desired frame is in the set of 
            // frames covered by the track
            if (t_frames.find(f_id) != t_frames.end()) {
                std::cout << t->id() << " ";
            }
        }

        std::cout << std::endl;
    }

    std::cout << '\n';

    std::cout << "Deterministic matched matrix\n" << set_matrix << std::endl;

    std::cout << "Track Importance Scores:\n";
    for (const auto& entry : set_importance_scores) {
      std::cout <<"Track ID: "<<entry.first << ", Score: "<<entry.second <<"\n";
    }

    std::cout << '\n';
}

