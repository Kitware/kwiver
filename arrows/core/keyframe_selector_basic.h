// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining the keyframe_selector_basic algorithm

#ifndef ARROWS_PLUGINS_CORE_KEYFRAME_SELECTOR_BASIC_H_
#define ARROWS_PLUGINS_CORE_KEYFRAME_SELECTOR_BASIC_H_

#include <vital/vital_config.h>
#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/keyframe_selection.h>

namespace kwiver {
namespace arrows {
namespace core {

  class keyframe_selector_basic;
  typedef std::shared_ptr<keyframe_selector_basic> keyframe_selector_basic_sptr;

  /// A basic query formulator
  class KWIVER_ALGO_CORE_EXPORT keyframe_selector_basic
    : public vital::algo::keyframe_selection
  {
  public:
    PLUGIN_INFO( "basic",
                 "A simple implementation of keyframe selection based on statistics"
                 " of KLT tracks" )

    /// Default Constructor
    keyframe_selector_basic();

    virtual ~keyframe_selector_basic() {}

    /// Get this algorithm's \link vital::config_block configuration block \endlink
    ///
    /// This base virtual function implementation returns an empty configuration
    /// block whose name is set to \c this->type_name.
    ///
    /// \returns \c config_block containing the configuration for this algorithm
    ///         and any nested components.
    virtual vital::config_block_sptr get_configuration() const;

    /// Set this algorithm's properties via a config block
    ///
    /// \throws no_such_configuration_value_exception
    ///   Thrown if an expected configuration value is not present.
    /// \throws algorithm_configuration_exception
    ///   Thrown when the algorithm is given an invalid \c config_block or is'
    ///   otherwise unable to configure itself.
    ///
    /// \param config  The \c config_block instance containing the configuration
    ///               parameters for this algorithm
    virtual void set_configuration(vital::config_block_sptr config);

    /// Check that the algorithm's currently configuration is valid
    ///
    /// This checks solely within the provided \c config_block and not against
    /// the current state of the instance. This isn't static for inheritence
    /// reasons.
    ///
    /// \param config  The config block to check configuration of.
    ///
    /// \returns true if the configuration check passed and false if it didn't.
    virtual bool check_configuration(vital::config_block_sptr config) const;

    /// Select keyframes from a set of tracks.  Different implementations can
    /// select key-frames in different ways. For example, one method could only
    /// add key-frames for frames that are new.  Another could increase the
    /// density of key-frames near existing frames so dense processing can be done.
    ///
    /// \param [in] current_keyframes The current key-frame selection data.  Set
    ///            to null if no key-frame data is available or you want to
    ///            perform key-frame selection from scratch.
    /// \param [in] tracks The tracks over which to select key-frames
    /// \returns selected key-frame data structure.  Tracks is modified in place
    ///            so the returned pointer points to the same object as tracks.
    virtual kwiver::vital::track_set_sptr
      select(kwiver::vital::track_set_sptr tracks) const;

  protected:

    class priv;
    std::shared_ptr<priv> d_;
  };

} // end namespace core
} // end namespace arrows
} // end namespace kwiver

#endif // ARROWS_PLUGINS_CORE_KEYFRAME_SELECTOR_BASIC_H_
