/*ckwg +29
 * Copyright 2018-2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Implementation file for video input using GDAL.
 */

#include "arrows/gdal/mie4nitf/mie4nitf_video_input.h"

#include <vital/types/timestamp.h>
#include <vital/exceptions/io.h>
#include <vital/exceptions/video.h>
#include <vital/util/tokenize.h>
#include <arrows/gdal/image_io.h>
#include <arrows/gdal/image_container.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlstring.h>
#include <vul/vul_reg_exp.h>

#include <kwiversys/SystemTools.hxx>

#include <memory>
#include <vector>
#include <sstream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace kwiver {
namespace arrows {
namespace mie4nitf {


#define INT_DEFAULT -1
#define CURRENT_FRAME_NUMBER_INIT 1
#define CURRENT_FRAME_INDEX_INIT 0
// ------------------------------------------------------------------
// Private implementation class
class mie4nitf_video_input::priv
{
public:
  /// Constructor
  priv() :
    f_current_frame(nullptr),
    f_current_frame_metadata(nullptr),
    f_current_frame_number(INT_DEFAULT),
    f_current_frame_index(INT_DEFAULT),
    start_time(INT_DEFAULT),
    video_path(""),
    number_of_frames(INT_DEFAULT),
    gdal_mie4nitf_dataset_(nullptr),
    num_subdatasets(INT_DEFAULT)
  {}

  // Pointer to the current frame.
  vital::image_container_sptr f_current_frame;

  // The metadata per frame.
  std::shared_ptr<xml_metadata_per_frame> f_current_frame_metadata;

  // The current frame number.  The undefined value is `INT_DEFAULT`.
  // The initial value is `CURRENT_FRAME_NUMBER_INIT`.
  int f_current_frame_number;

  // The current frame index.  The undefined value is `INT_DEFAULT`.
  // The initial value is `CURRENT_FRAME_INDEX_INIT`.
  int f_current_frame_index;

  // Start time of the video,
  // to offset the pts when computing the frame number.
  // (in stream time base)
  int64_t start_time;

  // Name of video we opened
  std::string video_path;

  // For logging in priv methods
  vital::logger_handle_t logger;

  // The number of frames the video has.
  int number_of_frames;

  // The pointer *J2K* MIE4NITF GDAL dataset.
  GDALDataset *gdal_mie4nitf_dataset_;

  // Metadata per frame parsed from the XML returned by GDAL :
  // char **str = GDALGetMetadata(<dataset_name>, "xml:TRE");
  std::vector<xml_metadata_per_frame> xml_metadata;

  // The number of subdatasets the main dataset (`gdal_mie4nitf_data`) has.
  int num_subdatasets;

  // ==================================================================
  /*
  * @brief Whether the video was opened.
  *
  * @return \b true if video was opened.
  */
  bool is_opened()
  {
    return this->gdal_mie4nitf_dataset_ != nullptr;
  }

  // Check if the current_frame is not a `nullptr`
  bool is_valid()
  {
    return (this->f_current_frame != nullptr);
  }

  xmlXPathObjectPtr get_node_set_from_context(xmlChar *xpath,
    xmlXPathContextPtr context)
  {
    xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);

    if (result == NULL)
    {
      xmlXPathFreeObject(result);
      LOG_ERROR(this->logger, "Error " << result <<
        " in xmlXPathEvalExpression");
      return NULL;
    }

    if (xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
      xmlXPathFreeObject(result);
      LOG_ERROR(this->logger, "Error no nodes found using XPath");
      return NULL;
    }
      return result;
  }

  xmlXPathContextPtr get_new_context(xmlDocPtr doc)
  {
    xmlXPathContextPtr context;

    context = xmlXPathNewContext(doc);
    if (context == NULL)
    {
      xmlXPathFreeContext(context);
      LOG_ERROR(this->logger, "Error "<< context << " in xmlXPathNewContext");
      return NULL;
    }
    return context;
  }

  xmlChar *get_attribute_value(const char *attr, xmlXPathContextPtr context)
  {
    std::string str = "./field[@name='" + std::string(attr) + "']";
    xmlChar* xml_str = const_char_to_xml_char(str.c_str());
    xmlXPathObjectPtr xpath = get_node_set_from_context(xml_str, context);
    assert(xpath -> nodesetval -> nodeNr == 1);
    xmlChar* prop = xmlGetProp(xpath->nodesetval->nodeTab[0],
      const_char_to_xml_char("value"));
    if(prop == NULL)
    {
      xmlFree(prop);
      LOG_ERROR(this->logger, "Error "<< prop << " in xmlGetProp");
      return NULL;
    }
    xmlXPathFreeObject(xpath);
    return prop;
  }

  std::string xml_char_to_string(xmlChar *p)
  {
    char *c  = reinterpret_cast<char *>(p);
    std::string s(c);
    return s;
  }

  xmlChar* const_char_to_xml_char(const char *p)
  {
    return (reinterpret_cast<xmlChar*>(const_cast<char *>(p)));
  }

  xml_metadata_per_frame get_attributes_per_frame(xmlXPathContextPtr context)
  {
    xml_metadata_per_frame md;
    xmlChar* st = get_attribute_value("START_TIMESTAMP", context);
    xmlChar* en = get_attribute_value("END_TIMESTAMP", context);
    xmlChar* img = get_attribute_value("IMAGE_SEG_INDEX", context);

    md.start_timestamp = xml_char_to_string(st);
    md.end_timestamp = xml_char_to_string(en);
    std::string ind = xml_char_to_string(img);
    md.image_seg_index = std::stoi(ind);
    xmlFree(st);
    xmlFree(en);
    xmlFree(img);
    return md;
  }

  int xmlXPathSetContextNode(xmlNodePtr node, xmlXPathContextPtr ctx)
  {
    if ((node == NULL) || (ctx == NULL))
        return(-1);

     if (node->doc == ctx->doc) {
        ctx->node = node;
	return(0);
    }
    return(-1);
  }

  void populate_frame_times(xmlDoc * const doc)
  {
    xmlChar* temporal_block_xpath =
        reinterpret_cast<xmlChar*>(const_cast<char *>(
	"//tre[@name='MTIMFA']/repeated[@name='CAMERAS' and @number='1']/"
        "group[@index='0']/repeated[@name='TEMPORAL_BLOCKS']/group"));

    xmlXPathContextPtr xpath_context = get_new_context(doc);
    xmlXPathObjectPtr xpath_obj =
        get_node_set_from_context(temporal_block_xpath, xpath_context);
  
    // assert(xpath_obj -> nodesetval -> nodeNr == NUM_FRAMES_EXPECTED);
    for (int i = 0; i < xpath_obj -> nodesetval -> nodeNr; ++i)
    {
      xmlNode *node = xpath_obj->nodesetval->nodeTab[i];
      xmlXPathContextPtr xpath_context_local = get_new_context(doc);

      xmlXPathSetContextNode(node, xpath_context_local);
      xml_metadata_per_frame md = get_attributes_per_frame(xpath_context_local);
      xml_metadata.push_back(md);
      xmlXPathFreeContext(xpath_context_local);
    }
    xmlXPathFreeObject(xpath_obj);
    xmlXPathFreeContext(xpath_context);
  }

  void print_metadata_per_frame(int num)
  {
    for (int i = 0; i < num; ++i)
    {
      xml_metadata_per_frame md = xml_metadata.at(i);
      printf("%-30s : %d\n", "IMAGE_SEG_INDEX", md.image_seg_index);
      printf("%-30s : %s\n", "START_TIMESTAMP", md.start_timestamp.c_str());
      printf("%-30s : %s\n", "END_TIMESTAMP", md.end_timestamp.c_str());
      printf("%-30s : %s\n", "FILENAME", md.filename.c_str());
      printf("%-30s : %s\n", "DESC", md.description.c_str());
      printf("----------------------\n");
    }
  }

  void populate_xml_metadata()
  {
    std::string concat_strings;
    char **str = GDALGetMetadata(this->gdal_mie4nitf_dataset_, "xml:TRE");
    while(*str != NULL)
    {
      concat_strings = concat_strings + (*str);
      str++;
    }

    xmlDoc *doc = nullptr;
    char *str_char = const_cast<char *>(concat_strings.c_str());
    if ((doc = xmlReadDoc(reinterpret_cast<xmlChar*>(str_char),
      NULL, NULL, XML_PARSE_NOBLANKS)) == NULL)
    {
      LOG_ERROR(this->logger, "Error could not read input string");
      return;
    }
    populate_frame_times(doc);
    populate_subset_metadata();
    xmlFreeDoc(doc);
  }

  typedef std::pair<std::string, std::string> string_pair;

  string_pair parse_key_value(char* md)
  {
    char** s = CSLTokenizeString2(md, "=", CSLT_ALLOWEMPTYTOKENS
    | CSLT_HONOURSTRINGS | CSLT_PRESERVEQUOTES | CSLT_PRESERVEESCAPES);

    int i;
    for(i = 0; s != NULL && s[i] != NULL; ++i) {}

    if (i != 2)
    {
      LOG_ERROR(this->logger, 
        "Error could parse key-value pair in the metadata");
      return string_pair();
    }
    string_pair p = string_pair(std::string(s[0]), std::string(s[1]));
    CSLDestroy(s);
    return p;
  }

  void populate_subset_metadata()
  {
    char **metadata = GDALGetMetadata(this->gdal_mie4nitf_dataset_,
      "SUBDATASETS");

    int ind = 1;

    while(metadata != NULL && *metadata != NULL)
    {
      std::string key1 = "SUBDATASET_" + std::to_string(ind) + "_NAME";
      std::pair<std::string, std::string> p1 = parse_key_value(*metadata);
      // std::cout << "METADATA: " << *metadata << std::endl;
      assert(p1.first == key1);
      ++metadata;
      if (metadata == NULL)
      {
        LOG_ERROR(this->logger, "Error could parse metadata");
	return;
      }

      std::string key2 = "SUBDATASET_" + std::to_string(ind) + "_DESC";
      std::pair<std::string, std::string> p2 = parse_key_value(*metadata);
      assert(p2.first == key2);
      ++metadata;
      xml_metadata_per_frame &md = xml_metadata.at(ind - 1);
      md.filename = p1.second;
      md.description = p2.second;
      ++ind;
    }
    num_subdatasets = xml_metadata.size();
    number_of_frames = xml_metadata.size();
  }

  kwiver::vital::image_container_sptr open_frame(std::string subdataset_name)
  {
    kwiver::arrows::gdal::image_io img_io = kwiver::arrows::gdal::image_io();

    kwiver::vital::image_container_sptr frame = \
      img_io.load_subdataset(subdataset_name);

    if(frame == nullptr)
    {
      VITAL_THROW( vital::invalid_file, subdataset_name,
        "GDAL could not load file.");
      return nullptr;
    }
    return frame;
  }

  int convert_frame_number_to_index (int frame_number)
  {
    return frame_number - 1;
  }

  bool goto_frame_number(int frame_number)
  {
    int frame_ind = convert_frame_number_to_index(frame_number);
    if ( frame_number < 1 || frame_number > this->number_of_frames )
    {
      LOG_ERROR(this->logger, "Frame number out of expected range.");
      return false;
    }

    this->f_current_frame_metadata = std::make_shared<xml_metadata_per_frame>
      (this->xml_metadata.at(frame_ind));
    this->f_current_frame_number = frame_number;

    // TODO(m-chaturvedi): Check for caching with David.
    // Shouldn't run in case this is the first frame.
    // Close the previous frame.

    this->f_current_frame =
      open_frame(this->f_current_frame_metadata->filename);

    if (this->f_current_frame == nullptr)
    {
      throw kwiver::vital::file_not_found_exception(
        this->f_current_frame_metadata->filename,
        "File not found");
      return false;
    }

    return true;
  }

  bool priv_end_of_video()
  {
    if(this->f_current_frame_number == this->number_of_frames)
    {
      return true;
    }
    return false;
  }

  // ==================================================================
  /*
  * @brief Open the given video MIE4NITF video.
  *
  * @return \b true if video was opened.
  */
  bool open(std::string video_name)
  {
    GDALAllRegister();

    gdal_mie4nitf_dataset_ = static_cast<GDALDataset*>
      (GDALOpen(video_name.c_str(), GA_ReadOnly));

     if ( !gdal_mie4nitf_dataset_ )
     {
       VITAL_THROW( vital::invalid_file, video_name,
         "GDAL could not load file.");
       return false;
     }
     populate_xml_metadata();
     return true;
  }

}; // end of internal class.



// ==================================================================
mie4nitf_video_input
::mie4nitf_video_input()
  : d( new priv() )
{
  attach_logger( "mie4nitf_video_input" ); // get appropriate logger
  d->logger = this->logger();
  this->set_capability(vital::algo::video_input::HAS_EOV, true);
  this->set_capability(vital::algo::video_input::HAS_FRAME_NUMBERS, true);
  this->set_capability(vital::algo::video_input::HAS_FRAME_TIME, true);
  this->set_capability(vital::algo::video_input::HAS_FRAME_DATA, true);
  this->set_capability(vital::algo::video_input::HAS_ABSOLUTE_FRAME_TIME, true);
  this->set_capability(vital::algo::video_input::HAS_METADATA, false);

  this->set_capability(vital::algo::video_input::IS_SEEKABLE, true);
  this->set_capability(vital::algo::video_input::HAS_TIMEOUT, false);

}


mie4nitf_video_input
::~mie4nitf_video_input()
{
  return;
}


// ------------------------------------------------------------------
// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
mie4nitf_video_input
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config =
    vital::algo::video_input::get_configuration();

  return config;
}

// ------------------------------------------------------------------
// Set this algorithm's properties via a config block
void
mie4nitf_video_input
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated vital::config_block to ensure that assumed 
  // values are present.  An alternative is to check for key presence before
  // performing a get_value() call.

  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);
  return;
}

// ------------------------------------------------------------------
bool
mie4nitf_video_input
::check_configuration(vital::config_block_sptr config) const
{
  bool retcode(true); // assume success

  return retcode;
}

// ------------------------------------------------------------------
void
mie4nitf_video_input
::open( std::string video_name )
{
  this->close();

  d->video_path = video_name;

  {

    if (!kwiversys::SystemTools::FileExists(d->video_path))
    {
      // Throw exception
      throw kwiver::vital::file_not_found_exception(video_name,
        "File not found");
    }

    if (!d->open(video_name))
    {
      throw kwiver::vital::video_runtime_exception("Video file open failed "
        "for unknown reasons");
    }
    return;
  }
}


// ------------------------------------------------------------------
void
mie4nitf_video_input
::close()
{
    GDALClose(d->gdal_mie4nitf_dataset_);
    d->start_time = -1;
    d->video_path = "";
    d->number_of_frames = 0;
    d->gdal_mie4nitf_dataset_ = nullptr;
    d->f_current_frame.reset();
}

size_t
mie4nitf_video_input
::num_frames() const
{
    return d->number_of_frames;
}

// ------------------------------------------------------------------
bool
mie4nitf_video_input
::next_frame( kwiver::vital::timestamp& ts,
              uint32_t timeout )
{
  // If the file is not opened
  if (!d->is_opened())
  {
    throw vital::file_not_read_exception(d->video_path,
        "Video not open");
    return false;
  }

  // If it's the last frame then return `false`
  if(d->priv_end_of_video())
  {
    return false;
  }

  int next_frame_number = d->f_current_frame_number;

  if(next_frame_number == INT_DEFAULT)
  {
    next_frame_number = CURRENT_FRAME_NUMBER_INIT;
  }
  else
  {
    ++ next_frame_number;
  }

  if(!d->goto_frame_number(next_frame_number))
  {
    return false;
  }

  ts = this->frame_timestamp();
  return true;
}

// ------------------------------------------------------------------
bool mie4nitf_video_input::seek_frame(kwiver::vital::timestamp& ts,
  kwiver::vital::timestamp::frame_t frame_number,
  uint32_t timeout)
{
  // Quick return if the file isn't open.
  if (!d->is_opened())
  {
    throw vital::file_not_read_exception(d->video_path, "Video not open");
    return false;
  }
  if (!d->goto_frame_number(frame_number))
  {
    return false;
  }

  // Quick return if the file isn't open.
  if (timeout != 0)
  {
    LOG_WARN(this->logger(), "Timeout argument is not supported.");
  }

  ts = this->frame_timestamp();
  return true;
}


// ------------------------------------------------------------------
kwiver::vital::image_container_sptr
mie4nitf_video_input
::frame_image( )
{
  std::shared_ptr<kwiver::vital::image_container> ptr = d->f_current_frame;
  return ptr;
}

// ------------------------------------------------------------------
kwiver::vital::timestamp
mie4nitf_video_input
::frame_timestamp() const
{
  if (!this->good())
  {
    return {};
  }

  // `frame_id_t` and `time_usec_t` is `int_64_t` while writing this.
  // (`vital_types.h`)
  vital::time_usec_t t =
    std::stod(d->f_current_frame_metadata->start_timestamp);

  vital::frame_id_t f =
    static_cast<kwiver::vital::frame_id_t>(d->f_current_frame_number);

  kwiver::vital::timestamp ts(t, f);
  return ts;
}


// ------------------------------------------------------------------
kwiver::vital::metadata_vector
mie4nitf_video_input
::frame_metadata()
{
// TODO(m-chaturvedi): Implement this.
  LOG_INFO(this->logger(), "Metadata access isn't supported yet");
  return kwiver::vital::metadata_vector();
}


// ------------------------------------------------------------------
kwiver::vital::metadata_map_sptr
mie4nitf_video_input
::metadata_map()
{
// TODO(m-chaturvedi): Implement this.
  LOG_INFO(this->logger(), "Metadata access isn't supported yet");
  std::shared_ptr<kwiver::vital::simple_metadata_map> ptr;
  return ptr;
}


// ------------------------------------------------------------------
bool
mie4nitf_video_input
::end_of_video() const
{
  return d->priv_end_of_video();
}


// ------------------------------------------------------------------
bool
mie4nitf_video_input
::good() const
{
  return d->is_valid();
}


// ------------------------------------------------------------------
bool
mie4nitf_video_input
::seekable() const
{
  return true;
}

} } } // end namespaces
