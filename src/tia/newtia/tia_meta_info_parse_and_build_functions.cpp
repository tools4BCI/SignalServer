#include "tia-private/newtia/tia_meta_info_parse_and_build_functions.h"
#include "tia-private/newtia/tia_exceptions.h"

#include "tia-private/newtia/string_utils.h"
#include "extern/include/rapidxml/rapidxml.hpp"
#include "extern/include/rapidxml/rapidxml_print.hpp"

#include <sstream>
#include <map>
#include <set>

using std::string;
using std::map;
using std::set;

namespace tia
{

namespace XML_TAGS
{
    string const TIA_META_INFO = "tiaMetaInfo";
    std::string const SUBJECT = "subject";
    std::string const SUBJECT_ID = "id";
    std::string const SUBJECT_FIRSTNAME = "firstName";
    std::string const SUBJECT_SURNAME = "surname";

    std::string const MASTER_SIGNAL = "masterSignal";
    std::string const SIGNAL = "signal";
    std::string const SIGNAL_TYPE = "type";
    std::string const SIGNAL_SAMPLINGRATE = "samplingRate";
    std::string const SIGNAL_NUMCHANNELS = "numChannels";
    std::string const SIGNAL_BLOCKSIZE = "blockSize";
    string const SIGNAL_REQUIRED_ATTRIBUTES_ARRAY[] = {SIGNAL_TYPE, SIGNAL_SAMPLINGRATE, SIGNAL_BLOCKSIZE, SIGNAL_NUMCHANNELS};
    set<string> const SIGNAL_REQUIRED_ATTRIBUTES (SIGNAL_REQUIRED_ATTRIBUTES_ARRAY, SIGNAL_REQUIRED_ATTRIBUTES_ARRAY + 4);

    std::string const CHANNEL = "channel";
    std::string const CHANNEL_NR = "nr";
    std::string const CHANNEL_LABEL = "label";
    string const CHANNEL_REQUIRED_ATTRIBUTES_ARRAY[] = {CHANNEL_NR, CHANNEL_LABEL};
    set<string> const CHANNEL_REQUIRED_ATTRIBUTES (CHANNEL_REQUIRED_ATTRIBUTES_ARRAY, CHANNEL_REQUIRED_ATTRIBUTES_ARRAY + 2);
}

//-----------------------------------------------------------------------------
void addAttribute (rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node, std::string const& attribute_name, std::string const& attribute_value);
void addAttribute (rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node, std::string const& attribute_name, unsigned attribute_value);
map<string, string> getAttributes (rapidxml::xml_node<>* node);
map<string, string> getAttributes (rapidxml::xml_node<>* node, set<string> required_attributes);

//-----------------------------------------------------------------------------
tobiss::SSConfig parseTiAMetaInfoFromXMLString (std::string const& tia_meta_info_xml_string)
{
    tobiss::SSConfig tia_meta_info;
    rapidxml::xml_document<> xml_doc;
    try
    {
        xml_doc.parse<rapidxml::parse_non_destructive | rapidxml::parse_validate_closing_tags> ((char*)tia_meta_info_xml_string.c_str ());
    }
    catch (rapidxml::parse_error &error)
    {
        throw TiAException (string (error.what()));
    }

    rapidxml::xml_node<>* tia_metainfo_node = xml_doc.first_node ();
    if (tia_metainfo_node->next_sibling ())
        throw TiAException ("Parsing TiAMetaInfo String: Too many first level nodes.");

    // parse subject
    rapidxml::xml_node<>* subject_node = tia_metainfo_node->first_node (XML_TAGS::SUBJECT.c_str());
    if (subject_node)
    {
        std::map<string, string> attributes = getAttributes (subject_node);
        if (attributes.count (XML_TAGS::SUBJECT_ID))
            tia_meta_info.subject_info.setId (attributes.at(XML_TAGS::SUBJECT_ID));
        if (attributes.count (XML_TAGS::SUBJECT_FIRSTNAME))
            tia_meta_info.subject_info.setFirstName (attributes.at(XML_TAGS::SUBJECT_FIRSTNAME));
        if (attributes.count (XML_TAGS::SUBJECT_SURNAME))
            tia_meta_info.subject_info.setSurname (attributes.at(XML_TAGS::SUBJECT_SURNAME));
    }

    // parse master signal info
    rapidxml::xml_node<>* master_signal_node = 0;
    master_signal_node = tia_metainfo_node->first_node (XML_TAGS::MASTER_SIGNAL.c_str());
    if (master_signal_node)
    {
        std::map<string, string> attributes = getAttributes (master_signal_node);
        if (attributes.count (XML_TAGS::SIGNAL_SAMPLINGRATE))
            tia_meta_info.signal_info.setMasterSamplingRate (toUnsigned (attributes.at(XML_TAGS::SIGNAL_SAMPLINGRATE)));
        if (attributes.count (XML_TAGS::SIGNAL_BLOCKSIZE))
            tia_meta_info.signal_info.setMasterSamplingRate (toUnsigned (attributes.at(XML_TAGS::SIGNAL_BLOCKSIZE)));
    }


    // parse signals
    rapidxml::xml_node<>* signal_node = 0;
    signal_node = tia_metainfo_node->first_node (XML_TAGS::SIGNAL.c_str());
    tobiss::SignalInfo::SignalMap& signal_map = tia_meta_info.signal_info.signals ();
    while (signal_node)
    {
        tobiss::Signal signal;
        map<string, string> signal_attributes = getAttributes (signal_node, XML_TAGS::SIGNAL_REQUIRED_ATTRIBUTES);
        signal.setType (signal_attributes[XML_TAGS::SIGNAL_TYPE]);
        signal.setSamplingRate (toUnsigned (signal_attributes[XML_TAGS::SIGNAL_SAMPLINGRATE]));
        signal.setBlockSize (toUnsigned (signal_attributes[XML_TAGS::SIGNAL_BLOCKSIZE]));

        unsigned const num_channels = toUnsigned (signal_attributes[XML_TAGS::SIGNAL_NUMCHANNELS]);

        // parse channels
        std::vector<tobiss::Channel>& channel_vector = signal.channels();
        for (unsigned channel_nr = 0; channel_nr < num_channels; channel_nr++)
        {
            tobiss::Channel channel;
            channel.setId (toString (channel_nr));
            channel_vector.push_back (channel);
        }

        rapidxml::xml_node<>* channel_node = signal_node->first_node (XML_TAGS::CHANNEL.c_str());
        while (channel_node)
        {
            map<string, string> channel_attributes = getAttributes (channel_node, XML_TAGS::CHANNEL_REQUIRED_ATTRIBUTES);
            unsigned channel_nr = toUnsigned (channel_attributes[XML_TAGS::CHANNEL_NR]);
            if (channel_nr > num_channels)
                throw TiAException ("Parse TiAMetaInfo: nr-attribute of channel exceeds numChannels attribute of signal!");

            channel_vector[channel_nr - 1].setId (channel_attributes[XML_TAGS::CHANNEL_LABEL]);
            channel_node = channel_node->next_sibling (XML_TAGS::CHANNEL.c_str ());
        }

        signal_map[signal_attributes[XML_TAGS::SIGNAL_TYPE]] = signal;
        signal_node = signal_node->next_sibling (XML_TAGS::SIGNAL.c_str ());
    }

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


    // master signal data
    char *master_signal_node_name = xml_doc.allocate_string (XML_TAGS::MASTER_SIGNAL.c_str ());
    rapidxml::xml_node<>* master_signal_node = xml_doc.allocate_node (rapidxml::node_element, master_signal_node_name);
    addAttribute (&xml_doc, master_signal_node, XML_TAGS::SIGNAL_SAMPLINGRATE, tia_meta_info.signal_info.masterSamplingRate());
    addAttribute (&xml_doc, master_signal_node, XML_TAGS::SIGNAL_BLOCKSIZE, tia_meta_info.signal_info.masterBlockSize());
    tia_metainfo_node->append_node (master_signal_node);


    // signals
    for (tobiss::SignalInfo::SignalMap::const_iterator signal_iter = tia_meta_info.signal_info.signals().begin ();
         signal_iter != tia_meta_info.signal_info.signals ().end (); ++signal_iter)
    {
        char *signal_node_name = xml_doc.allocate_string (XML_TAGS::SIGNAL.c_str ());
        rapidxml::xml_node<>* signal_node = xml_doc.allocate_node (rapidxml::node_element, signal_node_name);
        addAttribute (&xml_doc, signal_node, XML_TAGS::SIGNAL_TYPE, signal_iter->second.type ());
        addAttribute (&xml_doc, signal_node, XML_TAGS::SIGNAL_SAMPLINGRATE, signal_iter->second.samplingRate());
        addAttribute (&xml_doc, signal_node, XML_TAGS::SIGNAL_BLOCKSIZE, signal_iter->second.blockSize());
        addAttribute (&xml_doc, signal_node, XML_TAGS::SIGNAL_NUMCHANNELS, signal_iter->second.channels().size ());

        for (unsigned channel_nr = 0; channel_nr < signal_iter->second.channels().size (); channel_nr++)
        {
            char *channel_node_name = xml_doc.allocate_string (XML_TAGS::CHANNEL.c_str ());
            rapidxml::xml_node<>* channel_node = xml_doc.allocate_node (rapidxml::node_element, channel_node_name);
            addAttribute (&xml_doc, channel_node, XML_TAGS::CHANNEL_NR, channel_nr + 1);
            addAttribute (&xml_doc, channel_node, XML_TAGS::CHANNEL_LABEL, signal_iter->second.channels ()[channel_nr].id());
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


//-----------------------------------------------------------------------------
map<string, string> getAttributes (rapidxml::xml_node<>* node)
{
    set<string> no_required_attributes;
    return getAttributes (node, no_required_attributes);
}

//-----------------------------------------------------------------------------
map<std::string, std::string> getAttributes (rapidxml::xml_node<>* node, set<string> required_attributes)
{
    std::map<string, string> attributes;
    if (!node)
        return attributes;
    rapidxml::xml_attribute<>* attribute = node->first_attribute ();
    while (attribute)
    {
        string key (attribute->name (), attribute->name_size ());
        required_attributes.erase (key);
        string value (attribute->value (), attribute->value_size ());
        attributes[key] = value;
        attribute = attribute->next_attribute ();
    }

    if (required_attributes.size ())
    {
        string missing_attributes;
        for (set<string>::const_iterator iter = required_attributes.begin (); iter != required_attributes.end (); )
        {
            missing_attributes += *iter;
            if (++iter != required_attributes.end ())
                missing_attributes += ", ";
        }
        throw TiAException (string ("Parse TiAMetaInfo: Required attributes missing: ") + missing_attributes);
    }
    return attributes;
}



}
