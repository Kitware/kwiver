# This file is part of KWIVER, and is distributed under the
# OSI-approved BSD 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

import argparse
import os

START_STEP = 1
END_STEP = 5

TRACK_FILE = "results/tracks.txt"
CAMERA_DIR = "results/krtd"
LANDMARKS_FILE = "results/landmarks.ply"
DEPTH_DIR = "results/depth"
VOLUME_FILE = "results/volume.vti"
MESH_FILE = "results/mesh.vtp"
TEXTURED_MESH_FILE = "results/textured_mesh.vtp"


def run_and_return(cmd_string, dry_run=False):
    print("\nAbout to run `{}`".format(cmd_string))
    if not dry_run:
        ret_val = os.system(cmd_string)
        return ret_val
    return 0


parser = argparse.ArgumentParser(
    """
        Before running, make sure to `cd` to the build directory and `source`
        the appropriate setup_KWIVER.<extension> script for your platform

        Providing just the required arguments (kwiver binary and video) will run
        the process end-to-end on that video with default parameters. Or you
        can run a subset of the steps in the TeleSculptor pipeline
        (1) Track features
            Requires video-file
        (2) Initialize cameras
            Requires video-file, track-file
        (3) Estimate depths per view
            Requires video-file, camera-dir
        (4) Fuse depths into a mesh
            Requires camera-dir, depth-dir
        (5) Colorize mesh based on source imagery
            Requires video-file, camera-dir, mesh-file\n
        """,
    formatter_class=argparse.ArgumentDefaultsHelpFormatter)

# Required positiononals
parser.add_argument(
    "kwiver", help="Path to the kwiver binary")
parser.add_argument(
    "video",
    help="The source video to run on. Can also be a text file containing a \
          list of newline-seperated image paths")

# Which steps to run
parser.add_argument(
    "--first-step", help="The first step in the pipeline to run", type=int,
    default=START_STEP)
parser.add_argument(
    "--last-step", help="The last step in the pipeline to run", type=int,
    default=END_STEP)
parser.add_argument(
    "--dry-run",
    help="Print out the commands which would be run without executing them",
    action="store_true")

# Shared input/output arguments with logical defaults
parser.add_argument(
    "--track-file",
    help="Tracks linking feature points together.",
    default=TRACK_FILE)
parser.add_argument(
    "--camera-dir",
    help="Directory of .krtd files specifying the camera poses.",
    default=CAMERA_DIR)
parser.add_argument(
    "--landmarks-file",
    help="File of triangulated feature points.",
    default=LANDMARKS_FILE)
parser.add_argument(
    "--depth-dir",
    help="Directory of depth estimates per view.",
    default=DEPTH_DIR)
parser.add_argument(
    "--volume-file",
    help="File of integrated depth data from all views.",
    default=VOLUME_FILE)
parser.add_argument(
    "--uncolored-mesh",
    help="Mesh of the surface of the scene, without color.",
    default=MESH_FILE)
parser.add_argument(
    "--colored-mesh",
    help="Colored mesh of the surface of the scene",
    default=TEXTURED_MESH_FILE)

# Optional but shared
parser.add_argument(
    "--mask-file",
    help="Mask video with the same number of frames as the input video. Or a \
          text file of newline-seperated images.",
    default=None)

# Per-process additional arguments
parser.add_argument(
    "--optional-track-features-args",
    help="Additional CLI arguments to supply to the `track-features` tool",
    default="")
parser.add_argument(
    "--optional-init-cameras-landmarks-args",
    help="Additional CLI arguments to supply to the `init-cameras-landmarks` tool",
    default="")
parser.add_argument(
    "--optional-estimate-depth-args",
    help="Additional CLI arguments to supply to the `estimate-depth` tool",
    default="")
parser.add_argument(
    "--optional-fuse-depth-args",
    help="Additional CLI arguments to supply to the `fuse-depth` tool",
    default="")
parser.add_argument(
    "--optional-color-mesh-args",
    help="Additional CLI arguments to supply to the `color-mesh` tool",
    default="")

# Tool help
parser.add_argument(
    "--tool-help",
    help="Display the help for the KWIVER tool(s).",
    action="store_const",
    default="",
    const="--help")

args = parser.parse_args()

if args.mask_file is not None:
    # Add the flag and value
    mask_string = " -m {}".format(args.mask_file)
else:
    mask_string = ""

run_track_features = "{} track-features ".format(args.kwiver) + \
    "--video-file {} ".format(args.video) + \
    "--track-file {}{} {}".format(args.track_file,
                                  mask_string,
                                  args.optional_track_features_args) + \
    "{}".format(args.tool_help)

run_init_cameras = "{} init-cameras-landmarks ".format(args.kwiver) + \
    "--video {} --tracks {} --camera ".format(args.video,
                                              args.track_file) + \
    "{} --landmarks {}".format(args.camera_dir,
                               args.landmarks_file) + \
    "{} {}".format(args.optional_init_cameras_landmarks_args,
                   args.tool_help)

run_estimate_depth = "{} estimate-depth ".format(args.kwiver) + \
    "--video-source {} ".format(args.video) + \
    "--input-cameras-dir {} --output-depths-dir {}".format(args.camera_dir,
                                                           args.depth_dir) + \
    " --input-landmarks-file {}{}".format(args.landmarks_file,
                                          mask_string) + \
    " {} {}".format(
    args.optional_estimate_depth_args, args.tool_help)

run_fuse_depth = "{} fuse-depth ".format(args.kwiver) + \
    "--input-cameras-dir {} --input-depths-dir {} ".format(
        args.camera_dir, args.depth_dir) + \
    "--input-landmarks-file {}".format(args.landmarks_file) + \
    " --output-mesh-file {} {} {}".format(args.uncolored_mesh,
                                          args.optional_fuse_depth_args,
                                          args.tool_help)

run_color_mesh = "{} color-mesh ".format(args.kwiver) + \
    "--video-file {} ".format(args.video) + \
    "--cameras-dir {} --input-mesh {} ".format(args.camera_dir,
                                               args.uncolored_mesh) + \
    "--output-mesh {}{}".format(args.colored_mesh, mask_string) + \
    " {} {}".format(args.optional_color_mesh_args, args.tool_help)


if args.first_step <= 1 and args.last_step >= 1:
    run_and_return(run_track_features, args.dry_run)

if args.first_step <= 2 and args.last_step >= 2:
    run_and_return(run_init_cameras, args.dry_run)

if args.first_step <= 3 and args.last_step >= 3:
    run_and_return(run_estimate_depth, args.dry_run)

if args.first_step <= 4 and args.last_step >= 4:
    run_and_return(run_fuse_depth, args.dry_run)

if args.first_step <= 5 and args.last_step >= 5:
    run_and_return(run_color_mesh, args.dry_run)
