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
  : doc_(xml_file), external_data_file_(0)
{
  #ifdef DEBUG
    cout << "XMLParser: Constructor" << endl;
  #endif

  doc_.LoadFile();
  ticpp::Iterator<ticpp::Element> config(doc_.FirstChildElement(cst_.tobi, true));

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


  server_settings_ = config->FirstChildElement(cst_.ss, true);
  for(ticpp::Iterator<ticpp::Element> it(server_settings_); ++it != it.end(); )
    if(it->Value() == cst_.ss)
      throw(ticpp::Exception("Error -- Multiple server settings found!"));

  if(external_data_file_)
    return;

  subject_ = config->FirstChildElement(cst_.subject, true);
  for(ticpp::Iterator<ticpp::Element> it(subject_); ++it != it.end(); )
    if(it->Value() == cst_.subject)
      throw(ticpp::Exception("Error -- Only one subject definition allowed!"));


  // FIXME  -- hardcoded strings  --> shifted to HWThread

  ticpp::Iterator< ticpp::Attribute > attribute;
  for(ticpp::Iterator<ticpp::Element> it(config->FirstChildElement("hardware", true));
      it != it.end(); it++)
      if(it->Value() == "hardware")
    {
      map<string, string> m;
      for(attribute = attribute.begin(it.Get()); attribute != attribute.end();
          attribute++)
        m.insert(pair<string, string>(attribute->Name(), attribute->Value()));
      checkHardwareAttributes(m);

      hardware_.push_back(make_pair(m.find("name")->second,it));
    }
}

//---------------------------------------------------------------------------------------

bool XMLParser::equalsYesOrNo(const std::string& s)
{
  if(to_lower_copy(s) == "yes"  || s == "1")
    return(true);
  if(to_lower_copy(s) == "no" || s == "0")
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

  if(external_data_file_)
    return(subject_map_);

  map<string, string>& m = subject_map_;
  ticpp::Iterator<ticpp::Element> elem(subject_->FirstChildElement(cst_.s_id,true));
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(cst_.s_first_name, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(cst_.s_surname, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(cst_.s_birthday, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = subject_->FirstChildElement(cst_.s_sex, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  for(elem = subject_->FirstChildElement(true) ; elem != elem.end(); elem++)
  {
    string tmp(elem->Value());
    if(tmp == cst_.s_id || tmp == cst_.s_first_name || tmp == cst_.s_surname || tmp == cst_.s_birthday || tmp == cst_.s_sex)
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

  ticpp::Iterator<ticpp::Element> elem(server_settings_->FirstChildElement(cst_.ss_ctl_port, true));
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = server_settings_->FirstChildElement(cst_.ss_udp_bc_addr, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = server_settings_->FirstChildElement(cst_.ss_udp_port, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));

  elem = server_settings_->FirstChildElement(cst_.ss_tid_port, true);
  m.insert(pair<string, string>(elem->Value(), elem->GetText(false)));


  for(elem = server_settings_->FirstChildElement(true) ; elem != elem.end(); elem++)
  {
    string tmp(elem->Value());
    if(tmp == cst_.ss_ctl_port || tmp == cst_.ss_udp_bc_addr || tmp == cst_.ss_udp_port
       || tmp == cst_.ss_tid_port)
      continue;

    //    Save as gdf part
    if(tmp == cst_.ss_store_data) ///< TODO: and move to other file
    {
      parseFileLocation(elem, m);

      string overwrite(cst_.ss_file_overwrite_default);
      ticpp::Iterator<ticpp::Element>  child = elem->FirstChildElement(cst_.ss_file_overwrite, false);

      if(child != child.end())
        overwrite = lexical_cast<string>( equalsYesOrNo(child->GetText(false)) );

      m.insert(pair<string, string>(cst_.ss_file_overwrite, overwrite));

      continue;
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

  // FIXME  -- hardcoded strings  --> shifted to HWThread

  map<string,string>::iterator it;
  string error;
  if( (it = m.find("name")) == m.end())
  {
    throw(ticpp::Exception("Error in hardware -- Device name not specified!"));
  }
  if(it->second.empty() || it->second == " ")
  {
    throw(ticpp::Exception("Error in hardware -- Device has to be named!"));
  }
}

//---------------------------------------------------------------------------------------

//void XMLParser::parseFileReader()
//{
//
//  //   ticpp::Iterator<ticpp::Element> elem(server_settings_->FirstChildElement(cst_.ss_ctl_port, true));
//
//  parseFileLocation(file_reader_, file_reader_map_);
//
//  string speedup = "0";
//  ticpp::Iterator<ticpp::Element>  child = file_reader_->FirstChildElement(cst_.fr_speedup, false);
//
//  if(child != child.end())
//    speedup = lexical_cast<string>( child->GetText(false) );
//
//  file_reader_map_.insert(pair<string, string>(cst_.fr_speedup, speedup));
//
//
//  string stop_end = "1";
//  child = file_reader_->FirstChildElement(cst_.fr_stop, false);
//
//  if(child != child.end())
//    stop_end = lexical_cast<string>( cst_.equalsYesOrNo(child->GetText(false)) );
//
//  file_reader_map_.insert(pair<string, string>(cst_.fr_stop, stop_end));
//
//
//  // parse subject info from file
//
//
//  // parse signal info from file
//
//
////   map<string,string>::iterator it(file_reader_map_.begin());
////   for( ; it != file_reader_map_.end(); it++)
////     cout << "First: " << it->first << ";  Second: " << it->second << endl;
//}

//---------------------------------------------------------------------------------------

void XMLParser::parseFileLocation(ticpp::Iterator<ticpp::Element> elem, map<string,string>& m)
{

  string filename;
  string filetype;
  string filepath(cst_.ss_filepath_default);
  string tmp_filetype;

  ticpp::Iterator<ticpp::Element> child(elem->FirstChildElement(cst_.ss_filename, true));
  filename = child->GetText(false);

  child = elem->FirstChildElement(cst_.ss_filepath, false);
  if(child != child.end())
    filepath = child->GetText(false);

  size_t pos = filename.rfind(".");

  if(pos != string::npos)
    tmp_filetype = to_lower_copy(filename.substr(pos + 1, filename.length()-pos ));

  child = elem->FirstChildElement(cst_.ss_filetype, false);
  if(child != child.end())
    filetype = to_lower_copy(child->GetText(false));

  if(filetype == "" && tmp_filetype != "")
    filetype = tmp_filetype;
  if(filetype == tmp_filetype)
    filename = filename.substr(0, pos);


  if(filename == "")
    throw(ticpp::Exception("Error in " + cst_.ss + " -- No filename given!"));
  if(filetype == "")
    throw(ticpp::Exception("Error in " + cst_.ss + " -- No filetype given!"));

  m.insert(pair<string, string>(cst_.ss_filename, filename));
  m.insert(pair<string, string>(cst_.ss_filetype, filetype));
  m.insert(pair<string, string>(cst_.ss_filepath, filepath));
}

} // Namespace tobiss

//---------------------------------------------------------------------------------------
