// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface for detected_object_set_input_kpf

#ifndef KWIVER_ARROWS_KPF_DETECTED_OBJECT_SET_INPUT_KPF_H
#define KWIVER_ARROWS_KPF_DETECTED_OBJECT_SET_INPUT_KPF_H

#include <arrows/kpf/kwiver_algo_kpf_export.h>

#include <vital/algo/detected_object_set_input.h>

namespace kwiver {

namespace arrows {

namespace kpf {

class KWIVER_ALGO_KPF_EXPORT detected_object_set_input_kpf
  : public vital::algo::detected_object_set_input
{
public:
  PLUGGABLE_IMPL(
    detected_object_set_input_kpf,
    "Detected object set writer using kpf format."
  )

  virtual ~detected_object_set_input_kpf() = default;

  virtual bool check_configuration( vital::config_block_sptr config ) const;

  virtual bool read_set(
    kwiver::vital::detected_object_set_sptr& set,
    std::string& image_name );

private:
  virtual void new_stream();

  void initialize() override;

  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace kpf

} // namespace arrows

}     // end namespace

#endif // KWIVER_ARROWS_KPF_DETECTED_OBJECT_SET_INPUT_KPF_H
