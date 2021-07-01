// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation of mesh operations
 */

#include "mesh_intersect.h"

namespace kwiver {
namespace arrows {
namespace core {

using namespace kwiver::vital;

/// Intersect the ray from point to a triangle
bool
mesh_intersect_triangle(const vital::point_3d &p,
                        const vital::vector_3d& d,
                        const vital::point_3d& a,
                        const vital::point_3d& b,
                        const vital::point_3d& c,
                        double& dist,
                        double& u, double& v)
{
  vector_3d n((b.value() - a.value()).cross(c.value() - a.value()));
  return mesh_intersect_triangle(p, d, a, b, c, n, dist, u, v);
}

/// Intersect the ray from point to a triangle
bool
mesh_intersect_triangle(const vital::point_3d& p,
                        const vital::vector_3d& d,
                        const vital::point_3d& a,
                        const vital::point_3d& b,
                        const vital::point_3d& c,
                        const vital::vector_3d& n,
                        double& dist,
                        double& u, double& v)
{
  double denom = -d.dot(n);
  if (denom <= 0) // back facing triangles
    return false;

  vector_3d ap(p.value() - a.value());
  vector_3d t(d.cross(ap));
  v = (b.value() - p.value()).dot(t);
  if (v < 0.0 || v > denom)
    return false;

  u = -(c.value() - p.value()).dot(t);
  if (u < 0.0 || u+v > denom)
    return false;

  dist = ap.dot(n);
  if (dist < 0.0)
    return false;

  u /= denom;
  v /= denom;
  dist /= denom;

  return true;
}

/// Intersect the ray from point to a triangle and check if the distance is smaller
bool
mesh_intersect_triangle_min_dist(const vital::point_3d& p,
                                 const vital::vector_3d& d,
                                 const vital::point_3d& a,
                                 const vital::point_3d& b,
                                 const vital::point_3d& c,
                                 const vital::vector_3d& n,
                                 double& dist,
                                 double& u, double& v)
{
  double denom = -d.dot(n);
  if (denom <= 0) // back facing triangles
    return false;

  vector_3d ap(p.value() - a.value());
  double new_dist = ap.dot(n)/denom;
  if (new_dist < 0.0 || new_dist > dist)
    return false;

  vector_3d t(d.cross(ap));
  v = (b.value() - p.value()).dot(t);
  if (v < 0.0 || v > denom)
    return false;

  u = -(c.value() - p.value()).dot(t);
  if (u < 0.0 || u + v > denom)
    return false;

  dist = new_dist;
  u /= denom;
  v /= denom;

  return true;
}

}
}
}
