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
  state->detection = kwiver::vital::detected_object_sptr(new kwiver::vital::detected_object(bbox, confidence, type));

  // Image Point
  // This point is the coordinates for the object in the raw image coordinate system.
  // This point may be drawn from the center of the bounding box, bottom center, or wherever for that matter. 
  // It is important to keep this around because the camera/world model will need to know the raw-image coordinates
  // in order to unproject the image location into the world coordinate system, and we may at some point lose access to the detection.
  // NOTE the meta data describing the coordinate system used is not part of this class, that should be kept and enforced by the user

  // You can test to see if this object track has one
  if(state->get_image_point() == nullptr) // Nothing there, let's make one
    state->get_image_point() = std::make_shared <kwiver::vital::point_2d>();
  // What does it look like by default
  std::cout << *c_state->get_image_point() << std::endl;
  // Modify the image point
  state->get_image_point()->value() << 1., 1.;
  // Create and set a covariance matrix (These need to be symentrical)
  Eigen::Matrix2d cov2;
  cov2 << 1, 2, 2, 1;
  state->get_image_point()->covariance() = kwiver::vital::covariance_2d_sptr(new kwiver::vital::covariance_2d(cov2));
  // View what it looks like now
  std::cout << *c_state->get_image_point() << std::endl;

  // Tracking Points
  // This point is the location of the object within some tracking coordinate system.
  // This could be a 2-D or 3-D coordinate system, Cartesian or otherwise.
  // It is the coordinate system that the kinematics makes the most sense in,
  // and the kinematics filter will filter based on these coordinates.
  // In some cases, this may be a stabilized-image coordinate system.
  // NOTE the meta data describing the coordinate system used is not part of this class, that should be kept and enforced by the user
  // NOTE If used by an application, generally only one point will be used depending on the demension of the application

  // You can test to see if this object track has one
  if (state->get_track2d_point() == nullptr && state->get_track3d_point() == nullptr) // Nothing there, let's make a 3D one
    state->get_track3d_point() = std::make_shared<kwiver::vital::point_3d>();
  // What does it look like by default
  std::cout << *c_state->get_track3d_point() << std::endl;
  // Modify the image point
  state->get_track3d_point()->value() << 1., 2., 3.;
  // Create and set a covariance matrix (These need to be symentrical)
  Eigen::Matrix3d cov3;
  cov3 << 1, 2, 3, 2, 1, 2, 3, 2, 1;
  state->get_track3d_point()->covariance() = std::make_shared<kwiver::vital::covariance_3d>(cov3);
  // View what it looks like now
  std::cout << *c_state->get_track3d_point() << std::endl;

  // A world coordinate can be associated with the track as well
  if (state->get_geo_point() == nullptr) // Again, by default, there is no geo_point
    state->get_geo_point() = std::make_shared <kwiver::vital::geo_point>();
  // Set the location
  state->get_geo_point()->set_location(kwiver::vital::vector_2d(33, 44), kwiver::vital::SRID::lat_lon_WGS84);
  // You can convert to other coordinate systems
  auto nad83 = state->get_geo_point()->location(kwiver::vital::SRID::lat_lon_NAD83);
  //state->get_geo_point().covariance().matrix << 1, 2, 3,
  //                                              4, 5, 6,
  //                                              7, 8, 9;

  // Add the state to the track
  track->insert(kwiver::vital::track_state_sptr(state));

  // Add our track to our track set
  tracks->insert(track);
}
