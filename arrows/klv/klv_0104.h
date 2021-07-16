// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief This file contains the interface for klv 0104 video metadata.
 */

#ifndef KWIVER_ARROWS_KLV_KLV_0104_H_
#define KWIVER_ARROWS_KLV_KLV_0104_H_

#include <arrows/klv/klv_key.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/any.h>
#include <vital/vital_config.h>

#include <vector>
#include <string>
#include <cstddef>
#include <map>
#include <exception>
#include <cstdint>

namespace kwiver {
namespace arrows {
namespace klv {

// ----------------------------------------------------------------
/**
 * @brief klv 0104 metadata representation.
 *
 */
class KWIVER_ALGO_KLV_EXPORT klv_0104
{
public:
  static klv_uds_key key();

  /// Test to see if a 0104 key
  static bool is_key( klv_uds_key const& key );

  static klv_0104* instance();

  enum tag {  PLATFORM_DESIGNATION = 0,
              PLATFORM_DESIGNATION_ALT,
              STREAM_ID,
              ITEM_DESIGNATOR_ID,
              SECURITY_LOCAL_MD_SET,
              SECURITY_CLASSIFICATION,
              IMAGE_SOURCE_SENSOR,
              SENSOR_HORIZONTAL_FOV,
              SENSOR_VERTICAL_FOV,
              SENSOR_TYPE,
              IMAGE_COORDINATE_SYSTEM,
              TARGET_WIDTH,
              PLATFORM_HEADING_ANGLE,
              PLATFORM_PITCH_ANGLE,
              PLATFORM_ROLL_ANGLE,
              SENSOR_LATITUDE,
              SENSOR_LONGITUDE,
              SENSOR_ALTITUDE,
              FRAME_CENTER_LATITUDE,
              FRAME_CENTER_LONGITUDE,
              UPPER_LEFT_CORNER_LAT,
              UPPER_LEFT_CORNER_LON,
              UPPER_RIGHT_CORNER_LAT,
              UPPER_RIGHT_CORNER_LON,
              LOWER_RIGHT_CORNER_LAT,
              LOWER_RIGHT_CORNER_LON,
              LOWER_LEFT_CORNER_LAT,
              LOWER_LEFT_CORNER_LON,
              SLANT_RANGE,
              ANGLE_TO_NORTH,
              OBLIQUITY_ANGLE,
              START_DATE_TIME_UTC,
              EVENT_START_DATE_TIME_UTC,
              UNIX_TIMESTAMP,
              PLATFORM_TRUE_AIRSPEED,
              PLATFORM_INDICATED_AIRSPEED,
              PLATFORM_CALL_SIGN,
              FOV_NAME,
              WIND_DIRECTION,
              WIND_SPEED,
              PREDATOR_UAV_UMS,
              PREDATOR_UAV_UMS_V2,
              SENSOR_RELATIVE_ROLL_ANGLE,
              MISSION_ID,
              MISSION_START_TIME,
              PLATFORM_TAIL_NUMBER,
              MISSION_NUMBER,
              SENSOR_ROLL_ANGLE,
              // ---------------------- //
              UNKNOWN //must be last
  };

// ------------------------------------------------------------------
/// Class to store the tag name and a base class for different
/// types of values that can come from the klv
  class traits_base
  {
  public:
    virtual ~traits_base() { }

    virtual std::string to_string( kwiver::vital::any const& ) const = 0;
    virtual kwiver::vital::any convert( uint8_t const*, std::size_t ) = 0;
    virtual std::type_info const& typeid_for_tag( ) const = 0;
    virtual bool is_integral() const = 0;
    virtual bool is_floating_point() const = 0;

    std::string m_name;

  protected:
    traits_base( std::string const& name )
      : m_name( name )
    { }

    traits_base( std::string const& name, VITAL_UNUSED bool set )
      : m_name( name )
    { }

  };

  // ------------------------------------------------------------------
  /// Lookup the cooresponding tag for this key.
  /**
   * This method returns the tag that corresponds to the specified
   * key. If the key is not found, then the UNKNOWN tag is returned.
   *
   * @param key UDS key to look up.
   *
   * @return Tag corresponding to specified key or UNKNOWN tag if not found
   */
  tag get_tag( klv_uds_key const& key ) const;

  /// Extract the appropriate data type from raw bytes as a kwiver::vital::any
  kwiver::vital::any get_value( tag tg, uint8_t const* data, std::size_t length );

  /// Cast the tag to appropriate data type
  /**
   * Convert klv 0104 metadata tag to desired type. The only supported
   * types are std::string, double, uint64.
   *
   * @param tag Metadata tag enum
   * @param data
   *
   * @tparam T data type (string, double, uint64_t)
   *
   * @return Value from tag as desired type.
   */
  template < class T >
  T get_value( tag tag, kwiver::vital::any const& data ) const;

  /// Get the value of the data in the format of a string for any type
  /**
   * This method returns the string representation of the supplied
   * data. It is converted based on the correct type for the specified
   * tag.
   *
   * @param tg Tag to use to guide conversion.
   * @param data Data to be converted.
   *
   * @return String representation of the data.
   */
  std::string get_string( tag tg, kwiver::vital::any const& data ) const;

  /// Get the name of the tag as a string
  /**
   * This method returns the name of the supplied tag.
   *
   * @param tg Tag code to be converted to a string name.
   *
   * @return Name associated with the tag.
   */
  std::string get_tag_name( tag tg ) const;

  /// Get traits for tag
  /**
   *
   * @param tg Metadata tag value
   *
   * @return Reference to traits.
   */
  traits_base const& get_traits( tag tg ) const;

private:
  klv_0104();
  ~klv_0104();

  static klv_0104* s_instance;

  std::map< klv_uds_key, tag > m_key_to_tag;
  std::vector< traits_base* > m_traitsvec;
};

} } }  // end namespace

#endif
