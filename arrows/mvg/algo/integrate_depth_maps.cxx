// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
* \file
* \brief Source file for integration of depth maps using a voxel grid
*/

#include <arrows/mvg/algo/integrate_depth_maps.h>
#include <arrows/core/depth_utils.h>

#include <vital/util/transform_image.h>

#include <sstream>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace mvg {

/// Private implementation class
class integrate_depth_maps::priv
{
public:
  // Constructor
  priv()
    : ray_potential_rho(1.0),
      ray_potential_thickness(20.0),
      ray_potential_eta(1.0),
      ray_potential_epsilon(0.01),
      ray_potential_delta(10.0),
      grid_spacing {1.0, 1.0, 1.0},
      voxel_spacing_factor(1.0),
      m_logger(vital::get_logger("arrows.mvg.integrate_depth_maps"))
  {
  }

  // integrate a depth image into the integration volume
  void integrate_depth_map(image_of<double>& volume,
                           camera_perspective const& camera,
                           image_of<double> const& depth,
                           image_of<double> const& weight,
                           vector_3d const& origin,
                           vector_3d const& spacing) const;

  // compute the TSDF ray potential given an estimated depth and real depth
  double ray_potential(double est_depth, double real_depth) const;

  double ray_potential_rho;
  double ray_potential_thickness;
  double ray_potential_eta;
  double ray_potential_epsilon;
  double ray_potential_delta;

  int grid_dims[3];

  // Actual spacing is computed as
  //   voxel_scale_factor * pixel_to_world_scale * grid_spacing
  // relative spacings per dimension
  double grid_spacing[3];

  // multiplier on all dimensions of grid spacing
  double voxel_spacing_factor;

  double const_thickness;
  double const_delta;
  double const_slope;
  double const_freespace_val;
  double const_occluded_val;

  // Logger handle
  vital::logger_handle_t m_logger;
};

// ----------------------------------------------------------------------------

// integrate a depth image into the integration volume
void
integrate_depth_maps::priv
::integrate_depth_map(image_of<double>& volume,
                      camera_perspective const& camera,
                      image_of<double> const& depth,
                      image_of<double> const& weight,
                      vector_3d const& origin,
                      vector_3d const& spacing) const
{
  auto const ni = volume.width();
  auto const nj = volume.height();
  auto const nk = static_cast<int64_t>(volume.depth());

  auto const w = depth.width();
  auto const h = depth.height();

  #pragma omp parallel for
  for (int64_t k = 0; k < nk; ++k)
  {
    double const z = origin[2] + (k + 0.5) * spacing[2];
    for (size_t j = 0; j < nj; ++j)
    {
      double const y = origin[1] + (j + 0.5) * spacing[1];
      for (size_t i = 0; i < ni; ++i)
      {
        double const x = origin[0] + (i + 0.5) * spacing[0];
        vector_3d world_pt = vector_3d{ x, y, z };
        vector_2d image_pt = camera.project(world_pt);
        int const u = static_cast<int>(std::round(image_pt.x()));
        int const v = static_cast<int>(std::round(image_pt.y()));
        vector_2i pixel = image_pt.cast<int>();
        if (u < 0 || u >= w ||
            v < 0 || v >= h)
        {
          continue;
        }
        double const& d = depth(u, v);
        double const a = (weight.size() > 0) ? weight(u, v) : 1.0;

        if (d <= 0.0 || a <= 0.0)
        {
          continue;
        }
        double real_d = camera.depth(world_pt);
        volume(i, j, k) += a * ray_potential(d, real_d);
      }
    }
  }
}

// ----------------------------------------------------------------------------

// compute the TSDF ray potential given an estimated depth and real depth
double
integrate_depth_maps::priv
::ray_potential(double est_depth, double real_depth) const
{
  double diff = real_depth - est_depth;

  double abs_diff = std::abs(diff);

  if (abs_diff > const_delta)
  {
    return diff > 0.0 ? const_occluded_val
                      : const_freespace_val;
  }
  else if (abs_diff > const_thickness)
  {
    return std::copysign(ray_potential_rho, diff);
  }

  return const_slope * diff;
}

// ----------------------------------------------------------------------------

/// Constructor
integrate_depth_maps::integrate_depth_maps()
  : d_(new priv)
{
}

// ----------------------------------------------------------------------------

/// Destructor
integrate_depth_maps::~integrate_depth_maps()
{
}

// ----------------------------------------------------------------------------

/// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
integrate_depth_maps::get_configuration() const
{
  // get base config from base class
  auto config = vital::algo::integrate_depth_maps::get_configuration();

  config->set_value("ray_potential_thickness", d_->ray_potential_thickness,
                    "Distance that the TSDF covers sloping from Rho to zero. "
                    "Units are in voxels.");
  config->set_value("ray_potential_rho", d_->ray_potential_rho,
                    "Maximum magnitude of the TDSF");
  config->set_value("ray_potential_eta", d_->ray_potential_eta,
                    "Fraction of rho to use for free space constraint. "
                    "Requires 0 <= Eta <= 1.");
  config->set_value("ray_potential_epsilon", d_->ray_potential_epsilon,
                    "Fraction of rho to use in occluded space. "
                    "Requires 0 <= Epsilon <= 1.");
  config->set_value("ray_potential_delta", d_->ray_potential_delta,
                    "Distance from the surface before the TSDF is truncate. "
                    "Units are in voxels");
  config->set_value("voxel_spacing_factor", d_->voxel_spacing_factor,
                    "Multiplier on voxel spacing.  Set to 1.0 for voxel "
                    "sizes that project to 1 pixel on average.");

  std::ostringstream stream;
  stream << d_->grid_spacing[0] << " "
         << d_->grid_spacing[1] << " "
         << d_->grid_spacing[2];
  config->set_value("grid_spacing", stream.str(),
                    "Relative spacing for each dimension of the grid");

  return config;
}

// ----------------------------------------------------------------------------

/// Set this algorithm's properties via a config block
void
integrate_depth_maps::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated vital::config_block to ensure that
  // assumed values are present. An alternative is to check for key
  // presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  d_->ray_potential_rho =
    config->get_value<double>("ray_potential_rho", d_->ray_potential_rho);
  d_->ray_potential_thickness =
    config->get_value<double>("ray_potential_thickness",
                              d_->ray_potential_thickness);
  d_->ray_potential_eta =
    config->get_value<double>("ray_potential_eta", d_->ray_potential_eta);
  d_->ray_potential_epsilon =
    config->get_value<double>("ray_potential_epsilon", d_->ray_potential_epsilon);
  d_->ray_potential_delta =
    config->get_value<double>("ray_potential_delta", d_->ray_potential_delta);
  d_->voxel_spacing_factor =
    config->get_value<double>("voxel_spacing_factor", d_->voxel_spacing_factor);

  std::ostringstream ostream;
  ostream << d_->grid_spacing[0] << " "
          << d_->grid_spacing[1] << " "
          << d_->grid_spacing[2];
  std::string spacing =
    config->get_value<std::string>("grid_spacing", ostream.str());
  std::istringstream istream(spacing);
  istream >> d_->grid_spacing[0] >> d_->grid_spacing[1] >> d_->grid_spacing[2];
}

// ----------------------------------------------------------------------------

/// Check that the algorithm's currently configuration is valid
bool
integrate_depth_maps::check_configuration(vital::config_block_sptr config) const
{
  return true;
}

// ----------------------------------------------------------------------------

void
integrate_depth_maps::integrate(
  vector_3d const& minpt_bound,
  vector_3d const& maxpt_bound,
  std::vector<image_container_sptr> const& depth_maps,
  std::vector<image_container_sptr> const& weight_maps,
  std::vector<camera_perspective_sptr> const& cameras,
  image_container_sptr& volume,
  vector_3d &spacing) const
{
  double pixel_to_world_scale;
  pixel_to_world_scale =
    kwiver::arrows::core::
      compute_pixel_to_world_scale(minpt_bound, maxpt_bound, cameras);

  vector_3d diff = maxpt_bound - minpt_bound;
  vector_3d orig = minpt_bound;

  spacing = vector_3d(d_->grid_spacing);
  spacing *= pixel_to_world_scale * d_->voxel_spacing_factor;
  double max_spacing = spacing.maxCoeff();

  // precompute constants to make ray potential computation more efficient
  d_->const_delta = d_->ray_potential_delta * max_spacing;
  d_->const_thickness = d_->ray_potential_thickness * max_spacing;
  d_->const_slope = d_->ray_potential_rho / d_->const_thickness;
  d_->const_freespace_val = -d_->ray_potential_eta * d_->ray_potential_rho;
  d_->const_occluded_val = d_->ray_potential_epsilon * d_->ray_potential_rho;

  for (int i = 0; i < 3; i++)
  {
    d_->grid_dims[i] = static_cast<int>((diff[i] / spacing[i]));
  }

  LOG_DEBUG( logger(), "voxel size: " << spacing[0]
                       << " "         << spacing[1]
                       << " "         << spacing[2] );
  LOG_DEBUG( logger(), "grid: " << d_->grid_dims[0]
                       << " "   << d_->grid_dims[1]
                       << " "   << d_->grid_dims[2] );

  LOG_INFO( logger(), "initialize volume" );
  image_of<double> voxel_grid;
  if (volume)
  {
    voxel_grid = volume->get_image();
  }
  voxel_grid.set_size(d_->grid_dims[0],
                      d_->grid_dims[1],
                      d_->grid_dims[2]);

  // fill volume with zeros
  transform_image(voxel_grid, [] (double v) { return 0.0; });

  for (size_t i = 0; i < depth_maps.size(); ++i)
  {
    image_of<double> depth{ depth_maps[i]->get_image() };
    image_of<double> weight;
    if (i < weight_maps.size())
    {
      auto const& w = weight_maps[i];
      if (w->width() == depth.width() &&
          w->height() == depth.height())
      {
        weight = weight_maps[i]->get_image();
      }
    }
    if (i >= cameras.size() || !cameras[i])
    {
      continue;
    }

    // integrate depthms
    LOG_INFO( logger(), "depth map " << i );
    d_->integrate_depth_map(voxel_grid, *cameras[i], depth, weight,
                            orig, spacing);
  }

  volume = std::make_shared<simple_image_container>(voxel_grid);
}

} // end namespace mvg
} // end namespace arrows
} // end namespace kwiver