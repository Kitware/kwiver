// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "color_mesh.h"

#include <arrows/core/colorize.h>
#include <arrows/vtk/mesh_coloration.h>
#include <vital/types/camera_map.h>
#include <vital/algo/video_input.h>
#include <vital/algo/pointcloud_io.h>
#include <vital/applets/applet_config.h>
#include <vital/applets/config_validation.h>
#include <vital/io/camera_io.h>
#include <vital/io/metadata_io.h>
#include <vital/plugin_loader/plugin_manager.h>
#include <vital/config/config_block_io.h>
#include <vital/config/config_block.h>
#include <vital/config/config_parser.h>
#include <vital/util/get_paths.h>
#include <vital/util/transform_image.h>

#include <kwiversys/SystemTools.hxx>
#include <kwiversys/CommandLineArguments.hxx>

#include <vtkLookupTable.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkPointData.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace kwiver {
namespace arrows {
namespace vtk {

namespace kv = ::kwiver::vital;
namespace kva = ::kwiver::vital::algo;

namespace {

typedef kwiversys::SystemTools ST;
typedef kwiversys::CommandLineArguments argT;

kv::logger_handle_t main_logger( kv::get_logger( "color_mesh_applet" ) );

// ------------------------------------------------------------------
bool check_config(kv::config_block_sptr config)
{
  using namespace kwiver::tools;
  bool config_valid = true;

#define KWIVER_CONFIG_FAIL(msg) \
  LOG_ERROR(main_logger, "Config Check Fail: " << msg); \
  config_valid = false

  config_valid =
    validate_required_input_file("video_source", *config, main_logger)
    && config_valid;

  std::string extension = "";
  if (config->has_value("output_mesh"))
  {
    std::string output_mesh =
      config->get_value<std::string>("output_mesh");
    extension = ST::GetFilenameLastExtension( output_mesh );
  }
  if (extension == ".las")
  {
    config_valid =
      validate_required_input_file("input_geo_origin_filename", *config,
                                    main_logger) && config_valid;
  }
  else
  {
    config_valid =
      validate_optional_input_file("input_geo_origin_filename", *config,
                                    main_logger) && config_valid;
  }

  if (!kv::algo::video_input::check_nested_algo_configuration("video_reader", config))
  {
    KWIVER_CONFIG_FAIL("video_reader configuration check failed");
  }
  std::string active_attribute = config->get_value<std::string>("active_attribute");
  bool all_frames = config->get_value<bool>("all_frames");
  if (all_frames && active_attribute.size() > 0)
  {
    KWIVER_CONFIG_FAIL("active_attribute only applies for composite color");
  }

#undef KWIVER_CONFIG_FAIL

  return config_valid;
}

} // end namespace

class color_mesh::priv : public kwiver::arrows::vtk::mesh_coloration
{
public:
  priv() = default;
  kva::video_input_sptr video_reader_ = nullptr;
  kva::video_input_sptr mask_reader_ = nullptr;
  kv::config_block_sptr config_ = nullptr;
  std::string input_mesh_;
  std::string input_geo_origin_file_;
  std::string video_source_;
  std::string cameras_dir_;
  std::string mask_file_;
  std::string output_mesh_;
  std::string active_attribute_ = "mean";

  enum commandline_mode { SUCCESS, HELP, WRITE, FAIL };

  commandline_mode process_command_line(cxxopts::ParseResult& cmd_args)
  {
    using kwiver::tools::load_default_video_input_config;
    std::string opt_config;
    std::string opt_out_config;

    if (cmd_args["help"].as<bool>())
    {
      return HELP;
    }
    if (cmd_args.count("config"))
    {
      opt_config = cmd_args["config"].as<std::string>();
    }
    if (cmd_args.count("output-config") > 0)
    {
      opt_out_config = cmd_args["output-config"].as<std::string>();
    }

    // Set up top level configuration w/ defaults where applicable.
    config_ = default_config();

    // If -c/--config given, read in confg file, merge in with default just
    // generated
    if (!opt_config.empty())
    {
      config_->merge_config(kv::read_config_file(opt_config));
    }

    if (cmd_args.count("input-mesh"))
    {
      input_mesh_ = cmd_args["input-mesh"].as<std::string>();
      config_->set_value("input_mesh", input_mesh_);
    }
    if (cmd_args.count("input-geo-origin-file"))
    {
      input_geo_origin_file_ = cmd_args["input-geo-origin-file"].as<std::string>();
      config_->set_value("input_geo_origin_filename", input_geo_origin_file_);
    }
    if (cmd_args.count("video-file"))
    {
      video_source_ = cmd_args["video-file"].as<std::string>();
      config_->set_value("video_source", video_source_);
      // choose video or image list reader based on file extension
      config_->subblock_view("video_reader")->merge_config(
        load_default_video_input_config(video_source_));
    }
    if (cmd_args.count("cameras-dir"))
    {
      cameras_dir_ = cmd_args["cameras-dir"].as<std::string>();
      config_->set_value("cameras_dir", cameras_dir_);
    }
    if (cmd_args.count("output-mesh"))
    {
      output_mesh_ = cmd_args["output-mesh"].as<std::string>();
      config_->set_value("output_mesh", output_mesh_);
    }
    if (cmd_args.count("mask-file"))
    {
      mask_file_ = cmd_args["mask-file"].as<std::string>();
      config_->set_value("mask_source", mask_file_);
      // choose video or image list reader for masks based on file extension
      config_->subblock_view("mask_reader")->merge_config(
        load_default_video_input_config(mask_file_));
    }
    if (cmd_args.count("frame"))
    {
      frame_ = cmd_args["frame"].as<int>();
      config_->set_value("frame", frame_);
    }
    if (cmd_args.count("frame-sampling"))
    {
      frame_sampling_ = cmd_args["frame-sampling"].as<int>();
      config_->set_value("frame_sampling", frame_sampling_);
    }
    if (cmd_args.count("all-frames"))
    {
      all_frames_ = cmd_args["all-frames"].as<bool>();
      config_->set_value("all_frames", all_frames_);
    }
    if (cmd_args.count("active-attribute"))
    {
      active_attribute_ = cmd_args["active-attribute"].as<std::string>();
      config_->set_value("active_attribute", active_attribute_);
    }

    bool valid_config = check_config(config_);

    if (!opt_out_config.empty())
    {
      write_config_file(config_, opt_out_config);
      if (valid_config)
      {
        LOG_INFO(main_logger,
          "Configuration file contained valid parameters and may be "
          "used for running");
      }
      else
      {
        LOG_WARN(main_logger, "Configuration deemed not valid.");
      }
      config_ = nullptr;
      return WRITE;
    }
    else if (!valid_config)
    {
      LOG_ERROR(main_logger, "Configuration not valid.");
      config_ = nullptr;
      return FAIL;
    }

    // set variables from the config
    frame_sampling_ = config_->get_value("frame_sampling", frame_sampling_);
    frame_ = config_->get_value("frame", frame_);
    all_frames_ = config_->get_value("all_frames", all_frames_);
    occlusion_threshold_ = config_->get_value("occlusion_threshold", occlusion_threshold_);
    color_occluded_ = config_->get_value("color_occluded", color_occluded_);
    color_masked_ = config_->get_value("color_masked", color_masked_);
    remove_color_count_less_equal_ = config_->get_value("remove_color_count_less_equal", remove_color_count_less_equal_);

    return SUCCESS;
  }

  // ------------------------------------------------------------------
  kv::config_block_sptr default_config()
  {
    using kwiver::tools::load_default_video_input_config;
    typedef kwiver::tools::kwiver_applet kvt;
    auto config = kvt::find_configuration("applets/color_mesh.conf");

    // choose video or image list reader based on file extension
    config->subblock_view("video_reader")->merge_config(
      load_default_video_input_config(video_source_));
    // choose video or image list reader for masks based on file extension
    config->subblock_view("mask_reader")->merge_config(
      load_default_video_input_config(mask_file_));

    config->set_value("input_mesh", input_mesh_,
      "Path to an input mesh file in PLY, OBJ or VTP formats.");
    config->set_value("input_geo_origin_filename", "results/geo_origin.txt",
      "Path to a file to read the geographic origin from.");
    config->set_value("video_source", video_source_,
      "Path to an input file to be opened as a video. "
      "This could be either a video file or a text file "
      "containing new-line separated paths to sequential "
      "image files.");
    config->set_value("cameras_dir", cameras_dir_,
      "Directory containing cameras files (.krtd)");
    config->set_value(
      "output_mesh", output_mesh_,
      "Where to save the output mesh file in PLY or VTP formats."
      "Note that saving colors for several frames only works with the VTP format");
    config->set_value("mask_source", mask_file_,
      "Optional path to an mask input file to be opened "
      "as a video. "
      "This could be either a video file or a text file "
      "containing new-line separated paths to sequential "
      "image files. "
      "This list should be "
      "parallel in association to frames provided by the "
      "``video_source`` video. Mask images must be the same size "
      "as the image they are associated with.\n"
      "\n"
      "Leave this blank if no image masking is desired.");
    config->set_value(
      "frame_sampling", 1,
      "Used to choose frames for coloring. "
      "A frame is chosen if frame mod sampling == 0");
    config->set_value(
      "frame", 1,
      "Set color from frame");
    config->set_value(
      "all_frames", false,
      "Compute the average color or colors for all frames"
      "The selected frames are chosen using frame_sampling");
    config->set_value(
      "occlusion_threshold", 0.0,
      "We compare the depth buffer value with the depth of the mesh point. "
      "We use threshold >= 0 to fix floating point inaccuracies "
      "Default value is 0, bigger values will remove more points.");
    config->set_value(
      "color_occluded", false,
      "Color occluded points if parameter is true.");
    config->set_value(
      "active_attribute", active_attribute_,
      "Choose the active attribute between mean, median and count when saving "
      "a composite color (all-frames is false). "
      "For the VTP format, all attributes are saved, for PLY only the "
      "active attribute is saved.");
    config->set_value(
      "color_masked", false,
      "Color masked points if parameter is true.");

    kva::video_input::get_nested_algo_configuration("video_reader", config,
      kva::video_input_sptr());
    kva::video_input::get_nested_algo_configuration("mask_reader", config,
      kva::video_input_sptr());
    return config;
  }

  void initialize()
  {
    kva::video_input::set_nested_algo_configuration(
      "video_reader", config_, video_reader_);
    kva::video_input::set_nested_algo_configuration(
      "mask_reader", config_, mask_reader_);
  }

  static kv::camera_map_sptr load_camera_map(
    kva::video_input_sptr video_reader, std::string const& video_source,
    std::string const& cameras_dir)
  {
    if ( video_reader == nullptr )
    {
      return nullptr;
    }

    video_reader->open(video_source);

    kv::metadata_map_sptr metadata_map = video_reader->metadata_map();
    vital::camera_map::map_camera_t cameras;

    for (auto const& frame_metadata : metadata_map->metadata() )
    {
      const kv::metadata_vector mdv = frame_metadata.second;
      const size_t frame_ID = frame_metadata.first;
      const std::string name = basename_from_metadata(mdv, frame_ID);
      try
      {
        cameras[frame_ID] = vital::read_krtd_file( name, cameras_dir );
      }
      catch ( const vital::file_not_found_exception& )
      {
        continue;
      }
    }

    if ( cameras.empty() )
    {
      VITAL_THROW( vital::invalid_data, "No krtd files found" );
    }

    video_reader->close();
    return vital::camera_map_sptr( new vital::simple_camera_map( cameras ) );
  }

  static vtkSmartPointer<vtkPolyData> load_mesh(std::string const& input_mesh)
  {
    std::string ext = ST::GetFilenameExtension(input_mesh);
    if (ext == ".ply")
    {
      vtkNew<vtkPLYReader> reader;
      reader->SetFileName(input_mesh.c_str());
      reader->Update();
      vtkSmartPointer<vtkPolyData> mesh = reader->GetOutput();
      return mesh;
    }
    else if (ext ==  ".obj")
    {
      vtkNew<vtkOBJReader> reader;
      reader->SetFileName(input_mesh.c_str());
      reader->Update();
      vtkSmartPointer<vtkPolyData> mesh = reader->GetOutput();
      return mesh;
    }
    else if (ext == ".vtp")
    {
      vtkNew<vtkXMLPolyDataReader> reader;
      reader->SetFileName(input_mesh.c_str());
      reader->Update();
      vtkSmartPointer<vtkPolyData> mesh = reader->GetOutput();
      return mesh;
    }
    else
    {
      return nullptr;
    }
  }

  bool save_mesh(vtkSmartPointer<vtkPolyData> mesh,
                 char const * output_path)
  {
    std::string ext = ST::GetFilenameExtension(output_path);
    std::string filename_noext = ST::GetFilenameWithoutExtension(output_path);
    if (ext == ".vtp")
    {
      vtkNew<vtkXMLPolyDataWriter> writer;
      writer->SetFileName(output_path);
      writer->SetDataModeToBinary();
      writer->AddInputDataObject(mesh);
      writer->Write();
      return true;
    }
    else if (ext == ".ply")
    {
      vtkDataArray* scalars = mesh->GetPointData()->GetScalars();
      vtkNew<vtkPLYWriter> writer;
      writer->SetFileName(output_path);
      writer->SetArrayName(scalars->GetName());
      if (! vtkUnsignedCharArray::SafeDownCast(scalars))
      {
        // this is not a color, we have to use a lookup table to make it a color
        vtkNew<vtkLookupTable> lut;
        lut->SetHueRange(0.6, 0);
        lut->SetSaturationRange(1.0, 0);
        lut->SetValueRange(0.5, 1.0);
        lut->SetTableRange(scalars->GetRange());
        writer->SetLookupTable(lut);
      }
      writer->AddInputDataObject(mesh);
      writer->Write();
      return true;
    }
    else if (ext == ".las")
    {
      save_mesh_las(mesh, output_path);
      return true;
    }
    else
    {
      LOG_ERROR(main_logger, "Invalid file format: " << ext);
      return false;
    }
  }

  void save_mesh_las(vtkSmartPointer<vtkPolyData> mesh,
                     char const * output_path)
  {
    auto lgcs = vital::local_geo_cs();
    if (! read_local_geo_cs_from_file(lgcs, input_geo_origin_file_))
    {
      LOG_ERROR(main_logger, "Failed to read local geo cs from file: "
                << input_geo_origin_file_);
      return;
    }

    vtkSmartPointer<vtkPoints> inPts = mesh->GetPoints();
    vtkIdType numPts = inPts->GetNumberOfPoints();
    std::string colorArrayName = mesh->GetPointData()->GetScalars()->GetName();
    vtkSmartPointer<vtkDataArray> da = mesh->GetPointData()->GetArray(
      colorArrayName.c_str());
    vtkUnsignedCharArray* rgbArray = nullptr;

    std::vector<vital::vector_3d> points(numPts);
    std::vector<vital::rgb_color> colors;

    if (da->GetNumberOfComponents() == 3)
    {
      rgbArray = vtkArrayDownCast<vtkUnsignedCharArray>(da);
      colors.resize(numPts);
    }

    for (vtkIdType i = 0; i < numPts; ++i)
    {
      inPts->GetPoint(i, points[i].data());

      if (rgbArray)
      {
        vtkIdType idx = 3 * i;
        colors[i] = vital::rgb_color(rgbArray->GetValue(idx),
                                     rgbArray->GetValue(idx + 1),
                                     rgbArray->GetValue(idx + 2));
      }
    }

    auto pc_io = vital::algo::pointcloud_io::create("pdal");
    if (! pc_io)
    {
      LOG_ERROR(main_logger, "Could not find pointcloud_io algorithm pdal");
      return;
    }
    pc_io->set_local_geo_cs(lgcs);
    pc_io->save(output_path, points, colors);
  }

  bool run_algorithm()
  {
    // Attempt opening input and output files.
    //  - filepath validity checked above

    LOG_INFO(main_logger, "Reading video...");
    set_video(config_, video_source_);
    bool hasMask = !mask_file_.empty();
    if (hasMask)
    {
      set_mask(config_, mask_file_);
    }
    LOG_INFO(main_logger, "Load camera map...");
    auto cameras = load_camera_map(video_reader_, video_source_, cameras_dir_);
    set_cameras(cameras);
    LOG_INFO(main_logger, "Load mesh file...");
    auto input = load_mesh(input_mesh_);
    if (! input)
    {
      LOG_ERROR(main_logger, "Error loading the mesh");
      return false;
    }
    set_input(input);
    colorize();
    auto output = get_output();
    LOG_INFO(main_logger, "Save mesh file...");
    if (! all_frames_)
    {
      vtkDataArray* active = output->GetPointData()->GetArray(active_attribute_.c_str());
      if (! active)
      {
        active = output->GetPointData()->GetArray("mean");
      }
      output->GetPointData()->SetScalars(active);
    }
    if (! save_mesh(output, output_mesh_.c_str()))
    {
      return false;
    }
    return true;
  }

  void report_progress_changed(
    std::string const& message, int percentage) override
  {
        LOG_INFO(main_logger,
                 message << ": " << percentage << "%");
  }
};

// ----------------------------------------------------------------------------
int
color_mesh::
run()
{

  try
  {
    switch (d->process_command_line(command_args()))
    {
    case priv::HELP:
      std::cout << m_cmd_options->help();
      return EXIT_SUCCESS;
    case priv::WRITE:
      return EXIT_SUCCESS;
    case priv::FAIL:
      return EXIT_FAILURE;
    case priv::SUCCESS:
      ;
    }

    if (d->config_ == nullptr)
    {
      return EXIT_FAILURE;
    }

    d->initialize();
    if (!d->run_algorithm())
    {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }
  catch (std::exception const& e)
  {
    LOG_ERROR(main_logger, "Exception caught: " << e.what());

    return EXIT_FAILURE;
  }
  catch (...)
  {
    LOG_ERROR(main_logger, "Unknown exception caught");

    return EXIT_FAILURE;
  }
} // run

// ----------------------------------------------------------------------------
void
color_mesh::
add_command_options()
{
  m_cmd_options->custom_help( wrap_text(
    "[options] input-mesh video-file cameras-dir output-mesh\n"
    "This program colors an input-mesh from a video (or list of images) and "
    "a list of camera files stored in a directory. A mesh colored with "
    "the average color or with a color for a particular camera is produced.") );

  m_cmd_options->positional_help("\n  input-mesh  - input mesh file."
                                 "\n  video-file  - input video file."
                                 "\n  cameras-dir  - input camera directory."
                                 "\n  output-mesh - output mesh file."
  );

  m_cmd_options->add_options()
    ( "a,all-frames",
      "Compute average color or save each frame color",
      cxxopts::value<bool>()->default_value("false") )
    ( "c,config",
      "Configuration file for tool",
      cxxopts::value<std::string>() )
    ( "f,frame",
      "Frame index to use for coloring. "
      "If -1 use an average color for all frames.",
      cxxopts::value<int>()->default_value( "-1"))
    ( "g,input-geo-origin-file", "Input geographic origin file.",
      cxxopts::value<std::string>() )
    ( "h,help",     "Display applet usage" )
    ( "m,mask-file",
      "An input mask video or list of mask images to indicate "
      "which pixels to ignore.",
      cxxopts::value<std::string>())
    ( "o,output-config",
      "Output a configuration. This may be seeded with a "
      "configuration file from -c/--config.",
      cxxopts::value<std::string>() )
    ( "v,active-attribute",
      "Choose the active attribute between mean, median and count when saving "
      "a composite color (all-frames is false). "
      "For the VTP format, all attributes are saved, for PLY only the "
      "active attribute is saved.",
      cxxopts::value<std::string>())
    ( "s,frame-sampling",
      "Use for coloring only frames that satisfy frame mod sampling == 0",
      cxxopts::value<int>()->default_value( "1"))

    // positional parameters
    ("input-mesh", "Mesh input file", cxxopts::value<std::string>())
    ("video-file", "Video input file", cxxopts::value<std::string>())
    ("cameras-dir", "Camera input directory", cxxopts::value<std::string>())
    ("output-mesh", "Mesh output file", cxxopts::value<std::string>())
    ;

  m_cmd_options->parse_positional({
      "input-mesh", "video-file", "cameras-dir", "output-mesh" });
}

// ============================================================================
color_mesh::
color_mesh()
  : d(new priv())
{ }

color_mesh::
~color_mesh() = default;

} } } // end namespace
