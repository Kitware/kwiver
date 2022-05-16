// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of PDAL point cloud writer

#include "pointcloud_io.h"
#include <vital/exceptions/base.h>
#include <vital/exceptions/io.h>
#include <vital/logger/logger.h>
#include <vital/types/geodesy.h>

#include <kwiversys/SystemTools.hxx>

#include <pdal/Dimension.hpp>
#include <pdal/Options.hpp>
#include <pdal/PointTable.hpp>
#include <pdal/PointView.hpp>
#include <pdal/StageFactory.hpp>

#include <io/BufferReader.hpp>
#include <io/BpfReader.hpp>
#include <io/LasReader.hpp>

namespace kwiver {
namespace arrows {
namespace pdal {
/// Load point cloud from file with PDAL

kwiver::vital::pointcloud_d
pointcloud_io
  ::load_( vital::path_t const& filename ) const
{
  ::pdal::Options options;
  options.add( "filename", filename );

  ::pdal::PointViewPtr point_view;
  ::pdal::PointTable table;

  auto ext = kwiversys::SystemTools::GetFilenameExtension(filename);

  if ( ext == ".las" )
  {
    ::pdal::LasReader las_reader;
    las_reader.setOptions(options);
    las_reader.prepare(table);

    ::pdal::PointViewSet point_view_set = las_reader.execute(table);
    point_view = *point_view_set.begin();
  }
  else if ( ext == ".bpf" )
  {
    ::pdal::BpfReader bpf_reader;
    bpf_reader.setOptions(options);
    bpf_reader.prepare(table);

    ::pdal::PointViewSet point_view_set = bpf_reader.execute(table);
    point_view = *point_view_set.begin();
  }
  else
  {
    throw vital::invalid_file( filename,
                               "file is not a las or bpf file.");
  }

  bool hasColor = point_view->hasDim(::pdal::Dimension::Id::Red) &&
                  point_view->hasDim(::pdal::Dimension::Id::Green) &&
                  point_view->hasDim(::pdal::Dimension::Id::Blue);

  std::vector< kwiver::vital::vector_3d > positions;
  std::vector< kwiver::vital::rgb_color > colors;

  for( ::pdal::PointId idx = 0; idx < point_view->size(); ++idx )
  {
    auto x = point_view->getFieldAs< double >( ::pdal::Dimension::Id::X, idx );
    auto y = point_view->getFieldAs< double >( ::pdal::Dimension::Id::Y, idx );
    auto z = point_view->getFieldAs< double >( ::pdal::Dimension::Id::Z, idx );

    positions.push_back( kwiver::vital::vector_3d( x, y, z ) );

    if ( hasColor )
    {
      uint8_t red = point_view->getFieldAs< uint8_t >(
        ::pdal::Dimension::Id::Red, idx );
      uint8_t green = point_view->getFieldAs< uint8_t >(
        ::pdal::Dimension::Id::Green, idx );
      uint8_t blue = point_view->getFieldAs< uint8_t >(
        ::pdal::Dimension::Id::Blue, idx );

      colors.push_back( kwiver::vital::rgb_color( red, green, blue ) );
    }
  }

  kwiver::vital::pointcloud_d retval( positions );

  if ( hasColor )
  {
    retval.set_color( colors );
  }

  return retval;
}

/// Write point cloud to a file with PDAL

void
pointcloud_io
  ::save_( vital::path_t const& filename,
           std::vector< vital::vector_3d > const& points,
           std::vector< vital::rgb_color > const& colors ) const
{
  namespace kv = kwiver::vital;
  kv::logger_handle_t logger( kv::get_logger( "arrows.pdal.pointcloud_io" ) );

  if( !colors.empty() && colors.size() != points.size() )
  {
    throw vital::invalid_value( "pdal::pointcloud_io::save_: number of colors "
                                "provided does not match the number of points" );
  }

  ::pdal::Options options;
  options.add( "filename", filename );
  options.add( "system_id", "KWIVER" );
  options.add( "offset_x", "auto" );
  options.add( "offset_y", "auto" );
  options.add( "offset_z", "auto" );

  ::pdal::PointTable table;
  table.layout()->registerDim( ::pdal::Dimension::Id::X );
  table.layout()->registerDim( ::pdal::Dimension::Id::Y );
  table.layout()->registerDim( ::pdal::Dimension::Id::Z );
  if( !colors.empty() )
  {
    table.layout()->registerDim( ::pdal::Dimension::Id::Red );
    table.layout()->registerDim( ::pdal::Dimension::Id::Green );
    table.layout()->registerDim( ::pdal::Dimension::Id::Blue );
  }

  int crs = m_lgcs.origin().crs();
  kv::vector_3d offset( 0.0, 0.0, 0.0 );

  ::pdal::PointViewPtr view;
  // handle special cases of non-geographic coordinates
  if( crs < 0 )
  {
    view = std::make_shared< ::pdal::PointView >( table );
    options.add( "scale_x", 1e-4 );
    options.add( "scale_y", 1e-4 );
    options.add( "scale_z", 1e-4 );
  }
  else
  {
    offset = m_lgcs.origin().location( vital::SRID::lat_lon_WGS84 );
    std::string srs_name = "EPSG:" + std::to_string( crs );

    ::pdal::SpatialReference srs;
    srs = ::pdal::SpatialReference( srs_name );
    view = std::make_shared< ::pdal::PointView >( table, srs );
  }

  for( unsigned int id = 0; id < points.size(); ++id )
  {
    kv::vector_3d pt = points[ id ] + offset;
    view->setField( ::pdal::Dimension::Id::X, id, pt.x() );
    view->setField( ::pdal::Dimension::Id::Y, id, pt.y() );
    view->setField( ::pdal::Dimension::Id::Z, id, pt.z() );
    if( !colors.empty() )
    {
      kv::rgb_color const& rgb = colors[ id ];
      view->setField( ::pdal::Dimension::Id::Red, id, rgb.r );
      view->setField( ::pdal::Dimension::Id::Green, id, rgb.g );
      view->setField( ::pdal::Dimension::Id::Blue, id, rgb.b );
    }
  }

  ::pdal::BufferReader reader;
  reader.addView( view );

  ::pdal::StageFactory factory;
  ::pdal::Stage* writer = factory.createStage( "writers.las" );

  writer->setInput( reader );
  writer->setOptions( options );
  writer->prepare( table );
  writer->execute( table );
}
} // end namespace pdal
} // end namespace arrows
} // end namespace kwiver
