// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of camera rig I/O functions

#include "camera_rig_io.h"
#include "camera_io.h"

#include <vital/exceptions.h>
#include <vital/internal/cereal/archives/json.hpp>
#include <vital/internal/cereal/types/vector.hpp>

#include <kwiversys/SystemTools.hxx>

#include <fstream>
#include <stdexcept>

namespace { // anon

//
// helper class for left / right camera intrinsics shared beween json and yaml loaders
//
// (since this class has a default ctor, we can store it in a "left"/"right" map)
//

class intrinsics_builder
{
private:
  bool valid_;
  double fx_, fy_, cx_, cy_;
  kwiver::vital::vector_4d dist_;

public:
  intrinsics_builder(): valid_(false) {}
  intrinsics_builder( double fx,                // focal point x
                      double fy,                // focal point y
                      double cx,                // principal point x
                      double cy,                // principal point y
                      const kwiver::vital::vector_4d& dist ) // distance parameters
    : valid_(true), fx_(fx), fy_(fy), cx_(cx), cy_(cy), dist_(dist)
  {}

  kwiver::vital::camera_intrinsics_sptr make_intrinsics( void ) const
  {
    if (!valid_)
    {
      throw std::logic_error("trying to build a camera from uninitialized intrinsics");
    }

    double focal_length = 0.5*(fx_+fy_);
    double dx = 2*cx_, dy = 2*cy_;
    kwiver::vital::vector_2d principal_point(cx_, cy_);
    auto const aspect_ratio = 1.0, skew = 0.0;

    return std::make_shared<kwiver::vital::simple_camera_intrinsics>(
      focal_length,
      principal_point,
      aspect_ratio,
      skew,
      dist_,
      dx, dy
      );
  }
};

} // end anon namespace

namespace kwiver {
namespace vital {

static auto logger = get_logger( "vital.camera_rig_io" );

std::string
get_file_ext( path_t const & FN )
{
  std::string ext;
  auto const
    len = FN.length(),
    n = FN.rfind('.', len);
  if (n != std::string::npos)
  {
     ext = FN.substr(n);
  }
  return ext;
}

camera_rig_sptr
read_camera_rig( path_list_t const & cam_files )
{
  camera_rig_sptr rig ( new camera_rig() );
  for ( auto const & cf : cam_files )
  {
    try
    {
      rig->add( cf, read_krtd_file( cf ) );
    }
    catch ( const file_not_found_exception& )
    {
      LOG_ERROR(logger, "error: unable to find " << cf);
      continue;
    }
  }
  if ( rig->empty() )
  {
    VITAL_THROW( invalid_data,
                 "no cameras initialized from the given list of files" ) ;
  }
  return rig;
}

camera_rig_stereo_sptr
read_stereo_rig_json( path_t const& FN )
{
  std::ifstream is(FN);
  cereal::JSONInputArchive ar(is);
  camera_collection cams;
  std::map< std::string, intrinsics_builder > intrinsics_lr;
  std::string LEFT("left"), RIGHT("right");
  auto sides = {LEFT, RIGHT};

  for (const auto& name: sides)
  {
    double fx=1, fy=1;
    ar( cereal::make_nvp( "fx_" + name, fx) );
    ar( cereal::make_nvp( "fy_" + name, fy) );

    double cx=0, cy=0;
    ar ( cereal::make_nvp( "cx_" + name, cx) );
    ar ( cereal::make_nvp( "cy_" + name, cy) );

    vector_4d dist;
    ar( cereal::make_nvp( "k1_" + name, dist[0] ) );
    ar( cereal::make_nvp( "k2_" + name, dist[1] ) );
    ar( cereal::make_nvp( "p1_" + name, dist[2] ) );
    ar( cereal::make_nvp( "p2_" + name, dist[3] ) );

    intrinsics_lr[name] = intrinsics_builder( fx, fy, cx, cy, dist );
  }

  vector_3d center = { 0, 0, 0 };
  rotation_d rotation;
  cams[LEFT] = std::make_shared<simple_camera_perspective>(
    center, rotation, intrinsics_lr[LEFT].make_intrinsics()
  );

  std::vector<double> T, R;
  ar( CEREAL_NVP(T) );
  int const n=3;
  vector_3d tv;
  for (int i=0; i<n; ++i)
  {
    tv[i]=T[i];
  }

  ar( CEREAL_NVP(R) );
  Eigen::Matrix<double,3,3> rm;
  unsigned k=0;
  for (int i=0; i<n; ++i)
  {
    for (int j=0; j<n; ++j)
    {
      rm(i,j) = R[k++];
    }
  }
  rotation = rotation_d(rm);
  auto camp = std::make_shared<simple_camera_perspective>(
    center, rotation, intrinsics_lr[RIGHT].make_intrinsics()
  );
  camp->set_translation(tv);
  cams[RIGHT] = camp;

  return std::make_shared<camera_rig_stereo>(
      cams[LEFT], cams[RIGHT]
  );
}

camera_rig_stereo_sptr
read_stereo_rig_yaml( path_t const& FN )
{
  // TODO read
  return camera_rig_stereo_sptr();
}

camera_rig_stereo_sptr
read_stereo_rig( path_t const& FN )
{
  auto const & ext = get_file_ext(FN);
  if (ext == ".json")
  {
    return read_stereo_rig_json(FN);
  }
  else if ( ext == ".yml" || ext == ".yaml")
  {
    return read_stereo_rig_yaml(FN);
  }
  else
  {
    LOG_ERROR( logger, "unable to read stereo rig: unsupported extension "+ext );
  }
  return camera_rig_stereo_sptr();
}

void
write_camera_rig( camera_rig_sptr rig )
{
  if (rig == nullptr)
  {
    LOG_ERROR( logger,
     "unable to write: camera rig pointer is null" );
    return;
  }
  for (auto const & c: rig->cameras())
  {
    try
    {
      auto const & cam = dynamic_cast<camera_perspective const&>(*c.second);
      write_krtd_file(cam, c.first);
    }
    catch( std::exception const & e )
    {
      LOG_ERROR(logger, "unable to write " << c.first
          << ": " << e.what() );
    }
  }
}

void
write_stereo_rig_json( camera_rig_stereo_sptr rig, std::string const & FN )
{
  if ( rig == nullptr )
  {
    LOG_ERROR( logger, "unable to write stereo rig: pointer is null" );
    return;
  }
  std::ofstream of( FN );
  cereal::JSONOutputArchive::Options opt(
    32, cereal::JSONOutputArchive::Options::IndentChar::space, 2 );
  cereal::JSONOutputArchive ar( of, opt );
  std::vector< std::string > names = { "left", "right" };
  Eigen::Matrix<double,3,3> Rl;
  Eigen::Matrix<double,3,1> cl;
  for ( auto const & name : names )
  {
    try
    {
      auto const & cam =
        dynamic_cast<camera_perspective const&>( *rig->camera(name) );
      auto const & intr = *cam.intrinsics();
      auto const & f = intr.focal_length();
      auto const & c = intr.principal_point();
      auto const & d = intr.dist_coeffs();
      auto const & dlen = d.size();
      ar( cereal::make_nvp( "fx_" + name, f) );
      ar( cereal::make_nvp( "fy_" + name, f) );
      ar( cereal::make_nvp( "cx_" + name, c[0]) );
      ar( cereal::make_nvp( "cy_" + name, c[1]) );
      ar( cereal::make_nvp( "k1_" + name, dlen > 0 ? d[0] : 0.0 ) );
      ar( cereal::make_nvp( "k2_" + name, dlen > 1 ? d[1] : 0.0 ) );
      ar( cereal::make_nvp( "p1_" + name, dlen > 2 ? d[2] : 0.0 ) );
      ar( cereal::make_nvp( "p2_" + name, dlen > 3 ? d[3] : 0.0 ) );
      if ( name == "left" )
      {
        Rl = cam.rotation().matrix();
        cl = cam.center();
      }
      else if ( name == "right" )
      {
        // form translation & rotation w.r.t. left
        auto const & Rr = cam.rotation().matrix();
        auto const & rm = Rr * Rl.transpose();
        auto const & tr = cam.translation();
        auto const & tv = tr - Rr * cl;
        auto n = tv.size();
        std::vector<double> T(n);
        for (int i=0; i<n; ++i)
        {
          T[i] = tv[i];
        }
        ar( CEREAL_NVP(T) );
        std::vector<double> R;
        for (int i=0; i<3; ++i)
        {
          for (int j=0; j<3; ++j)
          {
            R.push_back( rm(i,j) );
          }
        }
        ar( CEREAL_NVP(R) );
      }
    }
    catch( std::exception const & e )
    {
      LOG_ERROR(logger, "unable to write " << name
          << ": " << e.what() );
    }
  }
}

void
write_stereo_rig_yaml( camera_rig_stereo_sptr rig, std::string const & FN )
{
  if ( rig == nullptr )
  {
    LOG_ERROR( logger, "unable to write stereo rig: pointer is null" );
    return;
  }
  // TODO write intrinsics and extrinsics using OpenCV FileStorage facility,
  // which likely requires declaring an abstract markup serialization class
  // e.g. in vital/io or vital/algo and its specialization in arrows/ocv
  // that wraps FileStorage functionality for YAML/JSON/XML serialization.
}

void
write_stereo_rig( camera_rig_stereo_sptr rig, std::string const & FN )
{
  auto const & ext = get_file_ext(FN);
  if (ext == ".json")
  {
    write_stereo_rig_json(rig, FN);
  }
  else if ( ext == ".yml" || ext == ".yaml")
  {
    write_stereo_rig_yaml(rig, FN);
  }
}

} // vital
} // kwiver
