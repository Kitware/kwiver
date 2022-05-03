// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test cxxopts.hpp

#include <test_gtest.h>

#include <vital/applets/cxxopts.hpp>

#include <string>

namespace cv = cxxopts::values;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(cxxopts, strings)
{
  std::string s = "test";
  EXPECT_EQ(cxxopts::stringLength(s), 4);
  EXPECT_EQ(cxxopts::stringAppend(s, "!"), "test!");
  EXPECT_EQ(cxxopts::stringAppend(s, 1, '?'), "test!?");
  EXPECT_EQ(cxxopts::stringAppend(s, s.rbegin(), s.rend()), "test!??!tset");
  EXPECT_FALSE(cxxopts::empty(s));
}

// ----------------------------------------------------------------------------
TEST(cxxopts, option_exception)
{
  cxxopts::OptionException oe("test");
  EXPECT_EQ((std::string) oe.what(), "test");

  cxxopts::OptionSpecException ose("test");
  EXPECT_EQ((std::string) ose.what(), "test");

  cxxopts::OptionParseException ope("test");
  EXPECT_EQ((std::string) ope.what(), "test");

  cxxopts::option_exists_error oee("test");
  EXPECT_EQ((std::string) oee.what(),
    "Option " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE + " already exists");

  cxxopts::invalid_option_format_error iofe("test");
  EXPECT_EQ((std::string) iofe.what(),
    "Invalid option format " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE);

  cxxopts::option_syntax_exception osye("test");
  EXPECT_EQ((std::string) osye.what(),
    "Argument " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE +
    " starts with a - but has incorrect syntax");

  cxxopts::option_not_exists_exception onee("test");
  EXPECT_EQ((std::string) onee.what(),
    "Option " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE + " does not exist");

  cxxopts::missing_argument_exception mae("test");
  EXPECT_EQ((std::string) mae.what(),
    "Option " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE +
    " is missing an argument");

  cxxopts::option_requires_argument_exception orae("test");
  EXPECT_EQ((std::string) orae.what(),
    "Option " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE +
    " requires an argument");

  cxxopts::option_not_has_argument_exception onhae("test", "arg");
  EXPECT_EQ((std::string) onhae.what(),
    "Option " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE +
    " does not take an argument, but argument " +
    cxxopts::LQUOTE + "arg" + cxxopts::RQUOTE + " given");

  cxxopts::option_not_present_exception onpe("test");
  EXPECT_EQ((std::string) onpe.what(),
    "Option " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE + " not present");

  cxxopts::argument_incorrect_type aite("test");
  EXPECT_EQ((std::string) aite.what(),
    "Argument " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE +
    " failed to parse");

  cxxopts::option_required_exception ore("test");
  EXPECT_EQ((std::string) ore.what(),
    "Option " + cxxopts::LQUOTE + "test" + cxxopts::RQUOTE +
    " is required but not present");
}

// ----------------------------------------------------------------------------
void signed_range_check(bool negative)
{
  cv::detail::check_signed_range<int16_t, int>(negative, (1 << 30), "");
}

TEST(cxxopts, signed_check)
{
  // Requires function calls because multiple template types breaks the macro
  EXPECT_THROW(signed_range_check(false), cxxopts::argument_incorrect_type);
  EXPECT_THROW(signed_range_check(true), cxxopts::argument_incorrect_type);
}

// ----------------------------------------------------------------------------
TEST(cxxopts, integer_parser)
{
  int x = -1;
  EXPECT_THROW(cv::integer_parser<int>("test", x), cxxopts::argument_incorrect_type);
  EXPECT_EQ(x, -1);

  cv::integer_parser<int>("0", x);
  EXPECT_EQ(x, 0);

  cv::integer_parser<int>("10", x);
  EXPECT_EQ(x, 10);

  cv::integer_parser<int>("0xfA", x);
  EXPECT_EQ(x, 0xFA);

  int8_t y = -1;
  EXPECT_THROW(cv::integer_parser<int8_t>("0xfff", y), cxxopts::argument_incorrect_type);
  EXPECT_EQ(y, -1);

  cv::integer_parser<int>("-3", x);
  EXPECT_EQ(x, -3);
}

// ----------------------------------------------------------------------------
TEST(cxxopts, stringstream_parser)
{
  int x = -1;
  cv::stringstream_parser<int>("1", x);
  EXPECT_EQ(x, 1);

  EXPECT_THROW(cv::stringstream_parser<int>("", x), cxxopts::argument_incorrect_type);
  EXPECT_EQ(x, 1);
}

// ----------------------------------------------------------------------------
TEST(cxxopts, parse_value)
{
  bool b = false;
  cv::parse_value("t", b);
  EXPECT_TRUE(b);
  cv::parse_value("f", b);
  EXPECT_FALSE(b);

  cv::parse_value("T", b);
  EXPECT_TRUE(b);
  cv::parse_value("F", b);
  EXPECT_FALSE(b);

  cv::parse_value("true", b);
  EXPECT_TRUE(b);
  cv::parse_value("false", b);
  EXPECT_FALSE(b);

  cv::parse_value("True", b);
  EXPECT_TRUE(b);
  cv::parse_value("False", b);
  EXPECT_FALSE(b);

  EXPECT_THROW(cv::parse_value("!", b), cxxopts::argument_incorrect_type);
}

// ----------------------------------------------------------------------------
TEST(cxxopts, options_help_one_group)
{
  cxxopts::Options options("test", "help");
  options.parse_positional("positional");
  options.positional_help("pos");

  EXPECT_EQ(options.help({""}), "help\nUsage:\n  test [OPTION...] pos\n\n");
  EXPECT_EQ(options.help(), "help\nUsage:\n  test [OPTION...] pos\n\n");
}
