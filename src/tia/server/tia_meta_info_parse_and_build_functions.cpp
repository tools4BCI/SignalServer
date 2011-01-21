#include "tia-private/server/tia_meta_info_parse_and_build_functions.h"

#include "extern/include/rapidxml/rapidxml.hpp"
#include "extern/include/rapidxml/rapidxml_print.hpp"

#include <sstream>

using std::string;

namespace tia
{

namespace XML_TAGS
{
    std::string const TIA_META_INFO = "tiaMetaInfo";
    std::string const SUBJECT = "subject";
    std::string const SUBJECT_ID = "id";
    std::string const SUBJECT_FIRSTNAME = "firstName";
    std::string const SUBJECT_SURNAME = "surname";

    std::string const SIGNAL = "signal";
    std::string const SIGNAL_TYPE = "type";
    std::string const SIGNAL_SAMPLINGRATE = "samplingRate";
    std::string const SIGNAL_BLOCKSIZE = "blockSize";

    std::string const CHANNEL = "channel";
    std::string const CHANNEL_NR = "nr";
    std::string const CHANNEL_LABEL = "label";
}

//-----------------------------------------------------------------------------
void addAttribute (rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node, std::string const& attribute_name, std::string const& attribute_value);
void addAttribute (rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node, std::string const& attribute_name, unsigned attribute_value);

//-----------------------------------------------------------------------------
tobiss::SSConfig parserTiAMetaInfoFromXMLString (std::string const& tia_meta_info_xml_string)
{
    tobiss::SSConfig tia_meta_info;
    return tia_meta_info;
}

//-----------------------------------------------------------------------------
std::string buildTiAMetaInfoXMLString (tobiss::SSConfig const& tia_meta_info)
{
    rapidxml::xml_document<> xml_doc;

    char *tia_metainfo_node_name = xml_doc.allocate_string (XML_TAGS::TIA_META_INFO.c_str ());
    rapidxml::xml_node<>* tia_metainfo_node = xml_doc.allocate_node (rapidxml::node_element, tia_metainfo_node_name);

    // subject node
    char *subject_node_name = xml_doc.allocate_string (XML_TAGS::SUBJECT.c_str ());
    rapidxml::xml_node<>* subject_node = xml_doc.allocate_node (rapidxml::node_element, subject_node_name);
    addAttribute (&xml_doc, subject_node, XML_TAGS::SUBJECT_ID, tia_meta_info.subject_info.id());
    addAttribute (&xml_doc, subject_node, XML_TAGS::SUBJECT_FIRSTNAME, tia_meta_info.subject_info.firstName());
    addAttribute (&xml_doc, subject_node, XML_TAGS::SUBJECT_SURNAME, tia_meta_info.subject_info.surname());
    // TODO: add further attributes
    tia_metainfo_node->append_node (subject_node);


    // signals
    for (tobiss::SignalInfo::SignalMap::const_iterator signal_iter = tia_meta_info.signal_info.signals().begin ();
         signal_iter != tia_meta_info.signal_info.signals ().end (); ++signal_iter)
    {
        char *signal_node_name = xml_doc.allocate_string (XML_TAGS::SIGNAL.c_str ());
        rapidxml::xml_node<>* signal_node = xml_doc.allocate_node (rapidxml::node_element, signal_node_name);
        addAttribute (&xml_doc, signal_node, XML_TAGS::SIGNAL_TYPE, signal_iter->second.type ());
        addAttribute (&xml_doc, signal_node, XML_TAGS::SIGNAL_SAMPLINGRATE, signal_iter->second.samplingRate());
        addAttribute (&xml_doc, signal_node, XML_TAGS::SIGNAL_BLOCKSIZE, signal_iter->second.blockSize());

        for (unsigned channel_id = 0; channel_id < signal_iter->second.channels().size (); channel_id++)
        {
            char *channel_node_name = xml_doc.allocate_string (XML_TAGS::CHANNEL.c_str ());
            rapidxml::xml_node<>* channel_node = xml_doc.allocate_node (rapidxml::node_element, channel_node_name);
            addAttribute (&xml_doc, channel_node, XML_TAGS::CHANNEL_NR, channel_id + 1);
            addAttribute (&xml_doc, channel_node, XML_TAGS::CHANNEL_LABEL, signal_iter->second.channels ()[channel_id].id());
            signal_node->append_node (channel_node);
        }

        tia_metainfo_node->append_node (signal_node);
    }


    xml_doc.append_node (tia_metainfo_node);

    string xml_string;
    rapidxml::print (std::back_inserter (xml_string), xml_doc, 0);
    return xml_string;
}

//-----------------------------------------------------------------------------
void addAttribute (rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node, std::string const& attribute_name, std::string const& attribute_value)
{
    if (attribute_value.size())
    {
        char *attribute_name_rx = doc->allocate_string (attribute_name.c_str ());
        char *attribute_value_rx = doc->allocate_string (attribute_value.c_str());

        rapidxml::xml_attribute<>* attribute = doc->allocate_attribute (attribute_name_rx, attribute_value_rx, attribute_name.size (), attribute_value.size());
        node->append_attribute (attribute);
    }
}

//-----------------------------------------------------------------------------
void addAttribute (rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node, std::string const& attribute_name, unsigned attribute_value)
{
    std::ostringstream oss;
    oss << std::dec << attribute_value;
    addAttribute (doc, node, attribute_name, oss.str ());
}


}
