/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

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
      std::cout << " XMLParser: Default Constructor" << std::endl;
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
        std::cout << "XMLParser: Destructor" << std::endl;
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

    //    /**
    //    * @brief TODO
    //    */
    //    void parseFileReader();


    //    /**
    //    * @brief TODO
    //    */
    //    const std::map<std::string, std::string> getFileReaderMap()
    //    { return(file_reader_map_); }


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

    bool equalsYesOrNo(const std::string& s);

    /**
    * @brief Parse filename, path, and extension
    */
    void parseFileLocation(ticpp::Iterator<ticpp::Element> elem, std::map<std::string, std::string>& m);

//-----------------------------------------------
  private:
    ticpp::Document doc_;    ///< Holding the XML document
    ticpp::Iterator<ticpp::Element> subject_;    ///< Iterator to the \<subject\> tag
    ticpp::Iterator<ticpp::Element> server_settings_;  ///< Iterator to the \<server_setting\> tag

    ticpp::Iterator<ticpp::Element> file_reader_;  ///< Iterator to the \<file-reader\> tag

    std::map<std::string, std::string> subject_map_;
    std::map<std::string, std::string> server_settings_map_;

    /**
    * @brief A vector containing iterators to the respective \<hardware> tags.
    */
    std::vector< std::pair<std::string, ticpp::Iterator<ticpp::Element> > > hardware_;

};

} // Namespace tobiss

//---------------------------------------------------------------------------------------
#endif // XMLPARSER_H
