// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "bundle_adjust_tool.h"

#include <arrows/mvg/metrics.h>
#include <arrows/mvg/sfm_utils.h>
#include <arrows/mvg/transform.h>

#include <kwiversys/Directory.hxx>
#include <kwiversys/SystemTools.hxx>

#include <vital/algo/bundle_adjust.h>
#include <vital/algo/triangulate_landmarks.h>
#include <vital/algo/video_input.h>
#include <vital/applets/applet_config.h>
#include <vital/applets/config_validation.h>
#include <vital/config/config_block.h>
#include <vital/config/config_block_io.h>
#include <vital/config/config_parser.h>
#include <vital/exceptions.h>
#include <vital/internal/cereal/external/rapidjson/document.h>
#include <vital/internal/cereal/external/rapidjson/istreamwrapper.h>
#include <vital/io/camera_from_metadata.h>
#include <vital/io/camera_io.h>
#include <vital/io/camera_map_io.h>
#include <vital/io/landmark_map_io.h>
#include <vital/io/metadata_io.h>
#include <vital/io/track_set_io.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/geodesy.h>
#include <vital/types/ground_control_point.h>
#include <vital/types/landmark.h>
#include <vital/util/get_paths.h>

#include <fstream>
#include <iostream>

namespace kwiver {

namespace arrows {

namespace mvg {

namespace kv = kwiver::vital;
using kv::feature_track_set_sptr;
using kv::algo::bundle_adjust;
using kv::algo::bundle_adjust_sptr;
using kv::algo::triangulate_landmarks;
using kv::algo::triangulate_landmarks_sptr;
using kv::algo::video_input;
using kv::algo::video_input_sptr;
using kv::camera_map_sptr;
using kv::camera_perspective;
using kv::camera_sptr;
using kv::landmark_map_sptr;
using kv::sfm_constraints;
using kv::sfm_constraints_sptr;

namespace {

typedef kwiversys::SystemTools ST;

kv::logger_handle_t logger( kv::get_logger( "bundle_adjust_tool" ) );

// ----------------------------------------------------------------------------
bool
check_config( kv::config_block_sptr config )
{
  using namespace kwiver::tools;

  bool config_valid = true;

#define KWIVER_CONFIG_FAIL( msg ) \
  LOG_ERROR( logger, "config check fail: " << msg ); \
  config_valid = false

  config_valid =
    validate_required_input_file( "GCP_filename", *config, logger ) &&
    config_valid;

  config_valid =
    validate_required_input_file( "input_cameras", *config, logger ) &&
    config_valid;

  config_valid =
    validate_required_output_dir( "output_cameras_directory", *config,
                                  logger ) &&
    config_valid;

  config_valid =
    validate_optional_input_file( "video_source", *config, logger ) &&
    config_valid;

  config_valid =
    validate_optional_input_file( "input_tracks_file", *config, logger ) &&
    config_valid;

  config_valid =
    validate_required_output_file( "output_landmarks_filename", *config,
                                   logger ) &&
    config_valid;

  config_valid =
    validate_optional_output_file( "geo_origin_filename", *config, logger ) &&
    config_valid;

  if( !video_input::check_nested_algo_configuration( "video_reader", config ) )
  {
    KWIVER_CONFIG_FAIL( "video_reader configuration check failed" );
  }

  if( !bundle_adjust::check_nested_algo_configuration( "bundle_adjust",
                                                       config ) )
  {
    KWIVER_CONFIG_FAIL( "bundle-adjust configuration check failed" );
  }

#undef KWIVER_CONFIG_FAIL

  return config_valid;
}

} // namespace

struct GroundControlPoint
{
  kv::ground_control_point_sptr gcp;
  kv::track_sptr feature;
};

// ----------------------------------------------------------------------------
// Keys
const auto TAG_TYPE               = "type";
const auto TAG_FEATURES           = "features";
const auto TAG_GEOMETRY           = "geometry";
const auto TAG_PROPERTIES         = "properties";
const auto TAG_COORDINATES        = "coordinates";

// Values
const auto TAG_FEATURE            = std::string( "Feature" );
const auto TAG_FEATURECOLLECTION  = std::string( "FeatureCollection" );
const auto TAG_POINT              = std::string( "Point" );

// Property keys (not part of GeoJSON specification)
const auto TAG_NAME               = "name";
const auto TAG_FRAME              = "frameId";
const auto TAG_FRAMES             = std::string( "frames" );
const auto TAG_LOCATION           = "location";
const auto TAG_REGISTRATIONS      = "registrations";
const auto TAG_USER_REGISTERED    = "userRegistered";

// ----------------------------------------------------------------------------
class GCP_helper
{
  using id_t = kwiver::vital::ground_control_point_id_t;
  using gcp_sptr = kv::ground_control_point_sptr;

  std::map< id_t, GroundControlPoint > groundControlPoints;
  id_t nextId = 0;

  void
  addPoint( id_t id, gcp_sptr const& point )
  {
    groundControlPoints[ id ].gcp = point;
  }

public:

  bool
  hasPoints() const
  {
    return !groundControlPoints.empty();
  }

  bool readGroundControlPoints( std::string const& path );

  kv::feature_track_set_sptr registrationTracks() const;
  kv::landmark_map_sptr registrationLandmarks() const;
};

// ----------------------------------------------------------------------------
template < typename T >
bool
isDoubleArray( rapidjson::GenericArray< true, T > const& a )
{
  for( auto const& v : a )
  {
    if( !v.IsDouble() )
    {
      return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
template < typename T >
GroundControlPoint
extractGroundControlPoint(
  rapidjson::GenericObject< true, T > const& f )
{
  // Check for geometry
  if( !f.HasMember( TAG_GEOMETRY ) )
  {
    LOG_DEBUG( logger, "ignoring feature with missing geometry" );
    return {};
  }

  auto const& geom = f[ TAG_GEOMETRY ].GetObject();
  if( geom.ObjectEmpty() )
  {
    LOG_DEBUG( logger, "ignoring feature with invalid geometry" );
    return {};
  }
  // Check geometry type
  if( geom[ TAG_TYPE ].GetString() != TAG_POINT )
  {
    // Non-point features are silently ignored
    return {};
  }

  // Create GCPs
  auto gcp = std::make_shared< kv::ground_control_point >();

  // Read feature track
  auto ft = kv::track::create();

  // Check for valid coordinates
  if( geom.HasMember( TAG_COORDINATES ) )
  {
    auto const& coords = geom[ TAG_COORDINATES ].GetArray();
    auto const size = coords.Size();
    if( size >= 2 && size <= 3 && isDoubleArray( coords ) )
    {
      // Set world location and elevation; per the GeoJSON specification
      // (RFC 7946), the coordinates shall have been specified in WGS'84
      constexpr static auto gcs = kv::SRID::lat_lon_WGS84;
      kv::vector_3d loc( coords[ 0 ].GetDouble(),
                         coords[ 1 ].GetDouble(), 0.0 );
      if( size > 2 )
      {
        loc[ 2 ] = coords[ 2 ].GetDouble();
      }
      gcp->set_geo_loc( { loc, gcs } );
    }
  }

  // Get properties
  if( f.HasMember( TAG_PROPERTIES ) )
  {
    auto const& props = f[ TAG_PROPERTIES ].GetObject();
    if( props.HasMember( TAG_NAME ) )
    {
      gcp->set_name( props[ TAG_NAME ].GetString() );
    }
    else
    {
      LOG_DEBUG( logger, "missing member: " << TAG_NAME );
    }

    if( props.HasMember( TAG_LOCATION ) )
    {
      auto const& lct = props[ TAG_LOCATION ].GetArray();
      if( lct.Size() == 3 && isDoubleArray( lct ) )
      {
        gcp->set_loc( { lct[ 0 ].GetDouble(),
                        lct[ 1 ].GetDouble(), lct[ 2 ].GetDouble() } );
      }
      else if( gcp->geo_loc().is_empty() )
      {
        gcp = nullptr;
      }
      else if( props.HasMember( TAG_USER_REGISTERED ) )
      {
        gcp->set_geo_loc_user_provided( props[ TAG_USER_REGISTERED ].GetBool() );
      }
    }

    if( props.HasMember( TAG_REGISTRATIONS ) )
    {
      auto const& regs = props[ TAG_REGISTRATIONS ].GetArray();
      for( auto const& riter : regs )
      {
        if( !riter.IsObject() )
        {
          continue;
        }

        auto const& reg = riter.GetObject();
        if( reg.HasMember( TAG_FRAME ) )
        {
          auto const& frame = reg[ TAG_FRAME ];
          if( frame.IsUint() )
          {
            if( reg.HasMember( TAG_LOCATION ) )
            {
              auto const& loc = reg[ TAG_LOCATION ].GetArray();
              if( loc.Size() == 2 && isDoubleArray( loc ) )
              {
                auto const t =
                  static_cast< kv::frame_id_t >( frame.GetUint() );
                auto feature = std::make_shared< kv::feature_d >();
                feature->set_loc( { loc[ 0 ].GetDouble(),
                                    loc[ 1 ].GetDouble() } );
                ft->insert(
                  std::make_shared< kv::feature_track_state >( t,
                                                               std::move(
                                                                 feature ) ) );
              }
            } // TAG_LOCATION
          }
        } // TAG_FRAME
      } // loop regs
    } // TAG_REGISTRATIONS
    if( ft->empty() )
    {
      ft = nullptr;
    }

    // Check if we read anything usable
    if( !gcp && !ft )
    {
      LOG_DEBUG( logger, "ignoring point feature" <<
                 " with no valid location information" );
      return {};
    }
  } // TAG_PROPERTIES

  return { gcp, ft };
}

// ----------------------------------------------------------------------------
bool
GCP_helper
::readGroundControlPoints( std::string const& path )
{
  auto fail = [ &path ]( char const* extra ) {
                LOG_ERROR( logger,
                    "failed to read ground control points from " <<
                           path << ": " << extra );
                return false;
              };

  // Open input file
  std::ifstream f( path );
  rapidjson::IStreamWrapper ifs( f );
  if( !f.is_open() )
  {
    return fail( "unable to open" );
  }

  // Read raw JSON data
  rapidjson::Document doc;
  doc.ParseStream( ifs );
  if( doc.IsNull() )
  {
    return fail( "failed to parse JSON" );
  }
  if( !doc.IsObject() )
  {
    return fail( "invalid JSON object" );
  }

  auto const& collection = doc.GetObject();
  if( !collection.HasMember( TAG_TYPE ) ||
      collection[ TAG_TYPE ].GetString() != TAG_FEATURECOLLECTION )
  {
    return fail( "root object must be a FeatureCollection" );
  }
  if( !collection.HasMember( TAG_FEATURES ) )
  {
    return fail( "expected a 'features' array" );
  }

  auto const& features = collection[ TAG_FEATURES ];
  if( !features.IsArray() )
  {
    return fail( "invalid FeatureCollection: must be an array" );
  }

  for( auto const& ftr : features.GetArray() )
  {
    auto fo = ftr.GetObject();
    if( fo[ TAG_TYPE ].GetString() != TAG_FEATURE )
    {
      LOG_WARN( logger, "ignoring non-feature object" << ftr.GetString() <<
                "in FeatureCollection" );
      continue;
    }

    auto const& gcp = extractGroundControlPoint( fo );

    if( gcp.gcp )
    {
      addPoint( nextId, gcp.gcp );
    }
    if( gcp.feature )
    {
      gcp.feature->set_id( nextId );
      groundControlPoints[ nextId ].feature = gcp.feature;
    }
    if( gcp.gcp || gcp.feature )
    {
      ++nextId;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
kv::feature_track_set_sptr
GCP_helper
::registrationTracks() const
{
  auto out = std::make_shared< kv::feature_track_set >();
  for( auto const& iter : groundControlPoints )
  {
    auto ftr = iter.second.feature;
    if( ftr )
    {
      // assume all registered features are inliers
      for( auto ts : *ftr )
      {
        auto fts = std::dynamic_pointer_cast< kv::feature_track_state >( ts );
        if( fts )
        {
          fts->inlier = true;
        }
      }

      auto attrs = ftr->attributes();
      if( attrs == nullptr )
      {
        attrs = std::make_shared< kv::attribute_set >();
      }
      attrs->add( "trusted", true );
      ftr->set_attributes( attrs );
      out->insert( ftr );
    }
  }
  return out;
}

// ----------------------------------------------------------------------------
kv::landmark_map_sptr
GCP_helper
::registrationLandmarks() const
{
  auto landmarks = kv::landmark_map::map_landmark_t{};
  for( auto const& iter : groundControlPoints )
  {
    if( iter.second.gcp )
    {
      auto lm = std::make_shared< kv::landmark_d >( iter.second.gcp->loc() );
      landmarks.emplace( iter.first, lm );
    }
  }
  return std::make_shared< kv::simple_landmark_map >( landmarks );
}

// ----------------------------------------------------------------------------
struct bundle_adjust_tool::priv
{
  priv( bundle_adjust_tool* parent ) : p( parent ) {}

  bundle_adjust_tool* p = nullptr;
  camera_map_sptr camera_map_ptr;
  landmark_map_sptr landmark_map_ptr;
  feature_track_set_sptr feature_track_set_ptr;
  sfm_constraints_sptr sfm_constraint_ptr;

  GCP_helper gcp_helper;
  bundle_adjust_sptr algo_bundle_adjust;
  triangulate_landmarks_sptr algo_triangulate_landmarks;
  kv::config_block_sptr config;
  kv::path_t video_file;
  kv::path_t tracks_file;
  kv::path_t cam_in;
  size_t num_frames = 0;
  kv::path_t cam_out_dir = "results/krtd";
  kv::path_t landmarks_file = "results/landmarks.ply";
  kv::path_t geo_origin_file = "results/geo_origin.txt";
  kv::path_t GCPFN = "gcps.json";
  bool ignore_metadata = false;

  using mapID2FN = std::unordered_map<unsigned, kv::path_t>;
  mapID2FN camID2FN;

  enum commandline_mode { SUCCESS, HELP, WRITE, FAIL, };

  commandline_mode
  process_command_line( cxxopts::ParseResult& cmd_args )
  {
    using kwiver::tools::load_default_video_input_config;

    static std::string opt_config;
    static std::string opt_out_config;

    if ( cmd_args["help"].as<bool>() )
    {
      return HELP;
    }
    if( cmd_args.count( "config" ) > 0 )
    {
      opt_config = cmd_args[ "config" ].as< std::string >();
    }
    if( cmd_args.count( "output-config" ) > 0 )
    {
      opt_out_config = cmd_args[ "output-config" ].as< std::string >();
    }

    // Set up top level configuration w/ defaults where applicable.
    config = default_config();

    // If -c/--config given, read in confg file, merge in with default just
    // generated
    if( !opt_config.empty() )
    {
      config->merge_config( kv::read_config_file( opt_config ) );
    }

    if( cmd_args.count( "tracks" ) > 0 )
    {
      tracks_file = cmd_args[ "tracks" ].as< std::string >();
      config->set_value( "input_tracks_file", tracks_file );
    }
    if( cmd_args.count( "video" ) > 0 )
    {
      video_file = cmd_args[ "video" ].as< std::string >();
      config->set_value( "video_source", video_file );
      // choose video or image list reader based on file extension
      config->subblock_view( "video_reader" )->merge_config(
        load_default_video_input_config( video_file ) );
    }
    if( cmd_args.count( "cam_in" ) > 0 )
    {
      cam_in = cmd_args[ "cam_in" ].as< std::string >();
      config->set_value( "input_cameras", cam_in );
    }
    if( cmd_args.count( "cam_out" ) > 0 )
    {
      cam_out_dir = cmd_args[ "cam_out" ].as< std::string >();
      config->set_value( "output_cameras_directory", cam_out_dir );
    }
    if( cmd_args.count( "landmarks" ) > 0 )
    {
      landmarks_file = cmd_args[ "landmarks" ].as< std::string >();
      config->set_value( "output_landmarks_filename", landmarks_file );
    }
    if( cmd_args.count( "geo-origin" ) > 0 )
    {
      geo_origin_file = cmd_args[ "geo-origin" ].as< std::string >();
      config->set_value( "geo_origin_filename", geo_origin_file );
    }
    if( cmd_args.count( "GCP" ) > 0 )
    {
      GCPFN = cmd_args[ "GCP" ].as< std::string >();
      config->set_value( "GCP_filename", GCPFN );
    }

    bool valid_config = check_config( config );

    if( !opt_out_config.empty() )
    {
      write_config_file( config, opt_out_config );
      if( valid_config )
      {
        LOG_INFO( logger,
                  "configuration file is valid and may be used for running" );
      }
      else
      {
        LOG_WARN( logger, "Configuration is invalid." );
      }
      config = nullptr;
      return WRITE;
    }
    else if( !valid_config )
    {
      LOG_ERROR( logger, "Configuration is invalid." );
      config = nullptr;
      return FAIL;
    }

    return SUCCESS;
  }

  kv::config_block_sptr
  default_config()
  {
    using kwiver::tools::load_default_video_input_config;
    typedef kwiver::tools::kwiver_applet kvt;

    auto config = kvt::find_configuration( "applets/bundle_adjust_tool.conf" );

    // choose video or image list reader based on file extension
    config->subblock_view( "video_reader" )->merge_config(
      load_default_video_input_config( video_file ) );

    config->set_value( "video_source", video_file,
                       "(optional) Path to an input file to be opened as a video. "
                       "This could be either a video file or a text file "
                       "containing new-line separated paths to sequential "
                       "image files. In this tool, video is only used to extract "
                       "metadata such as geospatial tags." );

    config->set_value( "input_tracks_file", tracks_file,
                       "(optional) Path to a file to input tracks from." );

    config->set_value( "input_cameras", cam_in,
                       "Path to a file to read camera models from." );

    config->set_value( "output_cameras_directory", cam_out_dir,
                       "Directory to write camera models to." );

    config->set_value( "output_landmarks_filename", landmarks_file,
                       "(optional) Path to a file to output landmarks to. "
                       "If this file exists, it will be overwritten." );

    config->set_value( "geo_origin_filename", geo_origin_file,
                       "(optional) Path to a file to write the geographic origin. "
                       "This file is only written if the geospatial metadata is "
                       "provided as input (e.g. in the input video). "
                       "If this file exists, it will be overwritten." );

    config->set_value( "ignore_metadata", ignore_metadata,
                       "Do not scan the video file for metadata." );

    bundle_adjust::get_nested_algo_configuration(
      "bundle_adjust", config, nullptr );
    video_input::get_nested_algo_configuration(
      "video_reader", config, nullptr );
    return config;
  }

  void initialize()
  {
    // Create algo_bundle_adjust from configuration
    bundle_adjust::set_nested_algo_configuration(
      "bundle_adjust", config, algo_bundle_adjust );
    // Create algo_triangulate_landmarks from configuration
    triangulate_landmarks::set_nested_algo_configuration(
      "triangulator", config, algo_triangulate_landmarks );
  }

  void clear_ptrs()
  {
    camera_map_ptr = nullptr;
    landmark_map_ptr = nullptr;
    feature_track_set_ptr = nullptr;
    sfm_constraint_ptr = nullptr;
  }

  void load_tracks();

  void load_cameras();

  void load_sfm_constraint();

  void load_GCP()
  {
    gcp_helper.readGroundControlPoints( GCPFN );
  }

  bool hasGCP() const
  {
    return gcp_helper.hasPoints();
  }

  bool write_cameras();

  bool write_landmarks()
  {
    kv::path_t out_landmarks_path =
      config->get_value< kv::path_t >( "output_landmarks_filename" );
    write_ply_file( landmark_map_ptr, out_landmarks_path );
    return true;
  }

  bool write_geo_origin()
  {
    if( sfm_constraint_ptr )
    {
      auto lgcs = sfm_constraint_ptr->get_local_geo_cs();
      if( !lgcs.origin().is_empty() )
      {
        kv::write_local_geo_cs_to_file( lgcs, geo_origin_file );
        return true;
      }
    }
    return false;
  }

  std::string get_filename( kv::frame_id_t frame_id );

  void run_algorithm();

};

// ----------------------------------------------------------------------------
std::string
bundle_adjust_tool::priv
::get_filename( kv::frame_id_t frame_id )
{
  if( !camID2FN.empty() )
  {
    auto i = camID2FN.find(frame_id);
    if( i != camID2FN.end() )
    {
      return i->second;
    }
  }
  if( sfm_constraint_ptr && sfm_constraint_ptr->get_metadata() )
  {
    auto videoMetadataMap = sfm_constraint_ptr->get_metadata();
    auto mdv = videoMetadataMap->get_vector( frame_id );
    if( !mdv.empty() )
    {
      return basename_from_metadata( mdv, frame_id );
    }
  }
  auto dummy_md = std::make_shared< kv::metadata >();
  dummy_md->add< kv::VITAL_META_VIDEO_URI >( std::string( video_file ) );
  return basename_from_metadata( dummy_md, frame_id );
}

// ----------------------------------------------------------------------------
void
bundle_adjust_tool::priv
::load_tracks()
{
  if( !config )
  {
    return;
  }
  tracks_file =
    config->get_value< kv::path_t >( "input_tracks_file" );
  if( tracks_file.empty() )
  {
    LOG_INFO( logger, "no input tracks" );
    return;
  }
  feature_track_set_ptr =
    kv::read_feature_track_file( tracks_file );
}

// ----------------------------------------------------------------------------
void
bundle_adjust_tool::priv
::load_cameras()
{
  if( !config )
  {
    LOG_WARN( logger, "no config to load cameras");
    return;
  }
  cam_in =
    config->get_value< kv::path_t >( "input_cameras" );
  if( cam_in.empty() )
  {
    LOG_WARN( logger, "no input cameras" );
    return;
  }
  std::ifstream f( cam_in );
  std::string FN;
  kv::camera_map::map_camera_t cameras; // keys expected to be 1-based
  for( unsigned id=1 ; std::getline( f, FN ); ++id )
  {
    LOG_INFO( logger, FN );
    try
    {
      cameras[id] = kv::read_krtd_file( FN );
      camID2FN[id] =
        kwiversys::SystemTools::GetFilenameWithoutLastExtension( FN );
    }
    catch ( std::exception const & e )
    {
      LOG_WARN( logger, "no camera from " << FN << "; error: " << e.what() );
      continue;
    }
  }
  camera_map_ptr = kv::camera_map_sptr( new kv::simple_camera_map( cameras ) );
}

// ----------------------------------------------------------------------------
bool
bundle_adjust_tool::priv
::write_cameras()
{
  std::string output_cameras_directory =
    config->get_value< std::string >( "output_cameras_directory" );
  for( auto iter : camera_map_ptr->cameras() )
  {
    int fid = iter.first;
    std::string const FN = get_filename( fid );
    camera_sptr cam = iter.second;
    std::string out_fname =
      output_cameras_directory + "/" + FN + ".krtd";
    kv::path_t out_path( out_fname );
    LOG_DEBUG( logger, "output cam id=" << fid << " to " << out_path );
    auto cam_ptr = std::dynamic_pointer_cast< camera_perspective >( cam );
    if( !cam )
    {
      LOG_ERROR( logger, "null perspective camera for " <<  out_fname );
      continue;
    }
    write_krtd_file( *cam_ptr, out_path );
  }
  return true;
}

// ----------------------------------------------------------------------------
void
bundle_adjust_tool::priv
::run_algorithm()
{
  if( feature_track_set_ptr )
  {
    // Create cameras, if they are in the map but null,
    // add a placeholder for each missing camera in the map.
    if( camera_map_ptr )
    {
      using kv::frame_id_t;
      using kv::camera_map;

      std::set< frame_id_t > frame_ids = feature_track_set_ptr->all_frame_ids();
      num_frames = frame_ids.size();

      camera_map::map_camera_t all_cams = camera_map_ptr->cameras();

      for( auto const& id : frame_ids )
      {
        if( all_cams.find( id ) == all_cams.end() )
        {
          all_cams[ id ] = kv::camera_sptr();
        }
      }
      camera_map_ptr = std::make_shared< kv::simple_camera_map >( all_cams );

      if( !landmark_map_ptr )
      {
        if ( feature_track_set_ptr )
        {
          auto cp = camera_map_ptr;
          auto tp = feature_track_set_ptr;

          kwiver::vital::landmark_map::map_landmark_t init_lms;
          std::set<kwiver::vital::track_id_t> track_ids = tp->all_track_ids();

          // Landmarks to triangulate must be created in the map.
          // These could be initialized to Null, but some versions of KWIVER may
          // crash, so it is safer to give them a value
          kwiver::vital::vector_3d const init_loc(0, 0, 0);
          for (auto const& id : track_ids)
          {
            init_lms[id] = std::make_shared<kwiver::vital::landmark_d>(init_loc);
          }
          kwiver::vital::landmark_map_sptr lp =
            std::make_shared<kwiver::vital::simple_landmark_map>(init_lms);

          algo_triangulate_landmarks->triangulate(cp, tp, lp);

          landmark_map_ptr = lp;
          feature_track_set_ptr = tp;
        }
        else
        {
          LOG_WARN( logger, "landmark triangulation algorithm is null" );
        }
      }
    }
  }

  //== handle manual annotation tracks and landmarks as trusted
  if( gcp_helper.hasPoints() )
  {
    auto reg_tracks = gcp_helper.registrationTracks();
    std::map< kv::track_id_t, kv::track_id_t > TrackIDRemap;
    if( reg_tracks->size() )
    {
      // combine registration and computed tracks
      if( feature_track_set_ptr )
      {
        // map track IDs to tracks
        std::map< kv::track_id_t, kv::track_sptr > TrackID2Track;
        // compute max track ID for re-mapping
        kv::track_id_t TrackIDmax = 0;
        for( auto const& trk : feature_track_set_ptr->tracks() )
        {
          auto ID = trk->id();
          TrackID2Track[ ID ] = trk;
          if( ID > TrackIDmax )
          {
            TrackIDmax = ID;
          }
        }
        for( auto const& trk : reg_tracks->tracks() )
        {
          auto ID = trk->id();
          auto it = TrackID2Track.find( ID );
          if( it == TrackID2Track.end() )
          {
            TrackID2Track[ ID ] = trk; // insert trusted
          }
          else // collision, re-map non-manual ID
          {
            auto attrs = it->second->attributes(); // trusted?
            if( attrs && attrs->has( "trusted" ) ) // erase the old manual
                                                   // track
            {
              TrackID2Track[ ID ] = trk; // replacing
            }
            else // re-map the regular track
            {
              auto newID = ++TrackIDmax;
              TrackIDRemap[ ID ] = newID; // use for landmark re-mapping
              it->second->set_id( newID );
              TrackID2Track[ newID ] = trk;
            }
          }
        }
        feature_track_set_ptr = std::make_shared< kv::feature_track_set >();
        for( auto const& tr : TrackID2Track )
        {
          feature_track_set_ptr->insert( tr.second );
        }
      }
      else
      {
        feature_track_set_ptr = reg_tracks;
      }
    }

    auto RgstLmks = gcp_helper.registrationLandmarks();
    if( RgstLmks->size() )
    {
      // combine GCPs with landmarks
      if( landmark_map_ptr )
      {
        kv::landmark_map::map_landmark_t lmks;
        for( auto const& tlm : landmark_map_ptr->landmarks() )
        {
          // re-map track IDs, if needed
          auto TID = tlm.first;
          auto it = TrackIDRemap.find( TID );
          if( it == TrackIDRemap.end() )
          {
            lmks.insert( tlm );
          }
          else // re-map track ID
          {
            lmks[ it->second ] = tlm.second;
          }
        }
        // trust manually picked landmarks in case of ID collision
        for( auto& rlm : RgstLmks->landmarks() )
        {
          lmks[ rlm.first ] = rlm.second;
        }
        landmark_map_ptr = std::make_shared< kv::simple_landmark_map >( lmks );
      }
      else
      {
        landmark_map_ptr = RgstLmks;
      }
    }
  }

  //== optimize
  kv::simple_camera_perspective_map cams;
  unsigned min_frm_id = -1;
  for( auto const& p : camera_map_ptr->cameras() )
  {
    auto ID = p.first;
    if ( min_frm_id > ID)
    {
      min_frm_id = ID;
    }
    auto c =
      std::dynamic_pointer_cast< kv::simple_camera_perspective >( p.second );
    if( c )
    {
      cams.insert( ID, c );
    }
  }
  if( !landmark_map_ptr )
  {
    throw std::logic_error( "no landmarks" );
  }
  if( !feature_track_set_ptr )
  {
    throw std::logic_error( "no feature tracks" );
  }

  auto lms = landmark_map_ptr->landmarks();
  std::set< kv::landmark_id_t > fixed_landmarks;
  std::vector< kv::track_sptr > trusted_tracks;
  for( auto t : feature_track_set_ptr->tracks() )
  {
    auto attrs = t->attributes();
    if( attrs && attrs->has( "trusted" ) )
    {
      trusted_tracks.push_back( t );
      fixed_landmarks.insert( t->id() );
    }
  }
  using kwiver::arrows::mvg::reprojection_rmse;

  auto const& cms = cams.cameras();
  std::set< vital::frame_id_t > fixed_cameras;
  auto err = reprojection_rmse( cms, lms, trusted_tracks );
  LOG_DEBUG( logger, "initial re-projection RMSE: " << err );

  algo_bundle_adjust->optimize( cams, lms, feature_track_set_ptr,
                                fixed_cameras,
                                fixed_landmarks, sfm_constraint_ptr );

  err = reprojection_rmse( cms, lms, trusted_tracks );
  LOG_DEBUG( logger, "final re-projection RMSE: " << err );

  landmark_map_ptr = std::make_shared< kv::simple_landmark_map >( lms );
  camera_map_ptr =
    std::make_shared< kv::simple_camera_perspective_map >( cams );
}

// ----------------------------------------------------------------------------
void
bundle_adjust_tool::priv
::load_sfm_constraint()
{
  sfm_constraint_ptr = std::make_shared< sfm_constraints >();

  if( config == nullptr )
  {
    LOG_WARN( logger, "config is null" );
    return;
  }

  if( config->get_value< bool >( "ignore_metadata", false ) )
  {
    LOG_INFO( logger, "ignoring meta-data" );
    video_file = config->get_value< std::string >( "video_source" );
    return;
  }

  if( config->has_value( "input_cameras" ) )
  {
    LOG_INFO( logger, "ignoring input video/images, using input camera priors" );
    return;
  }

  kv::image_container_sptr first_frame;
  if( config->has_value( "video_source" ) &&
      !config->get_value< std::string >( "video_source" ).empty() )
  {
    video_input_sptr video_reader;
    video_file = config->get_value< std::string >( "video_source" );
    video_input::set_nested_algo_configuration(
      "video_reader", config, video_reader );
    video_reader->open( video_file );
    if( video_reader->get_implementation_capabilities()
        .has_capability( video_input::HAS_METADATA ) )
    {
      sfm_constraint_ptr->set_metadata( video_reader->metadata_map() );

      kv::timestamp ts;
      video_reader->next_frame( ts );
      first_frame = video_reader->frame_image();
    }
    else
    {
      LOG_WARN( logger,
        "no meta-data in video file/image list input" );
      return;
    }
  }
  else
  {
    LOG_INFO( logger, "no video source or image list" );
    return;
  }

  using kv::simple_camera_intrinsics;
  using kv::simple_camera_perspective;
  using kv::frame_id_t;
  using kv::metadata_sptr;
  using kv::intrinsics_from_metadata;
  using kv::local_geo_cs;

#define GET_K_CONFIG( type, name ) \
  config->get_value< type >( bc + #name, K_def.name() )

  simple_camera_intrinsics K_def;
  const std::string bc = "video_reader:base_camera:";
  auto K = std::make_shared< simple_camera_intrinsics >(
    GET_K_CONFIG( double, focal_length ),
    GET_K_CONFIG( kv::vector_2d, principal_point ),
    GET_K_CONFIG( double, aspect_ratio ),
    GET_K_CONFIG( double, skew ) );

  auto base_camera = simple_camera_perspective();
  base_camera.set_intrinsics( K );
  auto md = sfm_constraint_ptr->get_metadata()->metadata();
  if( !md.empty() )
  {
    std::map< frame_id_t, metadata_sptr > md_map;

    for( auto const& md_iter : md )
    {
      // NOTE: just using first element of metadata vector for now
      md_map[ md_iter.first ] = md_iter.second[ 0 ];
    }

    bool init_cams_with_metadata =
      config->get_value< bool >( "initialize_cameras_with_metadata", true );

    if( init_cams_with_metadata )
    {
      auto im = first_frame;
      K->set_image_width( static_cast< unsigned >( im->width() ) );
      K->set_image_height( static_cast< unsigned >( im->height() ) );
      base_camera.set_intrinsics( K );

      bool init_intrinsics_with_metadata =
        config->get_value< bool >( "initialize_intrinsics_with_metadata",
                                   true );
      if( init_intrinsics_with_metadata )
      {
        // find the first metadata that gives valid intrinsics
        // and put this in baseCamera as a backup for when
        // a particular metadata packet is missing data
        for( auto mdp : md_map )
        {
          auto md_K =
            intrinsics_from_metadata( *mdp.second,
                                      static_cast< unsigned >( im->width() ),
                                      static_cast< unsigned >( im->height() ) );
          if( md_K != nullptr )
          {
            base_camera.set_intrinsics( md_K );
            break;
          }
        }
      }

      local_geo_cs lgcs = sfm_constraint_ptr->get_local_geo_cs();
      kv::camera_map::map_camera_t cam_map =
        initialize_cameras_with_metadata( md_map, base_camera, lgcs,
                                          init_intrinsics_with_metadata );
      camera_map_ptr =
        std::make_shared< kv::simple_camera_map >( cam_map );
      sfm_constraint_ptr->set_local_geo_cs( lgcs );
    }
  }
}

// ----------------------------------------------------------------------------
int
bundle_adjust_tool
::run()
{
  try
  {
    switch( d->process_command_line( command_args() ) )
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

    if( d->config == nullptr )
    {
      return EXIT_FAILURE;
    }

    if( d->algo_bundle_adjust == nullptr )
    {
      d->initialize();
    }

    if( d->feature_track_set_ptr == nullptr )
    {
      d->load_tracks();
    }
    // TODO optionally load landmarks
    if( d->sfm_constraint_ptr == nullptr )
    {
      d->load_sfm_constraint();
    }

    if( d->camera_map_ptr == nullptr )
    {
      d->load_cameras();
    }

    if( !d->hasGCP() )
    {
      d->load_GCP();
    }

    d->run_algorithm();

    if( !d->write_cameras() )
    {
      return EXIT_FAILURE;
    }

    if( !d->write_landmarks() )
    {
      return EXIT_FAILURE;
    }

    if( d->write_geo_origin() )
    {
      LOG_INFO( logger, "Saved geo-origin to " << d->geo_origin_file );
    }

    return EXIT_SUCCESS;
  }
  catch ( std::exception const& e )
  {
    LOG_ERROR( logger, "exception: " << e.what() );
    return EXIT_FAILURE;
  }
  catch ( ... )
  {
    LOG_ERROR( logger, "unknown exception" );
    return EXIT_FAILURE;
  }
} // run

// ----------------------------------------------------------------------------
void
bundle_adjust_tool
::add_command_options()
{
  m_cmd_options->custom_help( wrap_text( "[options]\n" ) );
  m_cmd_options->add_options()
  ( "h,help",   "display applet usage" )
  ( "c,config", "configuration file for tool",
                                cxxopts::value< std::string >() )
  ( "o,output-config",
    "output a configuration, which may be seeded with "
    "a configuration file from -c/--config",
    cxxopts::value< std::string >() )
  ( "p,GCP",
    "input 3D Ground Control Points (GCP) with corresponding "
    "2D Camera Registration Points (CRP) as JSON file",
    cxxopts :: value< std::string >() )
  ( "v,video", "input video file or image.txt list",
    cxxopts::value< std::string >() )
  ( "t,tracks", "input tracks.txt", cxxopts::value< std::string >() )
  ( "i,cam_in", "input camera models.txt list",
    cxxopts::value< std::string >() )
  ( "k,cam_out", "output directory for camera models",
    cxxopts::value< std::string >() )
  ( "l,landmarks", "output landmarks.ply file",
    cxxopts::value< std::string >() )
  ( "g,geo-origin", "output geographic origin file",
    cxxopts::value< std::string >() )
  ;
  // to remove tracks reading from the config, add this
  // m_cmd_options->parse_positional("tracks");
}

// ============================================================================
bundle_adjust_tool
::bundle_adjust_tool()
  : d( new priv( this ) )
{}

// ----------------------------------------------------------------------------
bundle_adjust_tool
::~bundle_adjust_tool()
{}

} // mvg

} // arrows

} // kwiver
