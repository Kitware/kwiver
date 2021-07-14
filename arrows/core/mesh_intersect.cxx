// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation of mesh operations
 */

#include "mesh_intersect.h"
#include <limits>
#include <vital/logger/logger.h>

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
  {
    return false;
  }

  u = -(c.value() - p.value()).dot(t);
  if (u < 0.0 || u+v > denom)
  {
    return false;
  }

  dist = ap.dot(n);
  if (dist < 0.0)
  {
    return false;
  }

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
  {
    return false;
  }

  vector_3d ap(p.value() - a.value());
  double new_dist = ap.dot(n)/denom;
  if (new_dist < 0.0 || new_dist > dist)
  {
    return false;
  }

  vector_3d t(d.cross(ap));
  v = (b.value() - p.value()).dot(t);
  if (v < 0.0 || v > denom)
  {
    return false;
  }

  u = -(c.value() - p.value()).dot(t);
  if (u < 0.0 || u + v > denom)
  {
    return false;
  }

  dist = new_dist;
  u /= denom;
  v /= denom;

  return true;
}

/// Find the closest point on the triangle to a reference point
unsigned char
mesh_triangle_closest_point(const vital::point_3d& p,
                            const vital::point_3d& a,
                            const vital::point_3d& b,
                            const vital::point_3d& c,
                            const vital::vector_3d& n,
                            double& dist,
                            double& u, double& v)
{
  double denom = 1.0/n.squaredNorm();

  vector_3d ap(p.value() - a.value());
  vector_3d bp(p.value() - b.value());
  vector_3d cp(p.value() - c.value());

  vector_3d t(n.cross(ap));
  v = bp.dot(t) * denom;
  u = -cp.dot(t) * denom;

  vector_3d ab(b.value() - a.value());
  vector_3d bc(c.value() - b.value());
  vector_3d ca(a.value() - c.value());

  double eps = std::numeric_limits<double>::epsilon();

  unsigned char state = 0;
  double uv;
  if (u <= eps)
  {
    double p_v = v - u * ab.dot(ca)/ca.squaredNorm();
    if (p_v <= eps)
    {
      state = 1;
    }
    else if (p_v >= 1.0)
    {
      state = 4;
    }
    else
    {
      u = 0.0; v = p_v;
      dist = ((1 - v)*ap + v*cp).norm();
      return 5;
    }
  }
  if (v <= eps)
  {
    double p_u = u - v * ca.dot(ab)/ab.squaredNorm();
    if (p_u <= eps)
    {
      state = 1;
    }
    else if (p_u >= 1.0)
    {
      state = 2;
    }
    else
    {
      u = p_u; v = 0.0;
      dist = ((1 - u)*ap + u*bp).norm();
      return 3;
    }
  }
  if ((uv = 1.0 - u - v) <= eps)
  {
    double s = -ca.dot(bc)/bc.squaredNorm();
    double p_u = u + uv * s;
    double p_v = v + uv * (1.0-s);
    if (p_v <= eps)
    {
      state = 2;
    }
    else if (p_u <= eps)
    {
      state = 4;
    }
    else
    {
      u=p_u; v=p_v;
      dist = (u*bp + v*cp).norm();
      return 6;
    }
  }

  switch (state)
  {
    case 1:
      u=0.0; v=0.0;
      dist = ap.norm();
      return 1;
    case 2:
      u=1.0; v=0.0;
      dist = bp.norm();
      return 2;
    case 4:
      u=0.0; v=1.0;
      dist = cp.norm();
      return 4;
    default:
      dist = std::abs(ap.dot(n) * std::sqrt(denom));
      return 7;
  }

  return 0;
}

/// Find the closest point on the triangle to a reference point
unsigned char
mesh_triangle_closest_point(const vital::point_3d& p,
                            const vital::point_3d& a,
                            const vital::point_3d& b,
                            const vital::point_3d& c,
                            double& dist,
                            double& u, double& v)
{
  vector_3d n((b.value() - a.value()).cross(c.value() - a.value()));
  return mesh_triangle_closest_point(p, a, b, c, n, dist, u, v);
}

/// Find the closest point on the triangle to a reference point
vital::point_3d
mesh_triangle_closest_point(const vital::point_3d& p,
                            const vital::point_3d& a,
                            const vital::point_3d& b,
                            const vital::point_3d& c,
                            double& dist)
{
  double u,v;
  mesh_triangle_closest_point(p, a, b, c, dist, u, v);
  double t = 1 - u - v;
  return point_3d(t*a[0] + u*b[0] + v*c[0],
                  t*a[1] + u*b[1] + v*c[1],
                  t*a[2] + u*b[2] + v*c[2]);
}

/// Find the closest point on a triangulated mesh to a reference point
int
mesh_closest_point(const vital::point_3d& p,
                   const vital::mesh& mesh,
                   vital::point_3d& cp,
                   double& u, double& v)
{
  // check for a triangular mesh
  if (mesh.faces().regularity() != 3)
  {
    LOG_ERROR(vital::get_logger("arrows.core.mesh_closest_point"),
      "Closest point calculation requires triangular mesh.");
    return -1;
  }

  const mesh_vertex_array<3>& verts = mesh.vertices<3>();
  const mesh_regular_face_array<3>& faces =
    static_cast<const mesh_regular_face_array<3>&>(mesh.faces());

  int isect = -1;
  double u1, v1;
  double shortest_dist = std::numeric_limits<double>::infinity();
  for (unsigned int i=0; i<faces.size(); ++i)
  {
    const mesh_regular_face<3>& f = faces[i];
    vital::point_3d a(verts[f[0]]);
    vital::point_3d b(verts[f[1]]);
    vital::point_3d c(verts[f[2]]);
    double dist = shortest_dist;
    if (mesh_triangle_closest_point(p, a, b, c, dist, u1, v1) &&
        dist < shortest_dist)
    {
      u = u1;
      v = v1;
      isect = i;
      shortest_dist = dist;
    }
  }

  // Get the closest point in physical space from barycentric coordinates
  double t = 1 - u - v;
  const mesh_regular_face<3>& f = faces[isect];
  vital::point_3d a(verts[f[0]]);
  vital::point_3d b(verts[f[1]]);
  vital::point_3d c(verts[f[2]]);
  cp.set_value(vector_3d(t*a[0] + u*b[0] + v*c[0],
                         t*a[1] + u*b[1] + v*c[1],
                         t*a[2] + u*b[2] + v*c[2]));
  return isect;
}

/// Intersect a ray from point to a triangulated mesh
int
mesh_intersect(const vital::point_3d& p,
               const vital::vector_3d& d,
               const vital::mesh& mesh,
               double& dist,
               double& u, double& v)
{
  // check for a triangular mesh
  if (mesh.faces().regularity() != 3)
  {
    LOG_ERROR(vital::get_logger("arrows.core.mesh_closest_point"),
      "Closest point calculation requires triangular mesh.");
    return -1;
  }

  // Calculate normals if needed
  if (!mesh.faces().has_normals())
  {
    LOG_ERROR(vital::get_logger("arrows.core.mesh_closest_point"),
      "Closest point calculation requires faces normals.");
    return -1;
  }

  const mesh_vertex_array<3>& verts = mesh.vertices<3>();
  const mesh_regular_face_array<3>& faces =
    static_cast<const mesh_regular_face_array<3>&>(mesh.faces());

  // Check that normal magnitude corresponds to face area
  if ( ((verts[faces[0][1]] - verts[faces[0][0]])
        .cross(verts[faces[0][2]] - verts[faces[0][0]])
               - 0.5*faces.normal(0)).norm() < 1e-14 )
  {
    LOG_ERROR(vital::get_logger("arrows.core.mesh_closest_point"),
      "Closest point calculation requires faces normal lengths be set to face area.");
    return -1;
  }

  int isect = -1;
  dist = std::numeric_limits<double>::infinity();
  for (unsigned int i=0; i<faces.size(); ++i)
  {
    const mesh_regular_face<3>& f = faces[i];
    vital::point_3d a(verts[f[0]]);
    vital::point_3d b(verts[f[1]]);
    vital::point_3d c(verts[f[2]]);
    if (mesh_intersect_triangle_min_dist(p, d, a, b, c, faces.normal(i), dist, u, v))
    {
      isect = i;
    }
  }
  return isect;
}

}
}
}
