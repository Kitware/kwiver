// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_VITAL_PLUGGABLE_H_
#define KWIVER_VITAL_PLUGGABLE_H_

#include <memory>


namespace kwiver::vital
{

class pluggable;
typedef std::shared_ptr<pluggable> pluggable_sptr;


/**
 * Base-class for pluggable classes.
 *
 * This base provides minimal structure and more acts as a means of
 * categorization so that factories have a basic type to handle.
 *
 * This interface intentionally does not utilize `config_block` in any
 * definitions so that we can have plugins in our configuration world
 * -- NOTE: This could be revised to have the config stuff that are plugins to
 *          not live in the config module if that is feasible (e.g. they're
 *          algorithms in a way).
 */
class pluggable
{
public:
  /// Expected static functions:

  /// Return a human-readable name for interface being defined.
  /// This is defined at the interface level and categorizes implementations
  /// registered for it.
  //static std::string type_name();

  /// Curry construction of this concrete class from an input config_block instance.
  // static pluggable_sptr from_config( config_block const& cb );

  /// Set into a config-block the default configuration for this concrete type.
  /// Result `cb` state may not be valid for construction, but should at least
  /// provide all the keys required.
  // static void get_default_config( config_block & cb );

  // TODO: Is this even really needed?
  /**
   * @brief Set into the given config_block the configuration of this instance
   *
   * This should provide keys into the `cb` that match the keys provided in the
   * default configuration and be accepted by `from_config`.
   */
//  virtual void get_config( config_block & cb ) const = 0;

  virtual ~pluggable() = default;
};



}

#endif //KWIVER_VITAL_PLUGGABLE_H_
