#include <assert.h>
#include <libxml/xpath.h>
#include <vital/logger/logger.h>
#include <vital/algo/algorithm.h>

namespace kwiver {
namespace arrows {
namespace mie4nitf {

      vital::logger_handle_t logger = vital::get_logger("mie4nitf_xml_helpers");

      xmlXPathObjectPtr get_node_set_from_context(xmlChar *xpath,
          xmlXPathContextPtr context)
      {
        xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);

        if (result == NULL)
        {
          xmlXPathFreeObject(result);
          LOG_ERROR(logger,
              "Error " << result << " in xmlXPathEvalExpression");
          return NULL;
        }

        if (xmlXPathNodeSetIsEmpty(result->nodesetval))
        {
          xmlXPathFreeObject(result);
          LOG_ERROR(logger, "Error no nodes found using XPath");
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
          LOG_ERROR(logger, "Error " << context << " in xmlXPathNewContext");
          return NULL;
        }
        return context;
      }

      xmlChar *const_char_to_xml_char(const char *p)
      {
        return (reinterpret_cast<xmlChar *>(const_cast<char *>(p)));
      }

      std::string xml_char_to_string(xmlChar *p)
      {
        char *c = reinterpret_cast<char *>(p);
        std::string s(c);
        return s;
      }

      xmlChar *get_attribute_value(const char *attr, xmlXPathContextPtr context)
      {
        std::string str = "./field[@name='" + std::string(attr) + "']";
        xmlChar *xml_str = const_char_to_xml_char(str.c_str());
        xmlXPathObjectPtr xpath = get_node_set_from_context(xml_str, context);
        assert(xpath->nodesetval->nodeNr == 1);
        xmlChar *prop = xmlGetProp(xpath->nodesetval->nodeTab[0],
            const_char_to_xml_char("value"));
        if (prop == NULL)
        {
          xmlFree(prop);
          VITAL_THROW(vital::metadata_exception,
              "Error " + xml_char_to_string(prop) + " in xmlGetProp");
        }
        xmlXPathFreeObject(xpath);
        return prop;
      }


    }  // namespace mie4nitf
  }  // namespace arrows
}  // namespace kwiver
