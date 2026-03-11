// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test match matrix with generated track sets.

#include <test_tracks.h>

#include <arrows/core/match_matrix.h>
#include <vital/tests/test_track_set.h>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

namespace {

// ----------------------------------------------------------------------------
// Function to generate match matrix with known values
Eigen::SparseMatrix< size_t >
gen_test_matrix()
{
  Eigen::Matrix< size_t, 5, 5 > dense_matrix;

  // Manually calculated matrix from gen_set_tracks()
  dense_matrix <<
    8, 6, 4, 4, 3,
    6, 8, 6, 6, 4,
    4, 6, 8, 8, 6,
    4, 6, 8, 8, 6,
    3, 4, 6, 6, 8;

  // Convert the dense matrix to a sparse matrix for unit test comparison
  Eigen::SparseMatrix< size_t > test_matrix = dense_matrix.sparseView();

  return test_matrix;
}

// ----------------------------------------------------------------------------
// Function to calculate the max possible importance score
double
gen_max_score( Eigen::SparseMatrix< size_t > matrix )
{
  double sum = 0.0;

  for( int row = 0; row < matrix.rows(); ++row )
  {
    for( int col = 0; col <= row; ++col )
    {
      size_t const value = matrix.coeff( row, col );
      if( value != 0 )
      {
        sum += 1.0 / static_cast< double >( value );
      }
    }
  }
  return sum;
}

// ----------------------------------------------------------------------------
// Function to generate importance scores from known values for comparison
std::vector< double >
gen_set_scores()
{
  std::vector< double > set_scores;

  // Manually calculated for the 'set_tracks' and 'set_matrix'
  set_scores = { 1.0 / 8, 8.0 / 3, 5.0 / 12, 1.0 / 8, 8.0 / 3, 1.625, 8.0 / 3,
                 5.0 / 12, 37.0 / 24, 5.0 / 6, 5.0 / 6, 5.0 / 6, 1.0 / 8,
                 1.0 / 8 };

  return set_scores;
}

// ----------------------------------------------------------------------------
// Function to check range of elements in match matrix
bool
matrix_values(
  const Eigen::SparseMatrix< size_t >& matrix,
  size_t max_tracks )
{
  for( int i = 0; i < matrix.rows(); ++i )
  {
    for( int k = 0; k < matrix.cols(); ++k )
    {
      auto const value = matrix.coeff( i, k );
      if( value < 0 || value > max_tracks )
      {
        return false;
      }
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
// Establish constants and create variables for test_tracks

// These two parameters can be varied for further testing
size_t const num_frames = 100;
size_t const max_tracks = 1000;

track_set_sptr test_tracks =
  kwiver::testing::generate_tracks( num_frames, max_tracks );

std::set< frame_id_t > frame_ids = test_tracks->all_frame_ids();
std::vector< frame_id_t > frames =
  std::vector< frame_id_t >( frame_ids.begin(), frame_ids.end() );

// Frames might be dropped in track set generation
int actual_num_frames = test_tracks->all_frame_ids().size();

Eigen::SparseMatrix< size_t > matched_matrix =
  kwiver::arrows::match_matrix( test_tracks, frames );

// ----------------------------------------------------------------------------
// Establish constants and create variables for set_tracks
// set_tracks are a determnistic track set

// DO NOT EDIT these two constants, might cause unit tests to fail
size_t const set_num_frames = 5;
size_t const set_max_tracks = 8;

track_set_sptr set_tracks =
  kwiver::testing::gen_set_tracks( set_num_frames, set_max_tracks );

std::set< frame_id_t > set_frame_ids = set_tracks->all_frame_ids();
std::vector< frame_id_t > set_frames =
  std::vector< frame_id_t >( set_frame_ids.begin(), set_frame_ids.end() );

Eigen::SparseMatrix< size_t > set_matrix =
  kwiver::arrows::match_matrix( set_tracks, set_frames );

std::map< track_id_t, double > set_importance_scores =
  kwiver::arrows::match_matrix_track_importance(
    set_tracks,
    set_frames, set_matrix );

} // end namespace anonymous

// ----------------------------------------------------------------------------
TEST ( match_matrix, matrix_dimensions )
{
  int num_rows = matched_matrix.rows();
  int num_cols = matched_matrix.cols();

  ASSERT_EQ( num_rows, actual_num_frames );
  ASSERT_EQ( num_cols, actual_num_frames );
}

// ----------------------------------------------------------------------------
// Test range of matrix values and symmetry
TEST ( match_matrix, matrix_values )
{
  EXPECT_TRUE( matrix_values( matched_matrix, max_tracks ) );
  EXPECT_TRUE( matched_matrix.isApprox( matched_matrix.transpose() ) );
}

// ----------------------------------------------------------------------------
// Test matrix diagonal values match the number of tracks in each frame
TEST ( match_matrix, diagonal_values )
{
  std::vector< size_t > tracks_in_frame( actual_num_frames, 0 );

  for( const auto& t : test_tracks->tracks() )
  {
    std::set< frame_id_t > t_frames = t->all_frame_ids();
    for( const auto& fid : t_frames )
    {
      tracks_in_frame[ fid ]++;
    }
  }

  std::vector< size_t > diag_elements( actual_num_frames, 0 );

  for( Eigen::Index i = 0; i < matched_matrix.rows(); ++i )
  {
    diag_elements[ i ] = ( matched_matrix.coeff( i, i ) );
  }

  EXPECT_EQ( diag_elements, tracks_in_frame );
}

// ----------------------------------------------------------------------------
// Test that match_matrix() function is equivalent to calculated matrix
TEST ( match_matrix, test_matrix )
{
  Eigen::SparseMatrix< size_t > test_matrix = gen_test_matrix();

  ASSERT_TRUE( set_matrix.isApprox( test_matrix ) );
}

// ----------------------------------------------------------------------------
TEST ( importance_score, vector_size )
{
  std::map< track_id_t, double > importance_scores =
    kwiver::arrows::match_matrix_track_importance(
      test_tracks,
      frames, matched_matrix );

  double max_score = gen_max_score( matched_matrix );

  std::vector< double > score_values;
  for( const auto& entry : importance_scores )
  {
    score_values.push_back( entry.second );
  }

  double largest_score = *std::max_element(
    score_values.begin(),
    score_values.end() );

  EXPECT_EQ( test_tracks->size(), importance_scores.size() );
  EXPECT_LE( largest_score, max_score );
}

// ----------------------------------------------------------------------------
// Test importance score function against pre-determined result
TEST ( importance_score, score_values )
{
  // invoke the importance scores that were manually calculated
  auto set_scores = gen_set_scores();

  std::vector< double > score_values;
  for( const auto& entry : set_importance_scores )
  {
    score_values.push_back( entry.second );
  }

  const double tolerance = 1e-5;

  for( size_t i = 0; i < set_scores.size(); ++i )
  {
    EXPECT_NEAR( set_scores[ i ], score_values[ i ], tolerance );
  }
}
