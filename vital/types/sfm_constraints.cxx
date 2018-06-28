#include <vital/types/sfm_constraints.h>

namespace kwiver {
namespace vital {

/// Private implementation class
class sfm_constraints::priv
{
public:
  /// Constructor
  priv();

  /// Destructor
  ~priv();

  metadata_map_sptr m_md;
  local_geo_cs m_lgcs;
};

sfm_constraints::priv
::priv()
{

}

sfm_constraints::priv
::~priv()
{

}

sfm_constraints
::sfm_constraints(const sfm_constraints& other)
  : m_priv(new priv)
{
  m_priv->m_lgcs = other.m_priv->m_lgcs;
  m_priv->m_md = other.m_priv->m_md;
}

sfm_constraints
::sfm_constraints()
  : m_priv(new priv)
{

}

sfm_constraints
::sfm_constraints( metadata_map_sptr md,
                   local_geo_cs const& lgcs)
  :m_priv(new priv)
{
  m_priv->m_md = md;
  m_priv->m_lgcs = lgcs;
}

sfm_constraints
::~sfm_constraints()
{

}

metadata_map_sptr
sfm_constraints
::get_metadata()
{
  return m_priv->m_md;
}

void
sfm_constraints
::set_metadata(metadata_map_sptr md)
{
  m_priv->m_md = md;
}

local_geo_cs
sfm_constraints
::get_local_geo_cs()
{
  return m_priv->m_lgcs;
}

void
sfm_constraints
::set_local_geo_cs(local_geo_cs const& lgcs)
{
  m_priv->m_lgcs = lgcs;
}

bool
sfm_constraints
::get_camera_orientation_prior_local(frame_id_t fid,
                                     rotation_d &R_loc) const
{
  if (m_priv->m_lgcs.origin().is_empty())
  {
    return false;
  }

  if (!m_priv->m_md)
  {
    return false;
  }

  auto &md = *m_priv->m_md;

  double platform_heading, platform_roll,   platform_pitch;
  double sensor_rel_az,    sensor_rel_roll, sensor_rel_el;

  if (md.get_platform_heading_angle(fid, platform_heading) &&
      md.get_platform_roll_angle(   fid, platform_roll) &&
      md.get_platform_pitch_angle(  fid, platform_pitch) &&
      md.get_sensor_rel_az_angle(   fid, sensor_rel_az) &&
      md.get_sensor_rel_el_angle(   fid, sensor_rel_el))
  {
    if (!md.get_sensor_rel_roll_angle(fid, sensor_rel_roll))
    {
      sensor_rel_roll = 0;
    }

    if (std::isnan(platform_heading) || std::isnan(platform_pitch) || std::isnan(platform_roll) ||
        std::isnan(sensor_rel_az) || std::isnan(sensor_rel_el) || std::isnan(sensor_rel_roll))
    {
      return false;
    }

    R_loc = m_priv->m_lgcs.compose_rotation(platform_heading, platform_pitch, platform_roll,
                                            sensor_rel_az, sensor_rel_el, sensor_rel_roll);

    return true;
  }

  return false;
}


bool
sfm_constraints
::get_camera_position_prior_local(frame_id_t fid,
                                  vector_3d &pos_loc) const
{
  if (m_priv->m_lgcs.origin().is_empty())
  {
    return false;
  }

  if (!m_priv->m_md)
  {
    return false;
  }

  kwiver::vital::geo_point gloc;
  double alt;
  if (!m_priv->m_md->get_sensor_location(fid, gloc) ||
      !m_priv->m_md->get_sensor_altitude(fid, alt))
  {
    return false;
  }

  auto geo_origin = m_priv->m_lgcs.origin();
  vector_2d loc = gloc.location(geo_origin.crs());
  loc -= geo_origin.location();
  alt -= m_priv->m_lgcs.origin_altitude();

  pos_loc[0] = loc.x();
  pos_loc[1] = loc.y();
  pos_loc[2] = alt;

  return true;
}

sfm_constraints::position_map
sfm_constraints
::get_camera_position_priors() const
{
  position_map local_positions;

  auto md = m_priv->m_md->metadata();

  for (auto mdv : md)
  {
    auto fid = mdv.first;

    vector_3d loc;
    if (!get_camera_position_prior_local(fid, loc))
    {
      continue;
    }
    if (local_positions.empty())
    {
      local_positions[fid] = loc;
    }
    else
    {
      auto last_loc = local_positions.crbegin()->second;
      if (loc == last_loc)
      {
        continue;
      }
      local_positions[fid] = loc;
    }
  }
  return local_positions;
}

}
}