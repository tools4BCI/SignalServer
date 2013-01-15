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
* @file xml_parser.cpp
**/

#include <config/xml_tags.h>
#include "config/xml_parser.h"
#include <boost/algorithm/string.hpp>

namespace tobiss
{

using std::vector;
using std::string;
using std::map;
using std::pair;
using std::cout;
using std::endl;

using boost::lexical_cast;
using boost::bad_lexical_cast;

using boost::uint16_t;
using boost::algorithm::to_lower_copy;

//---------------------------------------------------------------------------------------

XMLParser::XMLParser(string xml_file)
  : doc_(xml_file)
{
  #ifdef DEBUG
    cout << "XMLParser: Constructor" << endl;
  #endif

  doc_.LoadFile();
  ticpp::Iterator<ticpp::Element> config(doc_.FirstChildElement(xmltags::sigserver_config , true));

  //    Filereader part  --> TODO and move to other file
  //  file_reader_ = config->FirstChildElement(cst_.file_reader, false);

  //  if( file_reader_ != file_reader_.end() )
  //  {
  //    for(ticpp::Iterator<ticpp::Element> it(file_reader_) ; ++it != it.end(); )
  //      if(it->Value() == cst_.file_reader)
  //        throw(ticpp::Exception("Error -- Only one subject definition allowed!"));
  //
  //    external_data_file_ = 1;
  //
  //    parseFileReader();
  //  }


  server_settings_ = config->FirstChildElement(xmltags::server_settings, true);
  for(ticpp::Iterator<ticpp::Element> it(server_settings_); ++it != it.end(); )
    if(it->Value() == xmltags::server_settings)
      throw(ticpp::Exception("Error -- Multiple server settings found!"));


  subject_ = config->FirstChildElement(xmltags::subject, true);
  for(ticpp::Iterator<ticpp::Element> it(subject_); ++it != it.end(); )
    if(it->Value() == xmltags::subject)
      throw(ticpp::Exception("Error -- Only one subject definition allowed!"));


  ticpp::Iterator< ticpp::Attribute > attribute;
  for(ticpp::Iterator<ticpp::Element> it(config->FirstChildElement(xmltags::hardware, true));
      it != it.end(); it++)
      if(it->Value() == xmltags::hardware)
    {
      map<string, string> m;
      for(attribute = attribute.begin(it.Get()); attribute != attribute.end();
          attribute++)
        m.insert(pair<string, string>(attribute->Name(), attribute->Value()));
      checkHardwareAttributes(m);

      hardware_.push_back(make_pair(m.find(xmltags::hardware_name)->second,it));
    }
}

//---------------------------------------------------------------------------------------

bool XMLParser::equalsYesOrNo(const std::string& s)
{
  if(to_lower_copy(s) == "yes"  || s == "1" || s == "on")
    return(true);
  if(to_lower_copy(s) == "no" || s == "0" || s == "off")
    return(false);
  else
  {
    string e = s + " -- Value equals neiter \"yes, no, 0 or 1\"!";
    throw std::invalid_argument(e);
  }
}

//---------------------------------------------------------------------------------------

map<string,string> XMLParser::parseSubject()
{
  #ifdef DEBUG
    cout << "XMLParser: parseSubject" << endl;
  #endif

  map<string, string>& m = subject_map_;
  ticpp::Iterator<ticpp::Element> elem(subject_->FirstChildElement(xmltags::subject_id,true));
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(xmltags::subject_firstname, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(xmltags::subject_surname, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(xmltags::subject_birthday, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(xmltags::subject_sex, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  for(elem = subject_->FirstChildElement(true) ; elem != elem.end(); elem++)
  {
    string tmp(elem->Value());
    if(tmp == xmltags::subject_id || tmp == xmltags::subject_firstname ||
       tmp == xmltags::subject_surname || tmp == xmltags::subject_birthday || tmp == xmltags::subject_sex)
      continue;
    if(elem->GetText(false) != "")
      m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

    ticpp::Iterator< ticpp::Attribute > attribute;
    for(attribute = attribute.begin(elem.Get()); attribute != attribute.end();
        attribute++)
      m.insert(pair<string, string>(attribute->Name(), attribute->Value()));
  }
  return(subject_map_);
}

//---------------------------------------------------------------------------------------

map<string,string> XMLParser::parseServerSettings()
{
  #ifdef DEBUG
    cout << "XMLParser: parseServerSettings" << endl;
  #endif

  map<string, string>& m = server_settings_map_;

  ticpp::Iterator<ticpp::Element> elem(server_settings_->FirstChildElement(xmltags::ctl_port, true));
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = server_settings_->FirstChildElement(xmltags::udp_bc_addr, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = server_settings_->FirstChildElement(xmltags::udp_port, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  //elem = server_settings_->FirstChildElement( xmltags::tid_port, true);
  //m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));




  for(elem = server_settings_->FirstChildElement(true) ; elem != elem.end(); elem++)
  {
    string tmp(elem->Value());
    if(tmp == xmltags::ctl_port || tmp == xmltags::udp_bc_addr || tmp == xmltags::udp_port)
      continue;

    //  Store data
    /// TODO: Write seperate function for filesaving
    if(tmp == xmltags::store_data)
    {
      ticpp::Iterator< ticpp::Attribute > attribute;
      attribute = attribute.begin(elem.Get());

      bool store = 0;
      if(attribute != attribute.end())
      {
        if(attribute->Name() == xmltags::store_data_value)
        {
          store = equalsYesOrNo( attribute->Value());

          if(store)
          {
            parseFileLocation(elem, m);

            string file_exists(xmltags::file_exists_new_file);
            ticpp::Iterator<ticpp::Element>  child = elem->FirstChildElement(xmltags::file_exists, false);

            if(child != child.end())
              file_exists = lexical_cast<string>( child->GetText(false) );

            if(!( (file_exists == xmltags::file_exists_new_file)
                  || (file_exists == xmltags::file_exists_overwrite)) )
            {
              throw(std::invalid_argument( "Error in xml file: " + xmltags::file_exists + " -- allowed options: "
                                           + xmltags::file_exists_new_file + " or " + xmltags::file_exists_overwrite));
            }

            m.insert(pair<string, string>(xmltags::file_exists, file_exists));

            string append_str(xmltags::append_to_filename_default);
            child = elem->FirstChildElement(xmltags::append_to_filename, false);
            if(child != child.end())
              append_str = lexical_cast<string>( child->GetText(false) );
            m.insert(pair<string, string>(xmltags::append_to_filename, append_str));

            bool cont_sav = 0;
            child = elem->FirstChildElement(xmltags::continous_saving, false);
            if(child != child.end())
              cont_sav = equalsYesOrNo( child->GetText(false) );
            m.insert(pair<string, string>(xmltags::continous_saving,  lexical_cast<string>(cont_sav) ));
          }
        }
      }

      m.insert(std::make_pair(xmltags::store_data, lexical_cast<string>( store )));
      continue;
    }

    if(tmp == xmltags::tid_server)
    {
      ticpp::Iterator< ticpp::Attribute > attribute;
      attribute = attribute.begin(elem.Get());
      for(attribute = attribute.begin(elem.Get()); attribute != attribute.end();
          attribute++)
        m.insert(pair<string, string>(attribute->Name(), attribute->Value()));
    }


    if(elem->GetText(false) != "")
      m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

    ticpp::Iterator< ticpp::Attribute > attribute;
    for(attribute = attribute.begin(elem.Get()); attribute != attribute.end();
        attribute++)
      m.insert(pair<string, string>(attribute->Name(), attribute->Value()));
  }
  return(server_settings_map_);
}

//-----------------------------------------------------------------------------

void XMLParser::checkHardwareAttributes(std::map<std::string,std::string>& m)
{
  #ifdef DEBUG
    cout << "XMLParser: checkHardwareAttributes" << endl;
  #endif

  map<string,string>::iterator it;
  string error;
  if( (it = m.find(xmltags::hardware_name)) == m.end())
  {
    throw(std::invalid_argument("Error in hardware -- Device name not specified!"));
  }
  if(it->second.empty() || it->second == " ")
  {
    throw(std::invalid_argument("Error in hardware -- Device has to be named!"));
  }
}

//---------------------------------------------------------------------------------------

void XMLParser::parseFileLocation(ticpp::Iterator<ticpp::Element> elem, map<string,string>& m)
{

  string filename;
  string filetype;
  string filepath(xmltags::filepath_default);
  string tmp_filetype;

  ticpp::Iterator<ticpp::Element> child(elem->FirstChildElement(xmltags::filename, true));
  filename = child->GetText(false);

  child = elem->FirstChildElement(xmltags::filepath, false);
  if(child != child.end())
    filepath = child->GetText(false);

  size_t pos = filename.rfind(".");

  if(pos != string::npos)
    tmp_filetype = to_lower_copy(filename.substr(pos + 1, filename.length()-pos ));

  child = elem->FirstChildElement(xmltags::filetype, false);
  if(child != child.end())
    filetype = to_lower_copy(child->GetText(false));

  if(filetype == "" && tmp_filetype != "")
    filetype = tmp_filetype;
  if(filetype == tmp_filetype)
    filename = filename.substr(0, pos);


  if(filename == "")
    throw(ticpp::Exception("Error in " + xmltags::server_settings + " -- No filename given!"));
  if(filetype == "")
    throw(ticpp::Exception("Error in " + xmltags::server_settings + " -- No filetype given!"));

  m.insert(pair<string, string>(xmltags::filename, filename));
  m.insert(pair<string, string>(xmltags::filetype, filetype));
  m.insert(pair<string, string>(xmltags::filepath, filepath));
}

} // Namespace tobiss

//---------------------------------------------------------------------------------------
