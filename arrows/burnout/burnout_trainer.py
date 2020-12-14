# This file is part of KWIVER, and is distributed under the
# OSI-approved BSD 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

from __future__ import print_function
from __future__ import division

from vital.algo import TrainDetector

from vital.types import BoundingBox
from vital.types import CategoryHierarchy  # probably uneeded
from vital.types import DetectedObjectSet
from vital.types import DetectedObjectType

from vital.util.VitalPIL import get_pil_image

from PIL import Image as PILImage

from distutils.util import strtobool
from shutil import copyfile

import argparse
import numpy as np
import os
import shutil
import signal
import sys
import subprocess
import time

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

        # Errors if we try to index with empty arrays
        # The underlying array for the image is read-only
        if output_mask.ndim != 3:
            output_mask = np.repeat(np.expand_dims(
                output_mask, axis=2), repeats=3, axis=2)
        else:
            output_mask = output_mask.copy()

        for gt_det in gt_frame_dets:
            bbox = gt_det.bounding_box()
            if gt_det.mask is None:
                raise ValueError(
                    "viame_csv_reader::read_poly needs to be turned on")

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

        self.proc = None
        self._training_writer = None

        self._identifier = "adaboost_pixel_classifier"
        self._temp_dir = "adaboost_training"

        self._burnin_exec = "remove_metadata_burnin"
        self._feature_pipeline = "burnout_train_classifier.conf"
        self._positive_identifiers = "1"
        self._negative_identifiers = "0"
        self._max_iter_count = "200"

        # This needs to be set in .add_data_from_disk()
        self._categories = None

    def get_configuration(self):
        # Inherit from the base class
        cfg = super(TrainDetector, self).get_configuration()

        cfg.set_value("identifier", self._identifier)
        cfg.set_value("remove_metadata_burnin_exec", self._burnin_exec)
        cfg.set_value("feature_pipeline", self._feature_pipeline)
        cfg.set_value("temp_dir", self._temp_dir)
        cfg.set_value("positive_identifiers", self._positive_identifiers)
        cfg.set_value("negative_identifiers", self._negative_identifiers)
        cfg.set_value("max_iter_count", self._max_iter_count)

        return cfg

    def set_configuration(self, cfg_in):
        cfg = self.get_configuration()
        cfg.merge_config(cfg_in)

        self._identifier = str(cfg.get_value("identifier"))
        self._burnin_exec = str(cfg.get_value("remove_metadata_burnin_exec"))
        self._feature_pipeline = str(cfg.get_value("feature_pipeline"))
        self._temp_dir = str(cfg.get_value("temp_dir"))
        self._positive_identifiers = str(cfg.get_value("positive_identifiers"))
        self._negative_identifiers = str(cfg.get_value("negative_identifiers"))
        self._max_iter_count = str(cfg.get_value("max_iter_count"))

        from vital.modules.modules import load_known_modules
        load_known_modules()

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
            raise ValueError(
                "Error: train file and groundtruth count mismatch")

        if categories is not None:
            self._categories = categories.all_class_names()

        all_files = train_files + test_files
        all_dets = train_dets + test_dets
        for i, filename in enumerate(all_files):
            groundtruth, use_frame = self.filter_truth(
                all_dets[i], categories)
            if use_frame:
                self._training_writer.write_set(
                    groundtruth, os.path.abspath(filename), i)

    def update_model(self):
        """Generate a model on disk based on the intermediate images"""
        # Generate the features from images
        features_file = os.path.join(self._temp_dir, "features.txt")
        feature_command = [self._burnin_exec,
                           "-c",
                           self._feature_pipeline,
                           "--extract-features",
                           self._temp_dir,
                           "--gt-image-type",
                           "png",
                           "--feature-file",
                           features_file]

        self.proc = subprocess.run(feature_command)

        model_folder = os.path.join(self._temp_dir, "trained_model")
        os.makedirs(model_folder, exist_ok=True)
        model_file = os.path.join(model_folder, "trained_classifier.adb")

        train_command = ["train_pixel_model",
                         features_file,
                         model_file,
                         "--positive-identifiers",
                         self._positive_identifiers,
                         "--negative-identifiers",
                         self._negative_identifiers,
                         "--max-iter-count",
                         self._max_iter_count]

        self.proc = subprocess.run(train_command)

    def interupt_handler(self):
        # Give the subprocess five seconds to exit cleanly
        if self.proc is not None:
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
