// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utilities to make implementing text codecs easier.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_PRIV_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_PRIV_H_

// ----------------------------------------------------------------------------
// Sorry about all the macros, but these boilerplate snippets have 'return' or
// 'continue' keywords which can't be emulated from inside a utility function

// ----------------------------------------------------------------------------
#define ENCODE_HANDLE_ERROR( C )                                             \
        {                                                                          \
          result_code code;                                                        \
          std::tie( code, encoded_begin ) =                                        \
            m_encode_error_policy->handle( *this, C, encoded_begin, \
                                           encoded_end ); \
          if( code != DONE ) return { code, decoded_begin, encoded_begin };        \
        }

// ----------------------------------------------------------------------------
#define DECODE_HANDLE_ERROR                                               \
        {                                                                       \
          result_code code;                                                     \
          std::tie( code, decoded_begin ) =                                     \
            m_decode_error_policy->handle( *this, decoded_begin, decoded_end ); \
          if( code != DONE ) return { code, encoded_begin, decoded_begin };     \
        }

// ----------------------------------------------------------------------------
#define ENCODE_CHECK_CODE_POINT( C ) \
        if( !can_encode( C ) )             \
        {                                  \
          ENCODE_HANDLE_ERROR( C );        \
          ++decoded_begin;                 \
          continue;                        \
        }

// ----------------------------------------------------------------------------
#define ENCODE_CHECK_WRITE_SPACE( N )                      \
        if( encoded_begin + ( N ) > encoded_end )                \
        {                                                        \
          return { OUT_OF_SPACE, decoded_begin, encoded_begin }; \
        }

// ----------------------------------------------------------------------------
#define DECODE_CHECK_READ_SPACE( N )               \
        if( encoded_begin + ( N ) > encoded_end )        \
        {                                                \
          if( has_true_end ) { DECODE_HANDLE_ERROR; }    \
          return { DONE, encoded_begin, decoded_begin }; \
        }

// ----------------------------------------------------------------------------
#define DECODE_WRITE( C, N )                                 \
        if( can_encode( C ) )                                      \
        {                                                          \
          if( decoded_begin >= decoded_end )                       \
          {                                                        \
            return { OUT_OF_SPACE, encoded_begin, decoded_begin }; \
          }                                                        \
          *decoded_begin = C;                                      \
          ++decoded_begin;                                         \
        }                                                          \
        else                                                       \
        {                                                          \
          DECODE_HANDLE_ERROR;                                     \
        }                                                          \
        encoded_begin += ( N );

#endif
