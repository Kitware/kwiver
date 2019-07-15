/*ckwg +29
* Copyright 2017-2018 by Kitware, Inc.
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

#include "vital/types/object_track_set.h"
#include <vital/types/geo_point.h>
#include <vital/types/geodesy.h>

#include "vital/plugin_loader/plugin_manager.h"

#include <kwiversys/SystemTools.hxx>

void how_to_part_03_tracking()
{
  // Initialize KWIVER and load up all plugins
  kwiver::vital::plugin_manager::instance().load_all_plugins();

  // Many vision algorithms are used to track objects.
  // In this example we will explore the object tracking data types.

  // All tracks for a given scene are stored in a set
  kwiver::vital::object_track_set_sptr tracks = 
    kwiver::vital::object_track_set_sptr(new kwiver::vital::object_track_set());

  // Let's create a track 
  auto track = kwiver::vital::track::create();
  
  // Create the state of the track for frame 0, time 0
  kwiver::vital::object_track_state* state = new kwiver::vital::object_track_state(0, 0);
  const kwiver::vital::object_track_state* c_state = state;

  // Create an optional detection object (See how to part 2 for how to do this)
  kwiver::vital::bounding_box_d bbox(0, 0, 1, 1);
  double confidence = 1.0;
  kwiver::vital::detected_object_type_sptr type(new kwiver::vital::detected_object_type());
  type->set_score("vehicle", 0.03);
  type->set_score("person", 0.52);
  type->set_score("object", 0.23);
  state->detection() = kwiver::vital::detected_object_sptr(new kwiver::vital::detected_object(bbox, confidence, type));

  // Image Point
  // This point is the coordinates for the object in the raw image coordinate system.
  // This point may be drawn from the center of the bounding box, bottom center, or wherever for that matter. 
  // It is important to keep this around because the camera/world model will need to know the raw-image coordinates
  // in order to unproject the image location into the world coordinate system, and we may at some point lose access to the detection.
  // NOTE the meta data describing the coordinate system used is not part of this class, that should be kept and enforced by the user

  // You can test to see if this object track has one
  if(state->image_point() == nullptr) // Nothing there, let's make one
    state->image_point() = std::make_shared <kwiver::vital::point_2d>();
  // What does it look like by default
  std::cout << *c_state->image_point() << std::endl;
  // Modify the image point
  state->image_point()->value() << 1., 1.;
  // Create and set a covariance matrix (These need to be symentrical)
  Eigen::Matrix2d cov2;
  cov2 << 1, 2, 2, 1;
  state->image_point()->covariance() = kwiver::vital::covariance_2d_sptr(new kwiver::vital::covariance_2d(cov2));
  // View what it looks like now
  std::cout << *c_state->image_point() << std::endl;

  // Tracking Points

  // The 2D tracking point is the location of the object within some 2D tracking coordinate system.
  // It is the coordinate system that the kinematics makes the most sense in,
  // and the kinematics filter will filter based on these coordinates.
  // In some cases, this may be a stabilized-image coordinate system.
  // NOTE the meta data describing the coordinate system used is not part of this class, that should be kept and enforced by the user


  if (state->track2d_point() == nullptr) // Nothing there, let's make a one
    state->track2d_point() = std::make_shared<kwiver::vital::point_2d>();
  // What does it look like by default
  std::cout << *c_state->track2d_point() << std::endl;
  // Modify the track2d point
  state->track2d_point()->value() << 1., 2.;
  // Create and set a covariance matrix (These need to be symentrical)
  state->track2d_point()->covariance() = std::make_shared<kwiver::vital::covariance_2d>(cov2);
  // View what it looks like now
  std::cout << *c_state->track2d_point() << std::endl;

  // The Track Offset is a 3D cartesian coordinate of the tracked object.
  // By providing an origin world coordinate as the center of the cartesian system,
  // The offset can be converted into a world coordinate.

  // You can test to see if this object track has one
  if (state->track_offset() == nullptr) // Nothing there, let's make a one
    state->track_offset() = std::make_shared<kwiver::vital::geo_offset>();
  // What does it look like by default
  std::cout << *c_state->track_offset() << std::endl;
  // Modify the offset point
  state->track_offset()->value() << 1234, 5678, 90;
  // Create and set a covariance matrix (These need to be symentrical)
  Eigen::Matrix3d cov3;
  cov3 << 1, 2, 3, 2, 1, 2, 3, 2, 1;
  state->track_offset()->covariance() = std::make_shared<kwiver::vital::covariance_3d>(cov3);
  // View what it looks like now
  std::cout << *c_state->track_offset() << std::endl;

  // An origin defined as a geo_point can be used to calcuate a world coordinate from the track_offset
  kwiver::vital::geo_point_sptr origin = std::make_shared < kwiver::vital::geo_point>();
  // Set the origin location
  origin->set_location(kwiver::vital::vector_2d(-73.759291, 42.849631), kwiver::vital::SRID::lat_lon_WGS84);
  // Set the origin of our offset
  state->track_offset()->origin() = origin;
  // What is the geo location of our offset?
  auto loc = state->track_offset()->get_lon_lat_alt();
  std::cout << loc << std::endl;

  // You can also set the offset by providing two geo points
  kwiver::vital::geo_point_sptr location = std::make_shared < kwiver::vital::geo_point>();
  location->set_location(kwiver::vital::vector_2d(-73.74418, 42.90074), kwiver::vital::SRID::lat_lon_WGS84);
  state->track_offset()->set_from_geo_points(origin, location);
  // View what it looks like now
  std::cout << *c_state->track_offset() << std::endl;

  // Add the state to the track
  track->insert(kwiver::vital::track_state_sptr(state));
  // Add our track to our track set
  tracks->insert(track);
}
