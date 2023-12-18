// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utilities for reading and writing csv files.

#ifndef KWIVER_ARROWS_CORE_CSV_IO_H_
#define KWIVER_ARROWS_CORE_CSV_IO_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <cstdint>

namespace kwiver {

namespace arrows {

namespace core {

namespace csv {

constexpr char DEFAULT_DELIM = ',';
constexpr char DEFAULT_QUOTE = '"';
constexpr char DEFAULT_QUOTE_ESC = '"';
constexpr char DEFAULT_COMMENT = '#';

/// Special token to begin a CSV field. Only valid when writing.
struct KWIVER_ALGO_CORE_EXPORT begf_t {};

KWIVER_ALGO_CORE_EXPORT
extern begf_t const begf;

/// Special token to end a CSV field. Only valid when writing.
struct KWIVER_ALGO_CORE_EXPORT endf_t {};

KWIVER_ALGO_CORE_EXPORT
extern endf_t const endf;

/// Special token to skip a CSV field.
struct KWIVER_ALGO_CORE_EXPORT skipf_t {};

KWIVER_ALGO_CORE_EXPORT
extern skipf_t const skipf;

/// Special token indicating the end of a CSV line.
struct KWIVER_ALGO_CORE_EXPORT endl_t {};

KWIVER_ALGO_CORE_EXPORT
extern endl_t const endl;

/// Special token indicating the beginning of a CSV comment.
struct KWIVER_ALGO_CORE_EXPORT comment_t {};

KWIVER_ALGO_CORE_EXPORT
extern comment_t const comment;

} // namespace csv

// ----------------------------------------------------------------------------
/// Base class for CSV reader and writer.
class KWIVER_ALGO_CORE_EXPORT csv_io_base
{
public:
  csv_io_base( char delim, char quote, char quote_esc, char comment );

  /// Returns the delimiting character in use.
  char delim() const;

  /// Returns the quote character in use.
  char quote() const;

  /// Returns the quote escape character in use.
  char quote_esc() const;

  /// Returns the comment character in use.
  char comment() const;

protected:
  char m_delim;
  char m_quote;
  char m_quote_esc;
  char m_comment;
};

// ----------------------------------------------------------------------------
/// A barebones yet flexible CSV writer.
///
/// This class abstracts away writing delimiters and properly quoting fields.
/// No assumptions are made about consistent row lengths or column data types.
/// A newline is automatically added at the end of the file if there is not one
/// already.
///
/// \todo Add a way to specify precision and fixed/scientific for float values.
/// \todo Add an unprocessed string write option, to facilitate writing
///       comments that are not meant to mimic data fields.
class KWIVER_ALGO_CORE_EXPORT csv_writer : public csv_io_base
{
public:
  /// Construct a csv writer.
  ///
  /// \param os Output stream to write to.
  /// \param delim Character that separates fields.
  /// \param quote Character that begins and ends a quoted field.
  /// \param quote_esc
  ///   Character that must be used to escape \p quote and itself when they
  ///   appear in quoted fields. May be the same as \p quote.
  /// \param comment
  ///   Character that, when appearing as the first character in a line,
  ///   designates that line as a comment.
  csv_writer(
    std::ostream& os,
    char delim = csv::DEFAULT_DELIM,
    char quote = csv::DEFAULT_QUOTE,
    char quote_esc = csv::DEFAULT_QUOTE_ESC,
    char comment = csv::DEFAULT_COMMENT );
  ~csv_writer();

  /// Write a field of type \c T to the file, or perform a special operation.
  ///
  /// Passing \c csv::begf begins a field, causing future writes to concatenate
  /// their \p value together instead of writing them as separate fields, until
  /// \c csv::endf is passed. Passing \c csv::skipf writes an empty field.
  /// Passing \c csv::endl ends the current line. Passing \c csv::comment
  /// starts a comment on the current line. Writing while in a comment behaves
  /// in the exact same way as writing outside a comment, including quoting and
  /// delimiting behaviors.
  ///
  /// \throws std::invalid_argument If the operation is not valid.
  template < class T >
  csv_writer& write( T const& value );

private:
  void commit();

  std::ostream& m_os;
  std::stringstream m_ss;
  bool m_in_field;
  bool m_first_field;
};

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, char const* value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, std::string const& value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, bool value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, char value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, uint8_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, uint16_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, uint32_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, uint64_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, int8_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, int16_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, int32_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, int64_t value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, float value );

// ----------------------------------------------------------------------------
/// Write \p value as a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, double value );

// ----------------------------------------------------------------------------
/// Begin a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, csv::begf_t const& );

// ----------------------------------------------------------------------------
/// End a CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, csv::endf_t const& );

// ----------------------------------------------------------------------------
/// Write an empty CSV field.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, csv::skipf_t const& );

// ----------------------------------------------------------------------------
/// End a line of CSV fields.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, csv::endl_t const& );

// ----------------------------------------------------------------------------
/// Begin a CSV comment line.
KWIVER_ALGO_CORE_EXPORT
csv_writer& operator<<( csv_writer& os, csv::comment_t const& );

// ----------------------------------------------------------------------------
/// A barebones yet flexible CSV reader.
///
/// This class abstracts away reading delimiters, properly unquoting fields,
/// skipping blank lines, etc.
///
/// \todo Add an unprocessed string read option, to facilitate reading comments
///       that are not meant to mimic data fields.

class KWIVER_ALGO_CORE_EXPORT csv_reader : public csv_io_base
{
public:

  /// Exception thrown when converting a field to a particular type fails.
  class parse_error : public std::runtime_error
  {
  public:
    parse_error( std::string const& string, std::type_info const& to_type );

    std::string const& string() const;
    std::type_info const& to_type() const;

  private:
    std::string m_string;
    std::type_info const& m_to_type;
  };

  /// Construct a csv reader.
  ///
  /// \param is Input stream to read from.
  /// \param delim Character that separates fields.
  /// \param quote Character that begins and ends a quoted field.
  /// \param quote_esc
  ///   Character that must be used to escape \p quote and itself when they
  ///   appear in quoted fields.
  /// \param comment
  ///   Character that, when appearing as the first character in a line,
  ///   designates that line as a comment.
  csv_reader(
    std::istream& is,
    char delim = csv::DEFAULT_DELIM,
    char quote = csv::DEFAULT_QUOTE,
    char quote_esc = csv::DEFAULT_QUOTE_ESC,
    char comment = csv::DEFAULT_COMMENT );
  ~csv_reader();

  /// Parse the next field and return it as type \p T.
  ///
  /// This works as expected for string and arithmetic types. Pass \c
  /// csv::comment_t to begin reading the contents of a comment as regular
  /// fields. Otherwise, comments will be silently skipped. Passing \c
  /// csv::endl_t has the same effect as \c next_line(). Passing \c
  /// csv::skipf_t has the same effect as \c skip_field(). If \p T is a \c
  /// std::optional, \c std::nullopt will be returned if the field is empty or
  /// if parsing the field fails. Therefore, \c parse_error will not be thrown
  /// if \p T is a \c std::optional.
  ///
  /// \throws parse_error If parsing from the current field to type \p T fails.
  /// The current field is skipped if this occurs. A string representation of
  /// the offending field can be recovered from the exception.
  /// \throws std::invalid_argument if the operation is invalid, e.g. the
  /// cursor is at EOF.
  template < class T >
  T read();

  /// Move the cursor from the end of this line to the beginning of the next
  /// line.
  ///
  /// \note Use \c skip_line() to move to the next line regardless of the
  /// current position.
  ///
  /// \throws std::invalid_argument If \c is_at_eol() is \c false or \c
  /// is_at_eof() is \c true.
  csv_reader& next_line();

  /// From the cursor from anywhere in this line to the beginning of the next
  /// line.
  ///
  /// \throws std::invalid_argument If \c is_at_eof() is \c true.
  csv_reader& skip_line();

  /// Proceed to the next field without reading the current one.
  ///
  /// \throws std::invalid_argument If \c is_at_field() is \c false.
  csv_reader& skip_field();

  /// Returns \c true if the cursor is at the end of the input stream.
  ///
  /// \note Any number of trailing newlines are ignored, though trailing
  /// comments will cause this to return \c false.
  bool is_at_eof();

  /// Returns \c true if the cursor is the end of a line.
  bool is_at_eol();

  /// Returns \c true if the cursor is at the beginning of a comment line.
  bool is_at_comment();

  /// Returns \c true if the cursor is at the beginning of a field.
  bool is_at_field();

private:
  void skip_blank_lines();

  std::istream& m_is;
  bool m_first_field;
  bool m_is_eol;
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
