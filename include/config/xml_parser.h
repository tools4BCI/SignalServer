
/**
* @file xml_parser.h
*
* @brief xml_parser contains a representation of a xml file and does only basic xml parsing.
*
* xml_parser was written to do basic parsing on the configuration file for the signalserver project.
*
**/

#ifndef XMLPARSER_H
#define XMLPARSER_H

// ticpp
#include "ticpp/ticpp.h"

// stl
#include <iostream>
#include <vector>
#include <string>
#include <map>

// boost
#include <boost/lexical_cast.hpp>

#include "definitions/constants.h"

namespace tobiss
{
//---------------------------------------------------------------------------------------

/**
* @class XMLParser
*
* @brief Class holding a xml document and doing basic xml parsing.
*/
class XMLParser
{
  public:
    /**
    * @brief Default constructor
    */
    XMLParser()
    {
      #ifdef DEBUG
        cout << "XMLParser: Default Constructor" << endl;
      #endif
    }
    /**
    * @brief Constructor
    * @param[in] xml_file XML file to be loaded and parsed.
    * @throw ticpp::Exception if no subject defined or mutliple server settings availabe!
    */
    XMLParser(const std::string xml_file);
    /**
    * @brief Default destructor
    */
    virtual ~XMLParser()
    {
      #ifdef DEBUG
        cout << "XMLParser: Destructor" << endl;
      #endif
    }

    /**
    * @brief Get an ticpp Element pointing to the \<subject\> node.
    * @return ticpp::Iterator<ticpp::Element>  Element pointing to \<subject\>
    */
    const ticpp::Iterator<ticpp::Element> getSubjectElement() const {  return(subject_);  }

    /**
    * @brief Get an ticpp Element pointing to the \<server_settings\> node.
    * @return ticpp::Iterator<ticpp::Element>  Element pointing to \<server_setting\>
    */
    const ticpp::Iterator<ticpp::Element> getServerSettingsElement() const { return(server_settings_); }

    /**
    * @brief Get the n-th ticpp Element pointing to an \<hardware\> node.
    * @param[in] n  Number of the respective hardware block in the xml config file.
    * @return ticpp::Iterator<ticpp::Element>  Element pointing to the respective \<hardware>
    */
    const ticpp::Iterator<ticpp::Element> getHardwareElement(const unsigned int n) const
      {  return(hardware_.at(n).second);  }

    /**
    * @brief Get the ticpp Element pointing to the respective \<hardware\> node.
    * @param[in] name  Name of the respective hardware.
    * @return ticpp::Iterator<ticpp::Element>  Element pointing to the respective \<hardware> (0 if not found)
    */
    const ticpp::Iterator<ticpp::Element> getHardwareElement(const std::string& s) const
      {
        for(unsigned int n = 0; n < hardware_.size(); n++)
          if(s == hardware_.at(n).first)
            return(hardware_[n].second);

        return(0);
        //     throw(ticpp::Exception("XMLParser::getHardwareElement -- Element "+s+" not found!"));
        // * @throws ticpp::exception if hardware element not found.
      }

    /**
    * @brief Get the name of the n-th \<hardware\> node.
    * @param[in] n  Number of the respective hardware block in the xml config file.
    * @return string  Name of the respective \<hardware>
    */
    const std::string getHardwareElementName(const unsigned int n) const {  return(hardware_.at(n).first);  }

    /**
    * @brief Get the mode of the n-th  \<hardware\> node.
    * @param[in] n  Number of the respective hardware block in the xml config file.
    * @return string  Type (master or slave) of the respective \<hardware>
    */
    const std::string getHardwareElementType(const unsigned int n) const
    {
      return(hardware_.at(n).second->FirstChildElement(cst_.hw_mode ,false)->GetText());
    }

    /**
    * @brief Get the nuber of hardware sections in the config file.
    * @return unsigned int  Number of \<hardware\> tags in the config file.
    */
    unsigned int getNrOfHardwareElements() const  {  return(hardware_.size());  }

    /**
    * @brief Parse the \<subject\> section in the config file into a map.
    * @return std::map<string,string> A map containing tags and related text in a map.
    * @throw ticpp::Exception
    */
    std::map<std::string,std::string> parseSubject();

    /**
    * @brief Parse the \<server_setting\> section in the config file into a map.
    * @return std::map<string,string> A map containing tags and related text in a map.
    * @throw ticpp::Exception
    */
    std::map<std::string,std::string> parseServerSettings();

    /**
    * @brief Parse the \<measurement_channels\> section in the config file -- used for common channel settings.
    * @param[in] elem   The ticpp-element to parse.
    * @param[out] nr_ch The number of channels defined in the ticpp-element.
    * @param[out] naming The channel-naming defined in the ticpp-element.
    * @param[out] type The channel-types defined in the ticpp-element.
    * @throw ticpp::Exception
    */
    void parseDeviceChannels(ticpp::Iterator<ticpp::Element>const &elem, boost::uint16_t& nr_ch,
                             std::string& naming, std::string& type);

    /**
    * @brief Parse the \<selection> section in the config file -- used for individual channel settings.
    * @param[in] elem   The ticpp-element to parse.
    * @param[out] ch The channel number defined in the ticpp-element.
    * @param[out] name The channel's name defined in the ticpp-element.
    * @param[out] type The channel's type defined in the ticpp-element.
    * @throw ticpp::Exception
    */
    void parseChannelSelection(ticpp::Iterator<ticpp::Element>const &elem, boost::uint16_t& ch,
                               std::string& name, std::string& type);

    /**
    * @brief Check if all mandatory tags in \<hardware\> are given.
    * @throw ticpp::Exception
    */
    void checkMandatoryHardwareTags(ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief TODO
    */
    void parseFileReader();

    /**
    * @brief TODO
    */
    bool usesDataFile()
    { return(external_data_file_); }

    /**
    * @brief TODO
    */
    const std::map<std::string, std::string> getFileReaderMap()
    { return(file_reader_map_); }


//-----------------------------------------------

  private:
    /**
    * @brief Check attributes in \<hardware>
    * @throw ticpp::Exception if device name not specified
    */
    void checkHardwareAttributes(std::map<std::string,std::string>& m);
    /**
    * @brief Check, if a string represents a valid number.
    * @return bool
    */
    bool isNumber(const std::string& s)
    {
      for (unsigned int i = 0; i < s.length(); i++)
        if (!isdigit(s[i]))
          return false;

      return true;
    }

    /**
    * @brief TODO
    */
    void parseFileLocation(ticpp::Iterator<ticpp::Element> elem, std::map<std::string, std::string>& m);

//-----------------------------------------------
  private:
    ticpp::Document doc_;    ///< Holding the XML document
    ticpp::Iterator<ticpp::Element> subject_;    ///< Iterator to the \<subject\> tag
    ticpp::Iterator<ticpp::Element> server_settings_;  ///< Iterator to the \<server_setting\> tag

    ticpp::Iterator<ticpp::Element> file_reader_;  ///< Iterator to the \<file-reader\> tag
    bool external_data_file_;

    std::map<std::string, std::string> subject_map_;
    std::map<std::string, std::string> server_settings_map_;
    std::map<std::string, std::string> file_reader_map_;

    /**
    * @brief A vector containing iterators to the respective \<hardware> tags.
    */
    std::vector< std::pair<std::string, ticpp::Iterator<ticpp::Element> > > hardware_;

    Constants cst_;  ///< A static object containing constants.
};

} // Namespace tobiss

//---------------------------------------------------------------------------------------
#endif // XMLPARSER_H
