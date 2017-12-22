"""
ckwg +31
Copyright 2015-2017 by Kitware, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Kitware, Inc. nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific
   prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

==============================================================================

vital.types module

"""

# Common VITAL Components for easy access
from .bounding_box import BoundingBox
from .camera_intrinsics import CameraIntrinsics
from .color import RGBColor
from .covariance import Covariance
from .descriptor import Descriptor
from .descriptor_set import DescriptorSet
from .detected_object_type import DetectedObjectType
from .detected_object import DetectedObject
from .detected_object_set import DetectedObjectSet
from .eigen import EigenArray
from .image import Image
from .image_container import ImageContainer
from .rotation import Rotation

# Requires EigenArray
from .homography import Homography

# Requires EigenArray and RGBColor
from .feature import Feature

# Requires EigenArray and Rotation
from .similarity import Similarity

# Requires Covariance
from .landmark import Landmark
from .landmark_map import LandmarkMap

# Requires Descriptor, DetectedObject, Feature
from .track import TrackState, Track
from .track_set import TrackSet
from .feature_track_set import FeatureTrackState
from .object_track_set import ObjectTrackState
from .object_track_set import ObjectTrackSet

# Requires CameraIntrinsics, Covariance, EigenArray, Rotation
from .camera import Camera
from .camera_map import CameraMap
