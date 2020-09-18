# ckwg +29
# Copyright 2020 by Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#    * Neither name of Kitware, Inc. nor the names of any contributors may be used
#    to endorse or promote products derived from this software without specific
#    prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from __future__ import print_function
from __future__ import division

from vital.algo import TrainDetector
from vital.algo import DetectedObjectSetOutput

from vital.types import BoundingBox
from vital.types import CategoryHierarchy  # probably uneeded
from vital.types import DetectedObjectSet
from vital.types import DetectedObject
from vital.types import DetectedObjectType

from vital.util import VitalPIL
from vital.util.VitalPIL import get_pil_image

from PIL import Image as PILImage

from distutils.util import strtobool
from shutil import copyfile

import argparse
import numpy as np
import pickle
import os
import shutil
import signal
import sys
import subprocess
import threading
import time
import pdb

# Taken from
# https://github.com/Kitware/burn-out/blob/master/library/object_detectors/pixel_annotation_loader.cxx
COLOR_ID_MAP = {
    0: np.array([[[0, 0, 0]]], dtype=np.uint8),  # BACKGROUND_ID
    1: np.array([[[0, 255, 0]]], dtype=np.uint8),  # GREEN_PIXEL_ID
    3: np.array([[[255, 255, 0]]], dtype=np.uint8),  # YELLOW_PIXEL_ID
    4: np.array([[[0, 0, 255]]], dtype=np.uint8),  # BLUE_PIXEL_ID
    5: np.array([[[255, 0, 0]]], dtype=np.uint8),  # RED_PIXEL_ID
    6: np.array([[[255, 0, 255]]], dtype=np.uint8),  # PINK_PIXEL_ID
    # The GREEN_PIXEL_BORDER_ID enum is excluded for now
}


class BurnoutDataWriter():
    """
    Writes training data to disk in the way that the burnout training tool expects
    """

    def __init__(self, write_path, is_train=True, generate_for_empty=True):
        """
        write_path : str | path
            Where to write the data subdirectories
        is_train : bool
            Is this training data, as oposed to val
        gererate_for_empty : bool
            Generate image for set with no detections
        """
        self._generate_for_empty = generate_for_empty

        # Write the data to two different directories, train and validation
        if is_train:
            self._write_path = os.path.join(write_path, "train")
        else:
            self._write_path = os.path.join(write_path, "val")

        # TODO figure out whether this should be different per class
        # For now detections are just filled as green
        self._fill_color = COLOR_ID_MAP[1]

        # We assume this is called with write_path as an empty directory
        # This should probably make a call to a logger or something
        print("About to make directory for BurnoutWriter")
        print("About to make directory " +
              self._write_path + " for BurnOutTrainer")

        if os.path.isdir(self._write_path):
            shutil.rmtree(self._write_path)
        os.makedirs(self._write_path, exist_ok=True)

    def get_write_path(self):
        return self._write_path

    def write_set(self, gt_frame_dets, filename, index):
        """
        Takes a detected object and writes it to filename
        gt_frame_dets : kwiver.vital.detected_object_set
            The detection to write out
        filename : str
            filename of the image
        index : int
            Which frame this is
        """
        output_mask_filename = os.path.join(
            self._write_path, "gt-%d.png" % (index + 1,))
        output_image_filename = os.path.join(
            self._write_path, os.path.basename(filename))

        # check if this groundtruth has been written to before
        if os.path.isfile(output_mask_filename):
            # You might want to add more detections to a file on disk
            # TODO determine if it would be best to hold everything in memory
            # rather than going throught these obnoxious read/write cycles
            output_mask = np.asarray(PILImage.open(output_mask_filename))
        else:
            # Otherwise get a blank mask of the same shape as the imagery
            output_mask = np.zeros_like(
                PILImage.open(filename), dtype=np.uint8)

        # Get the mask for the detection and where to place it within the
        # image-wide mask
        # TODO determine which colors to use for each class

        # Errors if we try to index with empty arrays
        # The underlying array for the image is read-only
        if output_mask.ndim != 3:
            output_mask = np.repeat(np.expand_dims(output_mask, axis=2), repeats=3, axis=2)
        else:
            output_mask = output_mask.copy()

        for gt_det in gt_frame_dets:
            bbox = gt_det.bounding_box()
            if gt_det.mask is None:
                print("viame_csv_reader::read_poly needs to be turned on")
                continue
            det_mask = np.asarray(get_pil_image(gt_det.mask.image()))
            # These are the inds of the interior regions of the polygon w.r.t.
            # the detection
            y_inds, x_inds = np.nonzero(det_mask)
            # Now they are shifted to the global coordinate system
            y_inds = (y_inds + bbox.min_y()).astype(int)
            x_inds = (x_inds + bbox.min_x()).astype(int)

            # remove point outside the image
            invalid_inds = np.logical_or(np.logical_or(y_inds < 0,
                                         y_inds >= output_mask.shape[0]),
                                         np.logical_or(x_inds < 0,
                                         x_inds >= output_mask.shape[1]))
            valid_inds = np.logical_not(invalid_inds)
            x_inds = x_inds[valid_inds]
            y_inds = y_inds[valid_inds]
            output_mask[y_inds, x_inds] = self._fill_color

        # TODO this could be replaced by some sort of KWIVER utility
        shutil.copy(filename, output_image_filename)

        # Don't generate gt images for empty frames
        if len(gt_frame_dets) > 0 or self._generate_for_empty:
            output_mask_PIL = PILImage.fromarray(output_mask)
            output_mask_PIL.save(output_mask_filename)


class BurnOutTrainer(TrainDetector):
    """
    Implementation of BurnOutDetector class
    """

    def __init__(self):
        TrainDetector.__init__(self)

        self._identifier = "adaboost_pixel_classifier"
        self._temp_dir = "adaboost_training"
        # TODO determine what this is supposed to represent
        self._no_format = False

        # These need to be set in .add_data_from_disk()
        self._training_writer = None
        self._validation_writer = None
        self._categories = None

        # TODO make this more general
        self._burnin_exec = os.path.join(
            "~/dev/VIAME/build/install/bin", "remove_metadata_burnin")
        # TODO also make this one more general
        self._feature_pipeline = "~/dev/VIAME/build/install/configs/pipelines/burnout_train_classifier.conf"
        self._positive_identifiers = "1"
        self._negative_identifers = "0"
        self._max_iter_count = "200"

    def get_configuration(self):
        # Inherit from the base class
        cfg = super(TrainDetector, self).get_configuration()

        cfg.set_value("identifier", self._identifier)

        return cfg

    def set_configuration(self, cfg_in):
        cfg = self.get_configuration()
        cfg.merge_config(cfg_in)

        # Read configs from file
        self._identifier = str(cfg.get_value("identifier"))

        from vital.modules.modules import load_known_modules
        load_known_modules()

        if not self._no_format:
            self._training_writer = \
                BurnoutDataWriter(self._temp_dir, is_train=True)

        return True

    def check_configuration(self, cfg):
        if not cfg.has_value("identifier") or \
                len(cfg.get_value("identifier")) == 0:
            print("A model identifier must be specified!")
            return False
        return True

    def filter_truth(self, init_truth, categories):
        # TODO determine if this is still relevant here
        filtered_truth = DetectedObjectSet()
        use_frame = True
        for i, item in enumerate(init_truth):
            class_lbl = item.type().get_most_likely_class()
            if categories is not None and not categories.has_class_id(class_lbl):
                continue
            if categories is not None:
                class_lbl = categories.get_class_name(class_lbl)
            elif class_lbl not in self._categories:
                self._categories.append(class_lbl)

            truth_type = DetectedObjectType(class_lbl, 1.0)
            item.set_type(truth_type)

            filtered_truth.add(item)

        return filtered_truth, use_frame

    def add_data_from_disk(self, categories, train_files, train_dets,
                           test_files, test_dets):
        if len(train_files) != len(train_dets):
            print("Error: train file and groundtruth count mismatch")
            return

        if categories is not None:
            self._categories = categories.all_class_names()

        all_files = train_files + test_files
        all_dets = train_dets + test_dets
        for i in range(len(train_files) + len(test_files)):
            filename = all_files[i]
            groundtruth, use_frame = self.filter_truth(
                all_dets[i], categories)
            if use_frame:
                print("About to write trianing set")
                self._training_writer.write_set(
                    groundtruth, os.path.abspath(filename), i)

    def update_model(self):
        # TODO make these cross platform
        # TODO migrate to .format() since that should be supported on all
        # versions of python we expect
        feature_commands = ["{} -c {} \\".format(self._burnin_exec, self._feature_pipeline),
                            "    --extract-features {} \\".format(
                                self._temp_dir),
                            "    --gt-image-type png \\",
                            "    --feature-file {}/features.txt".format(
                                self._temp_dir)]
        feature_command = "\n".join(feature_commands)
        model_file = os.path.join(
            self._training_writer.get_write_path(), "trained_classifier.adb")
        train_command = "train_pixel_model {}/features.txt {} --positive-identifiers {} --negative-identifiers {} --max-iter-count {}".format(
            self._temp_dir, model_file, self._positive_identifiers,
            self._negative_identifers, self._max_iter_count)

        # TODO migrate to subprocess.run()
        print(feature_command)
        os.system(feature_command)
        print(train_command)
        os.system(train_command)
        final_model_folder = os.path.join(os.getcwd(), "category_models")
        os.makedirs(final_model_folder, exist_ok=True)
        final_model_file = os.path.join(
            final_model_folder, "trained_classifier.adb")
        try:
            shutil.copyfile(model_file, final_model_file)
        except FileNotFoundError:
            print("Final model was not produced to {}".format(model_file))

    def interupt_handler(self):
        # Also left in just in case
        self.proc.send_signal(signal.SIGINT)
        timeout = 0
        while self.proc.poll() is None:
            time.sleep(0.1)
            timeout += 0.1
            if timeout > 5:
                self.proc.kill()
                break
        sys.exit(0)


def __vital_algorithm_register__():
    from vital.algo import algorithm_factory

    # Register Algorithm
    implementation_name = "adaboost_pixel_classifier"

    if algorithm_factory.has_algorithm_impl_name(
            BurnOutTrainer.static_type_name(), implementation_name):
        return

    algorithm_factory.add_algorithm(implementation_name,
                                    "AdaBoost per-pixel classifier from Burnout training routine", BurnOutTrainer)

    algorithm_factory.mark_algorithm_as_loaded(implementation_name)
