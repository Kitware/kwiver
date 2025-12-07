// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "track_apix_writer.h"
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <geographic/geo_coords.h>
#include <iostream>
#include <map>
#include <shapefil.h>
#include <track_oracle/track_apix/track_apix.h>
#include <track_oracle/track_oracle_core.h>
#include <vul/vul_sprintf.h>

using std::floor;
using std::string;

using kwiver::geographic::geo_coords;

namespace  // anon
{

void reformat_time(
  const string time_format_string,
  double t_in_s,
  long& utc_time,
  int& utc_time_ms,
  string& time_string );

class shp_record_type
{
private:
  SHPHandle fh;

public:
  explicit shp_record_type( const string& prefix );
  ~shp_record_type();
  void emit_point( double lat, double lon );
};

class dbf_record_type
{
private:
  DBFHandle fh;
  int latField, lonField, timeField, msField, timeStrField, MGRSField,
    frameNumField,
    interplatField;

public:
  explicit dbf_record_type( const string& prefix );
  ~dbf_record_type();
  void emit_record(
    int row, ::kwiver::track_oracle::frame_handle_type frame,
    const string& time_format_string );
};

shp_record_type
::shp_record_type( const string& prefix )
{
  this->fh = SHPCreate( prefix.c_str(), SHPT_POINT );
}

shp_record_type
::~shp_record_type()
{
  SHPClose( this->fh );
}

void
shp_record_type
::emit_point(
  double lat,
  double lon )
{
  SHPObject* obj = SHPCreateSimpleObject( SHPT_POINT, 1, &lon, &lat, NULL );
  SHPWriteObject( this->fh, /* new shape = */ -1, obj );
  SHPDestroyObject( obj );
}

dbf_record_type
::dbf_record_type( const string& prefix )
{
  this->fh = DBFCreate( prefix.c_str() );

  this->latField = DBFAddField( this->fh, "Latitude", FTDouble, 12, 7 );
  this->lonField = DBFAddField( this->fh, "Longitude", FTDouble, 12, 7 );
  this->timeField = DBFAddField( this->fh, "DataUTCTim", FTInteger, 10, 0 );
  this->msField = DBFAddField( this->fh, "DataTimeMS", FTInteger, 4, 0 );
  this->timeStrField = DBFAddField( this->fh, "TimeString", FTString, 30, 0 );
  this->MGRSField = DBFAddField( this->fh, "MGRS", FTString, 24, 0 );
  this->frameNumField = DBFAddField( this->fh, "FrameNum", FTInteger, 7, 0 );
  this->interplatField = DBFAddField( this->fh, "Intrplat", FTInteger, 2, 0 );
}

dbf_record_type
::~dbf_record_type()
{
  DBFClose( this->fh );
}

void
dbf_record_type
::emit_record(
  int row,
  ::kwiver::track_oracle::frame_handle_type frame,
  const string& time_format_string )
{
  static ::kwiver::track_oracle::track_apix_type trk;

  geo_coords ll2mgrs( trk[ frame ].lat(), trk[ frame ].lon() );

  long utc_time_secs;
  int utc_time_ms;
  string time_string;
  reformat_time(
    time_format_string,
    trk[ frame ].utc_timestamp().get_time_seconds(),
    utc_time_secs,
    utc_time_ms,
    time_string );

  DBFWriteDoubleAttribute( this->fh, row, this->latField, trk[ frame ].lat() );
  DBFWriteDoubleAttribute( this->fh, row, this->lonField, trk[ frame ].lon() );
  DBFWriteIntegerAttribute( this->fh, row, this->timeField, utc_time_secs );
  DBFWriteIntegerAttribute( this->fh, row, this->msField, utc_time_ms );
  DBFWriteStringAttribute(
    this->fh, row, this->timeStrField,
    time_string.c_str() );
  DBFWriteStringAttribute(
    this->fh, row, this->MGRSField,
    ll2mgrs.mgrs_representation().c_str() );
  DBFWriteIntegerAttribute(
    this->fh, row, this->frameNumField,
    trk[ frame ].utc_timestamp().get_frame() );
  DBFWriteIntegerAttribute( this->fh, row, this->interplatField, 0 );
}

void
reformat_time(
  const string time_format_string,
  double t_in_s,
  long& utc_time,
  int& utc_time_ms,
  string& time_string )
{
  // Convert seconds since epoch to time components
  auto total_seconds = static_cast< long long >( floor( t_in_s ) );
  int ms = static_cast< int >( floor( ( t_in_s - total_seconds ) * 1000 ) );

  // Use std::chrono to handle the time point
  std::chrono::system_clock::time_point tp =
    std::chrono::system_clock::from_time_t(
      static_cast< std::time_t >( total_seconds ) );

  // Convert to time_t and then to tm structure
  std::time_t time_val = std::chrono::system_clock::to_time_t( tp );
  std::tm* tm_utc = std::gmtime( &time_val );

  // Extract time of day components
  int hours = tm_utc->tm_hour;
  int minutes = tm_utc->tm_min;
  int seconds = tm_utc->tm_sec;

  time_string = vul_sprintf(
    time_format_string.c_str(),
    static_cast< short >( tm_utc->tm_year + 1900 ),
    static_cast< short >( tm_utc->tm_mon + 1 ),
    static_cast< short >( tm_utc->tm_mday ),
    hours,
    minutes,
    seconds,
    ms );

  utc_time = total_seconds;
  utc_time_ms = ms;
}

} // anon namespace

namespace kwiver {

namespace track_oracle {

bool
track_apix_writer
::write(
  track_handle_type t,
  const string& filename,
  const string time_format_str )
{
  shp_record_type shp( filename );
  dbf_record_type dbf( filename );
  static track_apix_type trk;

  frame_handle_list_type frames = track_oracle_core::get_frames( t );
  for( unsigned i = 0; i < frames.size(); ++i )
  {
    shp.emit_point( trk[ frames[ i ] ].lat(), trk[ frames[ i ] ].lon() );
    dbf.emit_record( i, frames[ i ], time_format_str );
  }
  return true;
}

} // ...track_oracle

} // ...kwiver
