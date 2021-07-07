// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Operations to modify meshes
 */

#ifndef KWIVER_ARROWS_CORE_MESH_INTERSECT_H
#define KWIVER_ARROWS_CORE_MESH_INTERSECT_H

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/types/mesh.h>
#include <vital/types/point.h>

namespace kwiver {
namespace arrows {
namespace core {

/// Intersect the ray from point to a triangle
/**
 * Intersect the ray from point p with direction d and the triangle
 * defined by a,b,c.
 *
 * \param [in]   p     point that is the start of the ray
 * \param [in]   d     direction of the ray
 * \param [in]   a     corner point of triangle
 * \param [in]   b     corner point of triangle
 * \param [in]   c     corner point of triangle
 * \param [out]  dist  the distance to the triangle
 * \param [out]  u     barycentric coordinate of the intersection
 * \param [out]  v     barycentric coordinate of the intersection
 * \returns      true  if intersection occurs
 * Barycentric coordinates are u and v such that (1-u-v)*a + u*b + v*c = p+dist*d
 */
KWIVER_ALGO_CORE_EXPORT
bool
mesh_intersect_triangle(const vital::point_3d& p,
                        const vital::vector_3d& d,
                        const vital::point_3d& a,
                        const vital::point_3d& b,
                        const vital::point_3d& c,
                        double& dist,
                        double& u, double& v);


/// Intersect the ray from point to a triangle
/**
 * Intersect the ray from point p with direction d and the triangle defined
 * by a,b,c. The un-normalized normal vector (b-a)x(c-a) is precomputed and
 * also passed in.
 *
 * \param [in]   p     point that is the start of the ray
 * \param [in]   d     direction of the ray
 * \param [in]   a     corner point of triangle
 * \param [in]   b     corner point of triangle
 * \param [in]   c     corner point of triangle
 * \param [in]   n     pre-computed normal for triangle
 * \param [out]  dist  the distance to the triangle
 * \param [out]  u     barycentric coordinate of the intersection
 * \param [out]  v     barycentric coordinate of the intersection
 * \returns      true  if intersection occurs
 * Barycentric coordinates are u and v such that (1-u-v)*a + u*b + v*c = p+dist*d
 */
KWIVER_ALGO_CORE_EXPORT
bool
mesh_intersect_triangle(const vital::point_3d& p,
                        const vital::vector_3d& d,
                        const vital::point_3d& a,
                        const vital::point_3d& b,
                        const vital::point_3d& c,
                        const vital::vector_3d& n,
                        double& dist,
                        double& u, double& v);

/// Intersect the ray from point to a triangle and check if the distance is smaller
/**
 * Intersect the ray from point p with direction d and the triangle defined
 * by a,b,c. The un-normalized normal vector (b-a)x(c-a) is precomputed and
 * also passed in.
 *
 * \param [in]   p     point that is the start of the ray
 * \param [in]   d     direction of the ray
 * \param [in]   a     corner point of triangle
 * \param [in]   b     corner point of triangle
 * \param [in]   c     corner point of triangle
 * \param [in]   n     pre-computed normal for triangle
 * \param [out]  dist  the distance to the triangle
 * \param [out]  u     barycentric coordinate of the intersection
 * \param [out]  v     barycentric coordinate of the intersection
 * \returns      true  if intersection occurs and the new dist is less than the old distance (but > 0)
 * Barycentric coordinates are u and v such that (1-u-v)*a + u*b + v*c = p+dist*d
 */
KWIVER_ALGO_CORE_EXPORT
bool
mesh_intersect_triangle_min_dist(const vital::point_3d& p,
                                 const vital::vector_3d& d,
                                 const vital::point_3d& a,
                                 const vital::point_3d& b,
                                 const vital::point_3d& c,
                                 const vital::vector_3d& n,
                                 double& dist,
                                 double& u, double& v);

/// Find the closest point on the triangle to a reference point
/**
 * Find the closest point on the triangle a,b,c to point p. The un-normalized
 * normal vector (b-a)x(c-a) is precomputed and also passed in
 *
 * \param [in]   p     reference point to get closest distance to
 * \param [in]   a     corner point of triangle
 * \param [in]   b     corner point of triangle
 * \param [in]   c     corner point of triangle
 * \param [in]   n     pre-computed normal for triangle
 * \param [out]  dist  the distance to the triangle
 * \param [out]  u     barycentric coordinate of the intersection
 * \param [out]  v     barycentric coordinate of the intersection
 * \returns      a code indicating that the closest point:
 *               - 0 does not exist (should not occur)
 *               - 1 is \a a
 *               - 2 is \a b
 *               - 3 is on the edge from \a a to \a b
 *               - 4 is \a c
 *               - 5 is on the edge from \a a to \a c
 *               - 6 is on the edge from \a b to \a c
 *               - 7 is on the face of the triangle
 */
KWIVER_ALGO_CORE_EXPORT
unsigned char
mesh_triangle_closest_point(const vital::point_3d& p,
                            const vital::point_3d& a,
                            const vital::point_3d& b,
                            const vital::point_3d& c,
                            const vital::vector_3d& n,
                            double& dist,
                            double& u, double& v);

/// Find the closest point on the triangle to a reference point
/**
 * Find the closest point on the triangle a,b,c to point p.
 *
 * \param [in]   p     reference point to get closest distance to
 * \param [in]   a     corner point of triangle
 * \param [in]   b     corner point of triangle
 * \param [in]   c     corner point of triangle
 * \param [out]  dist  the distance to the triangle
 * \param [out]  u     barycentric coordinate of the intersection
 * \param [out]  v     barycentric coordinate of the intersection
 * \returns      a code indicating that the closest point:
 *               - 0 does not exist (should not occur)
 *               - 1 is \a a
 *               - 2 is \a b
 *               - 3 is on the edge from \a a to \a b
 *               - 4 is \a c
 *               - 5 is on the edge from \a a to \a c
 *               - 6 is on the edge from \a b to \a c
 *               - 7 is on the face of the triangle
 */
KWIVER_ALGO_CORE_EXPORT
unsigned char
mesh_triangle_closest_point(const vital::point_3d& p,
                            const vital::point_3d& a,
                            const vital::point_3d& b,
                            const vital::point_3d& c,
                            double& dist,
                            double& u, double& v);

/// Find the closest point on the triangle to a reference point
/**
 * Find the closest point on the triangle a,b,c to point p
 *
 * \param [in]   p     reference point to get closest distance to
 * \param [in]   a     corner point of triangle
 * \param [in]   b     corner point of triangle
 * \param [in]   c     corner point of triangle
 * \param [out]  dist  the closest distance to the triangle
 * \returns      the point on the triangle closest to the reference point
 */
KWIVER_ALGO_CORE_EXPORT
vital::point_3d
mesh_triangle_closest_point(const vital::point_3d& p,
                            const vital::point_3d& a,
                            const vital::point_3d& b,
                            const vital::point_3d& c,
                            double& dist);

}
}
}
#endif // KWIVER_ARROWS_CORE_MESH_INTERSECT_H
