/*ckwg +5
 * Copyright 2014-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_ORACLE_INSTANCES_H
#define INCL_TRACK_ORACLE_INSTANCES_H

#include <track_oracle/track_oracle_core_impl.txx>
#include <track_oracle/track_oracle_core.txx>

#define TRACK_ORACLE_INSTANCES(T) \
  template kwiver::track_oracle::field_handle_type kwiver::track_oracle::track_oracle_core_impl::unlocked_create_element<T>( const kwiver::track_oracle::element_descriptor& e ); \
  template kwiver::track_oracle::field_handle_type kwiver::track_oracle::track_oracle_core::create_element<T>( const kwiver::track_oracle::element_descriptor& e ); \
  template T& kwiver::track_oracle::track_oracle_core_impl::unlocked_get_field<T>( kwiver::track_oracle::oracle_entry_handle_type track, kwiver::track_oracle::field_handle_type field ); \
  template T& kwiver::track_oracle::track_oracle_core::get_field<T>( kwiver::track_oracle::oracle_entry_handle_type track, kwiver::track_oracle::field_handle_type field ); \
  template std::pair< bool, T > kwiver::track_oracle::track_oracle_core::get<T>( const kwiver::track_oracle::oracle_entry_handle_type& track, const kwiver::track_oracle::field_handle_type& field ); \
  template std::pair< bool, T > kwiver::track_oracle::track_oracle_core_impl::get<T>( kwiver::track_oracle::oracle_entry_handle_type track, kwiver::track_oracle::field_handle_type field ); \
  template kwiver::track_oracle::oracle_entry_handle_type kwiver::track_oracle::track_oracle_core::lookup<T>( kwiver::track_oracle::field_handle_type field, const T& val, kwiver::track_oracle::domain_handle_type domain ); \
  template void kwiver::track_oracle::track_oracle_core::remove_field<T>( kwiver::track_oracle::oracle_entry_handle_type row, kwiver::track_oracle::field_handle_type field );\
  template std::pair< std::map<kwiver::track_oracle::oracle_entry_handle_type, T>*, T> kwiver::track_oracle::track_oracle_core_impl::lookup_table<T>( kwiver::track_oracle::field_handle_type field ); \
  template T& kwiver::track_oracle::track_oracle_core_impl::get_field<T>( kwiver::track_oracle::oracle_entry_handle_type track, kwiver::track_oracle::field_handle_type field ); \
  template void kwiver::track_oracle::track_oracle_core_impl::remove_field<T>( kwiver::track_oracle::oracle_entry_handle_type row, kwiver::track_oracle::field_handle_type field ); \
  template kwiver::track_oracle::oracle_entry_handle_type kwiver::track_oracle::track_oracle_core_impl::lookup<T>( kwiver::track_oracle::field_handle_type field, const T& val, kwiver::track_oracle::domain_handle_type domain );

#endif
