// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
* \file
* \brief Implementation for kwiver::vital::sfm_constraints class storing
*        constraints to be used in SfM.
*/

#include <vital/types/sfm_constraints.h>

#include <vital/math_constants.h>
#include <vital/types/rotation.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
class sfm_constraints::priv
{
public:
  struct im_size
  {
    unsigned width;
    unsigned height;
  };

  metadata_map_sptr m_md;
  local_geo_cs m_lgcs;
  std::map<frame_id_t, im_size> m_image_sizes;
};

// ----------------------------------------------------------------------------
sfm_constraints
::sfm_constraints(const sfm_constraints& other)
  : m_priv{new priv{*other.m_priv}}
{
}

// ----------------------------------------------------------------------------
sfm_constraints
::sfm_constraints()
  : m_priv(new priv)
{
}

// ----------------------------------------------------------------------------
sfm_constraints
::sfm_constraints( metadata_map_sptr md,
                   local_geo_cs const& lgcs)
  :m_priv(new priv)
{
  m_priv->m_md = md;
  m_priv->m_lgcs = lgcs;
}

// ----------------------------------------------------------------------------
sfm_constraints
::~sfm_constraints()
{
}

// ----------------------------------------------------------------------------
metadata_map_sptr
sfm_constraints
::get_metadata()
{
  return m_priv->m_md;
}

// ----------------------------------------------------------------------------
void
sfm_constraints
::set_metadata(metadata_map_sptr md)
{
  m_priv->m_md = md;
}

// ----------------------------------------------------------------------------
local_geo_cs
sfm_constraints
::get_local_geo_cs()
{
  return m_priv->m_lgcs;
}

// ----------------------------------------------------------------------------
void
sfm_constraints
::set_local_geo_cs(local_geo_cs const& lgcs)
{
  m_priv->m_lgcs = lgcs;
}

// ----------------------------------------------------------------------------
optional<float>
sfm_constraints
::get_focal_length_prior(frame_id_t fid) const
{
  if (!m_priv->m_md)
  {
    return nullopt;
  }

  auto &md = *m_priv->m_md;

  std::set<frame_id_t> frame_ids_to_try;

  auto const& image_width = get_image_width(fid);
  if (!image_width)
  {
    return nullopt;
  }
  auto const image_width_double = static_cast<double>(*image_width);

  if (fid >= 0)
  {
    frame_ids_to_try.insert(fid);
  }
  else
  {
    frame_ids_to_try = md.frames();
  }

  std::vector<double> focal_lengths;
  for (auto test_fid : frame_ids_to_try)
  {
    if ( md.has<VITAL_META_SENSOR_HORIZONTAL_FOV>(test_fid) )
    {
      double hfov = md.get<VITAL_META_SENSOR_HORIZONTAL_FOV>(test_fid);
      focal_lengths.push_back(static_cast<float>(
        (image_width_double * 0.5) / tan(0.5 * hfov * deg_to_rad)));
      continue;
    }

    if ( md.has<VITAL_META_TARGET_WIDTH>(test_fid) &&
         md.has<VITAL_META_SLANT_RANGE>(test_fid) )
    {
      auto const focal_length =
        static_cast<float>(
          image_width_double *
          md.get<VITAL_META_SLANT_RANGE>(test_fid) /
          md.get<VITAL_META_TARGET_WIDTH>(test_fid) );
      focal_lengths.push_back(focal_length);
      continue;
    }
  }
  if (focal_lengths.empty())
  {
    return nullopt;
  }
  // compute the median focal length
  std::nth_element(focal_lengths.begin(),
                   focal_lengths.begin() + focal_lengths.size() / 2,
                   focal_lengths.end());
  return focal_lengths[focal_lengths.size() / 2];
}

// ----------------------------------------------------------------------------
optional<rotation_d>
sfm_constraints
::get_camera_orientation_prior_local(frame_id_t fid) const
{
  if (m_priv->m_lgcs.origin().is_empty())
  {
    return nullopt;
  }

  if (!m_priv->m_md)
  {
    return nullopt;
  }

  auto &md = *m_priv->m_md;

  if ( md.has<VITAL_META_PLATFORM_HEADING_ANGLE>(fid) &&
       md.has<VITAL_META_PLATFORM_ROLL_ANGLE>(fid) &&
       md.has<VITAL_META_PLATFORM_PITCH_ANGLE>(fid) &&
       md.has<VITAL_META_SENSOR_REL_AZ_ANGLE>(fid) &&
       md.has<VITAL_META_SENSOR_REL_EL_ANGLE>(fid) )
  {
    double platform_heading = md.get<VITAL_META_PLATFORM_HEADING_ANGLE>(fid);
    double platform_roll = md.get<VITAL_META_PLATFORM_ROLL_ANGLE>(fid);
    double platform_pitch = md.get<VITAL_META_PLATFORM_PITCH_ANGLE>(fid);
    double sensor_rel_az = md.get<VITAL_META_SENSOR_REL_AZ_ANGLE>(fid);
    double sensor_rel_el = md.get<VITAL_META_SENSOR_REL_EL_ANGLE>(fid);

    double sensor_rel_roll = 0;
    if ( md.has<VITAL_META_SENSOR_REL_ROLL_ANGLE>(fid) )
    {
      sensor_rel_roll = md.get<VITAL_META_SENSOR_REL_ROLL_ANGLE>(fid);
    }

    if (std::isfinite(platform_heading) && std::isfinite(platform_pitch) &&
        std::isfinite(platform_roll) && std::isfinite(sensor_rel_az) &&
        std::isfinite(sensor_rel_el) && std::isfinite(sensor_rel_roll))
    {
      return compose_rotations<double>(
        platform_heading, platform_pitch, platform_roll,
        sensor_rel_az, sensor_rel_el, sensor_rel_roll);
    }
  }

  return nullopt;
}

// ----------------------------------------------------------------------------
optional<vector_3d>
sfm_constraints
::get_camera_position_prior_local(frame_id_t fid) const
{
  if (m_priv->m_lgcs.origin().is_empty())
  {
    return nullopt;
  }

  if (!m_priv->m_md)
  {
    return nullopt;
  }

  kwiver::vital::geo_point gloc;
  if (m_priv->m_md->has<VITAL_META_SENSOR_LOCATION>(fid))
  {
    gloc = m_priv->m_md->get<VITAL_META_SENSOR_LOCATION>(fid);
  }
  else
  {
    return nullopt;
  }

  auto geo_origin = m_priv->m_lgcs.origin();
  vector_3d loc = gloc.location(geo_origin.crs());
  loc -= geo_origin.location();

  return loc;
}

// ----------------------------------------------------------------------------
sfm_constraints::position_map
sfm_constraints
::get_camera_position_priors() const
{
  position_map local_positions;

  if (!m_priv->m_md)
  {
    return local_positions;
  }

  for (auto mdv : m_priv->m_md->metadata())
  {
    auto fid = mdv.first;

    auto const& loc = get_camera_position_prior_local(fid);
    if (!loc)
    {
      continue;
    }
    if (local_positions.empty())
    {
      local_positions[fid] = *loc;
    }
    else
    {
      auto last_loc = local_positions.crbegin()->second;
      if (*loc == last_loc)
      {
        continue;
      }
      local_positions[fid] = *loc;
    }
  }
  return local_positions;
}

// ----------------------------------------------------------------------------
void
sfm_constraints
::store_image_size(
  frame_id_t fid, unsigned image_width, unsigned image_height)
{
  m_priv->m_image_sizes[fid] = {image_width, image_height};
}

// ----------------------------------------------------------------------------
optional<unsigned>
sfm_constraints
::get_image_height(frame_id_t fid) const
{
  if (fid >= 0)
  {
    auto const data_it = m_priv->m_image_sizes.find(fid);
    if (data_it == m_priv->m_image_sizes.end())
    {
      return nullopt;
    }
    return data_it->second.height;
  }
  else
  {
    if (m_priv->m_image_sizes.empty())
    {
      return nullopt;
    }
    return m_priv->m_image_sizes.begin()->second.height;
  }
}

// ----------------------------------------------------------------------------
optional<unsigned>
sfm_constraints
::get_image_width(frame_id_t fid) const
{
  if (fid >= 0)
  {
    auto const data_it = m_priv->m_image_sizes.find(fid);
    if (data_it == m_priv->m_image_sizes.end())
    {
      return nullopt;
    }
    return data_it->second.width;
  }
  else
  {
    if (m_priv->m_image_sizes.empty())
    {
      return nullopt;
    }
    return m_priv->m_image_sizes.begin()->second.width;
  }
}

} // namespace vital

} // namespace kwiver
