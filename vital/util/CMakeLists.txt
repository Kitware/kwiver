###
# Build vital/util library
#
# A hard requirement for the vital_util library is that it not depend
# on vital library.
#

# ==================================================================
###
# These headers are installed
#

set( public_headers
  any_converter.h
  bounded_buffer.h
  compare.h
  cpu_timer.h
  data_stream_reader.h
  demangle.h
  enumerate_matrix.h
  file_md5.h
  get_paths.h
  hex_dump.h
  interval.h
  interval_map.h
  simple_stats.h
  singleton.h
  source_location.h
  string_editor.h
  string_encoding.h
  string.h
  text_codec_ascii.h
  text_codec_endian.h
  text_codec_error_policies.h
  text_codec.h
  text_codec_transcode.h
  text_codec_utf_16.h
  text_codec_utf_8.h
  thread_pool.h
  timer.h
  token_expand_editor.h
  token_expander.h
  tokenize.h
  token_type_env.h
  token_type.h
  token_type_symtab.h
  token_type_sysenv.h
  transform_image.h
  visit.h
  wall_timer.h
  wrap_text_block.h
  )

# ----------------------
set( private_headers
  text_codec_priv.h
  thread_pool_builtin_backend.h
  thread_pool_gcd_backend.h
  thread_pool_sync_backend.h
  )

# ----------------------
set( sources
  demangle.cxx
  get_paths.cxx
  source_location.cxx

  data_stream_reader.cxx
  file_md5.cxx
  hex_dump.cxx
  string.cxx
  string_editor.cxx
  string_encoding.cxx
  text_codec_ascii.cxx
  text_codec.cxx
  text_codec_error_policies.cxx
  text_codec_transcode.cxx
  text_codec_utf_16.cxx
  text_codec_utf_8.cxx
  thread_pool.cxx
  token_expand_editor.cxx
  token_expander.cxx
  token_type.cxx
  token_type_env.cxx
  token_type_symtab.cxx
  token_type_sysenv.cxx
  wrap_text_block.cxx
  )

kwiver_install_headers(
  ${public_headers}
  SUBDIR   vital/util
  )

# install generated header
kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/vital_util_export.h
  SUBDIR   vital/util
  NOPATH
  )

kwiver_add_library( vital_util
  ${public_headers}
  ${private_headers}
  ${sources}
  ${CMAKE_CURRENT_BINARY_DIR}/vital_util_export.h
  )

target_link_libraries( vital_util
  PRIVATE         kwiversys
                  vital_logger
  )

###
# Add tests if enabled
if (KWIVER_ENABLE_TESTS)
  add_subdirectory( tests )
endif()
