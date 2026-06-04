// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface to uid factory

#ifndef KWIVER_ARROWS_UUID_FACTORY_H
#define KWIVER_ARROWS_UUID_FACTORY_H

#include <arrows/uuid/kwiver_algo_uuid_export.h>
#include <vital/algo/uuid_factory.h>

namespace kwiver {

namespace arrows {

namespace uuid {

class KWIVER_ALGO_UUID_EXPORT uuid_factory_uuid
  : public vital::algo::uuid_factory
{
public:
  PLUGGABLE_IMPL(
    uuid_factory_uuid,
    "Global UUID generator using system library as source for UUID." );

  virtual ~uuid_factory_uuid() = default;

  bool check_configuration( vital::config_block_sptr config ) const override;

  // Main method to generate UUID's
  virtual kwiver::vital::uid create_uuid();

private:
  void initialize() override;
};

} // namespace uuid

} // namespace arrows

}     // end namespace

#endif // KWIVER_ARROWS_UUID_FACTORY_H
