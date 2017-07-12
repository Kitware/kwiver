/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
 * \brief Abstract polygon interface
 */

#ifndef VITAL_TYPES_POLYGON_H
#define VITAL_TYPES_POLYGON_H

#include <vital/vital_config.h>
#include <vital/vital_export.h>
#include <vital/types/vector.h>

#include <memory>
#include <vector>

namespace kwiver {
namespace vital {

// ----------------------------------------------------------------
/**
 * @brief Abstract base polygon class.
 *
 * This class represents a polygon with a limited number of
 * attributes. The concrete implementation of the polygon is delegated
 * to a concrete derived class. There may be more than one possible
 * implementation. These implementations should provide a way to
 * access the implementation specific methods because they usually
 * provide additional attributes and operations. These derived classes
 * should supply conversion methods to and from the basic (core)
 * implementation.
 *
 * This class behaviour is considered the specification for all
 * derived classes.
 */

class VITAL_EXPORT polygon
{
public:
  typedef kwiver::vital::vector_2d point_t;

  polygon();
  polygon( const std::vector< kwiver::vital::polygon::point_t >& dat);
  ~polygon();

  /**
   * @brief Add point to end of polygon.
   *
   * This method adds a point to the end of the list of points that
   * define the polygon.
   *
   * @param x The X coordinate
   * @param y The Y coordinate
   */
  void push_back( double x, double y );

  /**
   * @brief Add point to end of polygon.
   *
   * This method adds a point to the end of the list of points that
   * define the polygon.
   *
   * @param pt The point to add to polygon.
   */
  void push_back( const point_t& pt );

  /**
   * @brief Get number of vertices in polygon.
   *
   * This method returns the number of vertices or points in this
   * polygon.
   *
   *  @return Number of vertices/points.
   */
  size_t num_vertices() const;

  /**
   * @brief Get list of vertices.
   *
   * This method returns the list of points that make up the polygon.
   *
   * @return List of vertices.
   */
  std::vector< kwiver::vital::polygon::point_t > get_vertices() const;

  /**
   * @brief Does this polygon contain the point.
   *
   * This method determines if the specified point is within the
   * polygon or not. Vertex points and points in the boundary are
   * considered within the polygon.
   *
   * @param x The X coordinate
   * @param y The Y coordinate
   *
   * @return \b true if the point is within the polygon.
   */
  bool contains( double x, double y );

  /**
   * @brief Does this polygon contain the point.
   *
   * This method determines if the specified point is within the
   * polygon or not. Vertex points and points in the boundary are
   * considered within the polygon.
   *
   * @param pt The point to test.
   *
   * @return \b true if the point is within the polygon.
   */
  bool contains( const point_t& pt );

  /**
   * @brief Get Nth vertex in polygon.
   *
   * This method returns the requested vertex point. If the index is
   * beyond the bounds of this polygon, an exception is thrown.
   *
   * @param idx The vertex index, from 0 to num_vertices()-1
   *
   * @return The point at the specified vertex.
   *
   * @throws std::out_of_range exception
   */
  point_t at( size_t idx ) const;

private:
  std::vector< kwiver::vital::polygon::point_t > m_polygon;
}; // end class polygon

// Types for managing polygons
typedef std::shared_ptr< polygon > polygon_sptr;
typedef std::vector< polygon_sptr >  polygon_sptr_list;

} } // end namespace

#endif // VITAL_TYPES_POLYGON_H
