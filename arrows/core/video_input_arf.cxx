// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

// Implementation was heavily taken from
// https://www.mathworks.com/matlabcentral/fileexchange/78652-arf_read_write

#include "video_input_arf.h"

#include <vital/types/image.h>
#include <vital/types/image_container.h>
#include <vital/types/metadata_traits.h>
#include <vital/types/timestamp.h>

#include <vital/exceptions.h>
#include <vital/vital_types.h>

#include <fstream>
#include <iostream>

const uint32_t GAP_SIZE = 40;
const uint32_t COLOR_MAP_SIZE = 256 * 3;

namespace kv = kwiver::vital;

namespace kwiver {
namespace arrows {
namespace core {

// ----------------------------------------------------------------------------
class video_input_arf::priv
{
public:
  constexpr static uint16_t endian_test = 0x1234;
  constexpr static bool convert_endian =
    (static_cast<uint8_t const&>(endian_test) == 0x34);

  // Metadata map
  bool have_metadata_map = false;
  vital::metadata_sptr metadata = nullptr;
  vital::metadata_map::map_metadata_t metadata_map;

  // Utilities for little/big endian conversion
  uint32_t byteswap(uint32_t);

  void byteswap_image_bytes(unsigned char* img_bytes);

  // Local state
  FILE* file = nullptr;
  bool frame_info = false;
  uint32_t version = 0;
  uint32_t offset = 0;
  uint32_t rows = 0;
  uint32_t cols = 0;
  uint32_t num_frames = 0;
  uint32_t frame_pad = 0;
  uint32_t bpp = 0;
  uint32_t img_size = 0;

  kv::image_pixel_traits px;
  kv::frame_id_t current_frame = 0;
  kv::image_container_sptr current_image;
};

// ----------------------------------------------------------------------------
video_input_arf
::video_input_arf()
  : d{ new video_input_arf::priv }
{
  attach_logger( "arrows.core.video_input_arf" );

  set_capability( vital::algo::video_input::HAS_EOV, true );
  set_capability( vital::algo::video_input::HAS_FRAME_NUMBERS, true );
  set_capability( vital::algo::video_input::HAS_FRAME_TIME, true );
  set_capability( vital::algo::video_input::HAS_METADATA, false );

  set_capability( vital::algo::video_input::HAS_FRAME_DATA, false );
  set_capability( vital::algo::video_input::HAS_ABSOLUTE_FRAME_TIME, false );
  set_capability( vital::algo::video_input::HAS_TIMEOUT, false );
  set_capability( vital::algo::video_input::IS_SEEKABLE, true );
}

// ----------------------------------------------------------------------------
video_input_arf
::~video_input_arf()
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
video_input_arf
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config =
    vital::algo::video_input::get_configuration();

  return config;
}

// ----------------------------------------------------------------------------
void
video_input_arf
::set_configuration( vital::config_block_sptr in_config )
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);
}

// ----------------------------------------------------------------------------
bool
video_input_arf
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
video_input_arf
::open( std::string filename)
{
  // Close the video in case already open
  this->close();

  d->file = fopen(filename.c_str(), "rb");
  if (d->file == nullptr)
  {
    VITAL_THROW(kv::invalid_file, filename, "Could not open file");
  }

  uint32_t header[32];
  fread(header, 4, 8, d->file);
  uint32_t magic = d->byteswap(header[0]);
  if (magic != 3149642413)
  {
    VITAL_THROW(kv::invalid_file, filename, "Is not an ARF file");
  }

  d->version = d->byteswap(header[1]);
  if (d->version != 2)
  {
    VITAL_THROW(kv::invalid_file, filename, "Unsupported ARF version");
  }

  d->rows = d->byteswap(header[2]);
  d->cols = d->byteswap(header[3]);
  uint32_t imgtype = d->byteswap(header[4]);
  d->num_frames = d->byteswap(header[5]);
  d->offset = d->byteswap(header[6]);
  uint32_t flags = d->byteswap(header[7]);

  bool arf_info = (flags & (2 ^ 0)) != 0;
  bool arf_colormap = (flags & (2 ^ 1)) != 0;
  bool arf_comment = (flags & (2 ^ 2)) != 0;
  bool arf_multiband = (flags & (2 ^ 3)) != 0;
  bool arf_framedata = (flags & (2 ^ 4)) != 0;
  bool arf_extra = (flags & ((2 ^ 5) - 1)) != 0;
  if (arf_extra)
    LOG_INFO(logger(),
      "Additional unsupported/unimplemented ARF flags exist, read may fail");

  if (flags == 1 && d->offset == 288)
  {
    LOG_INFO(logger(),
      "header looks incorrect, assuming ARF_COMMENT instead of ARF_INFO");
    arf_info = false;
    arf_comment = true;
  }

  if (arf_info)
  {
    // TODO support this! (need example)
    //fread(&image_source, 4, 1, d->file);
    //fread(&start_x, 4, 1, d->file);
    //fread(&start_y, 4, 1, d->file);
    //fread(&nud->avg, 4, 1, d->file);
    //fread(&capture_rate, 4, 1, d->file);
    //fread(&capture_time, 4, 6, d->file);
    //capture_loc = readString(32);
    //sensor_name = readString(32);
    //digitizer = readString(32);
    //fread(&sensor_hfov, 1, 1, d->file);
    //fread(&sensor_vfov, 1, 1, d->file);
    //fread(&samples_per_dwell, 4, 1, d->file);
  }

  unsigned char color_map[COLOR_MAP_SIZE];
  if (arf_colormap)
  {
    // Not sure what to do with this....
    fread(&color_map, 1, COLOR_MAP_SIZE, d->file);
  }

  if (arf_comment)
  {
    //comment = readString(256);
  }

  if (arf_multiband)
  {
    // TBD
  }

  if (arf_framedata)
  {
    // TODO
    uint32_t footer_flags;
    fread(&footer_flags, 4, 1, d->file);
    d->frame_info = (footer_flags & (2 ^ 0)) != 0;
    d->frame_pad = 2 * 4 + 2 * 4 + 6 * 4;//40 bytes
  }

  switch (imgtype)
  {
  case 0:
    d->bpp = 1;//uint8
    d->px.type = kv::image_pixel_traits::UNSIGNED;
    break;
  case 1:
  case 2:
  case 5:
    d->bpp = 2;// uint16
    d->px.type = kv::image_pixel_traits::UNSIGNED;
    break;
  case 3://
    d->bpp = 2;// int16
    d->px.type = kv::image_pixel_traits::SIGNED;
    break;
  case 6:
    d->bpp = 4;// uint32
    d->px.type = kv::image_pixel_traits::UNSIGNED;
    break;
  case 7:
    d->bpp = 4;// single
    d->px.type = kv::image_pixel_traits::FLOAT;
    break;
  case 8:
    d->bpp = 8;// double
    d->px.type = kv::image_pixel_traits::FLOAT;
    break;
  case 10:
    d->bpp = 3;// uint8
    d->px.type = kv::image_pixel_traits::UNSIGNED;
    break;
  default:
    close();
    VITAL_THROW(kv::invalid_file, filename,
      "Unsupported ARF image type (bits per pixel)");
  }
  d->px.num_bytes = d->bpp;

  if (d->offset == 0)
  {
    // TODO sum all this up to take a guess...
    //d->offset = 32 + 140 * arf_info + 768 * arf_colormap + 256 * arf_comment;
    LOG_INFO(logger(), "No offest found, assuming offset of " << d->offset);
  }

  if (d->num_frames == 0)
  {
    // TODO Fatal or not?
    VITAL_THROW(kv::invalid_file, filename, "ARF file has no frames");
  }

  d->img_size = (d->rows * d->cols * d->bpp);

  d->current_frame = 0;
}

// ----------------------------------------------------------------------------
void
video_input_arf
::close()
{
  if (d->file != nullptr)
    fclose(d->file);
  d->file = nullptr;

  d->img_size = 0;
  d->frame_info = false;
  d->version = 0;
  d->offset = 0;
  d->rows = 0;
  d->cols = 0;
  d->num_frames = 0;
  d->frame_pad = 0;
  d->bpp = 0;

  d->current_frame = 0;
  d->current_image = nullptr;
}

// ----------------------------------------------------------------------------
bool
video_input_arf
::end_of_video() const
{
  return  d->current_frame == d->num_frames;
}

// ----------------------------------------------------------------------------
bool
video_input_arf
::good() const
{
  return d->img_size>0 && ! this->end_of_video();
}

// ----------------------------------------------------------------------------
bool
video_input_arf
::seekable() const
{
  return true;
}

// ----------------------------------------------------------------------------
size_t
video_input_arf
::num_frames() const
{
  return d->num_frames;
}

// ----------------------------------------------------------------------------
bool
video_input_arf
::next_frame(kv::timestamp& ts,   // returns timestamp
              VITAL_UNUSED uint32_t     timeout ) // not supported
{
  // Reset current timestamp
  ts = kv::timestamp();

  // Check for at end of video
  if ( this->end_of_video() )
  {
    return false;
  }

  ++d->current_frame;
  d->current_image = nullptr;

  // Check for at end of video
  if ( this->end_of_video() )
  {
    return false;
  }

  // Return timestamp
  ts = this->frame_timestamp();

  return true;
}

// ----------------------------------------------------------------------------
bool
video_input_arf
::seek_frame(kv::timestamp& ts,   // returns timestamp
             kv::timestamp::frame_t current_frame,
             VITAL_UNUSED uint32_t  timeout )
{
  // Reset current timestamp
  ts = kv::timestamp();

  // Check if requested frame exists
  if (current_frame > d->num_frames || current_frame <= 0)
  {
    return false;
  }

  d->current_image = nullptr;
  d->current_frame = current_frame;

  // Return timestamp
  ts = this->frame_timestamp();

  return true;
}

// ----------------------------------------------------------------------------
kv::timestamp
video_input_arf
::frame_timestamp() const
{
  // Check for at end of video
  if ( this->end_of_video() )
  {
    return {};
  }

  kv::timestamp ts;

  ts.set_frame( d->current_frame);

  return ts;
}

// ----------------------------------------------------------------------------
kv::image_container_sptr
video_input_arf
::frame_image()
{
  if (!good())
    return nullptr;

  if (!d->current_image && this->good())
  {
    auto img_bytes = std::make_shared<kv::image_memory>(d->img_size);

    // TODO Questions if using the GAP_SIZE is correct or not..
    fseek(d->file, d->offset + (d->current_frame * (d->img_size + GAP_SIZE)),
      SEEK_SET);
    fread(img_bytes->data(), d->img_size, 1, d->file);
    if (d->convert_endian)
    {
      d->byteswap_image_bytes(
        reinterpret_cast<unsigned char*>(img_bytes->data()));
    }

    // TODO Test with multi channel images
    kv::image img(img_bytes, img_bytes->data(),
      d->cols, d->rows, 1, 1, d->cols, 1, d->px);

    auto m = std::make_shared<kv::metadata>();
    m->set_timestamp(this->frame_timestamp());
    d->current_image = std::make_shared< kv::simple_image_container >(img, m);
  }
  return  d->current_image;
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_arf
::frame_metadata()
{
  return {};
}

// ----------------------------------------------------------------------------
kv::metadata_map_sptr
video_input_arf
::metadata_map()
{
  return nullptr;
}

// ----------------------------------------------------------------------------
uint32_t
video_input_arf::priv
::byteswap(uint32_t value)
{
#ifdef WIN32
  return _byteswap_ulong(value);
#else
  return __builtin_bswap32(value);
#endif
}

// ----------------------------------------------------------------------------
void
video_input_arf::priv
::byteswap_image_bytes(unsigned char* img_bytes)
{
  if (bpp == 1)
    return; // Unless imgtype == 10?
  unsigned char tmp;
  if (bpp == 2)
  {
    for (uint32_t i = 0; i < img_size; i += bpp)
    {
      tmp = img_bytes[i];
      img_bytes[i] = img_bytes[i + 1];
      img_bytes[i + 1] = tmp;
    }
  }
  else if (bpp == 4)
  {
    for (uint32_t i = 0; i < img_size; i += bpp)
    {
      tmp = img_bytes[i];
      img_bytes[i] = img_bytes[i + 3];
      img_bytes[i + 3] = tmp;

      tmp = img_bytes[i + 1];
      img_bytes[i + 1] = img_bytes[i + 2];
      img_bytes[i + 2] = tmp;
    }
  }
  else if (bpp == 8)
  {
    for (uint32_t i = 0; i < img_size; i += bpp)
    {
      tmp = img_bytes[i];
      img_bytes[i] = img_bytes[i + 7];
      img_bytes[i + 7] = tmp;

      tmp = img_bytes[i + 1];
      img_bytes[i + 1] = img_bytes[i + 6];
      img_bytes[i + 6] = tmp;

      tmp = img_bytes[i + 2];
      img_bytes[i + 2] = img_bytes[i + 5];
      img_bytes[i + 5] = tmp;

      tmp = img_bytes[i + 3];
      img_bytes[i + 3] = img_bytes[i + 4];
      img_bytes[i + 4] = tmp;
    }
  }
}

} } } // end namespace
