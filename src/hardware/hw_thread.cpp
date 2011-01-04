/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file hw_thread.cpp
**/

#include "hardware/hw_thread.h"
#include "tia/constants.h"
#include <boost/algorithm/string.hpp>

#include <boost/lexical_cast.hpp>

namespace tobiss
{

using boost::lexical_cast;
using boost::bad_lexical_cast;

using boost::uint16_t;
using boost::uint32_t;

using boost::algorithm::to_lower_copy;

using std::map;
using std::pair;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::dec;
using std::hex;

//-----------------------------------------------------------------------------

//const string HWThread::hardware_("hardware");
//const string HWThread::hardware_name_("name");
const string HWThread::hardware_version_("version");
const string HWThread::hardware_serial_("serial");

const string HWThread::hw_mode_("mode");
const string HWThread::hw_devset_("device_settings");
const string HWThread::hw_fs_("sampling_rate");

const string HWThread::hw_channels_("measurement_channels");
const string HWThread::hw_ch_nr_("nr");
const string HWThread::hw_ch_names_("names");
const string HWThread::hw_ch_type_("type");

const string HWThread::hw_blocksize_("blocksize");

const string HWThread::hw_chset_("channel_settings");
const string HWThread::hw_chset_sel_("selection");
const string HWThread::hw_chset_ch_("ch");
const string HWThread::hw_chset_nr_("nr");
const string HWThread::hw_chset_name_("name");

//-----------------------------------------------------------------------------

void HWThread::checkMandatoryHardwareTags(ticpp::Iterator<ticpp::Element> hw)
{
  #ifdef DEBUG
    cout << "HWThread: checkMandatoryHardwareTags" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(hw);
  ticpp::Iterator< ticpp::Attribute > attribute;

  try
  {
    checkMandatoryHardwareTagsXML(hw);
  }
  catch(ticpp::Exception& e)
  {
    string ex_str(type_ + " -- ");
    throw(std::invalid_argument(ex_str + e.what()));
  }

  elem = hw->FirstChildElement(hw_mode_, true);
  if(equalsMaster(elem->GetText(false)))
    mode_ = MASTER;
  if(equalsSlave(elem->GetText(false)))
    mode_ = SLAVE;
  if(equalsAperiodic(elem->GetText(false)))
    mode_ = APERIODIC;
}

//-----------------------------------------------------------------------------

bool HWThread::samplesAvailable()
{
  #ifdef DEBUG
    cout << "HWThread: samplesAvailable" << endl;
  #endif

  boost::shared_lock<boost::shared_mutex> lock(rw_);
  bool tmp(samples_available_);
  lock.unlock();
  return(tmp);
}

//-----------------------------------------------------------------------------

void HWThread::setSamplingRate(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "HWThread: setSamplingRate" << endl;
  #endif

    /// @todo: Allow also float sampling rates (e.g. 0.1)
  try
  {
    fs_ = lexical_cast<double>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- Sampling Rate is not a number!");
    throw(std::invalid_argument(ex_str));
  }
  if(fs_ == 0)
  {
    string ex_str(type_ + " -- Sampling Rate is 0!");
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------
void HWThread::setDeviceChannels(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "HWThread: setDeviceChannels" << endl;
  #endif

  string naming;
  string type;

  try
  {
    parseDeviceChannels(elem, nr_ch_, naming, type);
  }
  catch(ticpp::Exception& e)
  {
    string ex_str(type_ + " -- ");
    throw(std::invalid_argument(ex_str + e.what()));
  }

  Constants cst;
  for(uint16_t n = 1; n <= nr_ch_; n++)
    channel_info_.insert(
        pair<uint16_t, pair<string, uint32_t> >(n, pair<string, uint32_t>(naming,
                                                                          cst.getSignalFlag(type))));
  homogenous_signal_type_ = 1;

  setChannelTypes();
}

//---------------------------------------------------------------------------------------
void HWThread::setBlocks(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "HWThread: setBlocks" << endl;
  #endif

  boost::int16_t blocks = 0;

  try
  {
    blocks = lexical_cast<boost::int16_t>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- Blocksize: value is not a number!");
    throw(std::invalid_argument(ex_str));
  }
  if(blocks <= 0)
  {
    string ex_str(type_ + " -- Blocksize: value is <= 0!");
    throw(std::invalid_argument(ex_str));
  }

  blocks_ = blocks;
}

//---------------------------------------------------------------------------------------
void HWThread::setChannelSelection(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "HWThread: setChannelSelection" << endl;
  #endif

  bool nr_ch_dirty = 0;
  ticpp::Iterator<ticpp::Element> elem;
  if(nr_ch_ == 0)
    elem = father->FirstChildElement(hw_chset_ch_,true);
  else
    elem = father->FirstChildElement(hw_chset_ch_,false);

  Constants cst;
  if (elem != elem.end())
  {
    if(channel_info_.size() != 0)
      nr_ch_dirty = 1;

    channel_info_.clear();

    for(ticpp::Iterator<ticpp::Element> it(elem); it != it.end(); it++)
      if(it->Value() == hw_chset_ch_)
      {
        string name;
        string type;
        uint16_t ch = 0;
        try
        {
          parseChannelSelection(it, ch, name, type);
        }
        catch(ticpp::Exception& e)
        {
          string ex_str(type_ + " -- ");
          throw(std::invalid_argument(ex_str + e.what()));
        }
        channel_info_.insert(
            pair<uint16_t, pair<string, uint32_t> >(ch, pair<string, uint32_t>(name,
                                                                               cst.getSignalFlag(type))));
      }
      nr_ch_ = channel_info_.size();
  }

  if(nr_ch_dirty)
  {
    cout << " *** Danger ***" << endl;
    cout << "   -- Global setting \"Nr. Of Channels\" and naming overridden by individual channel setting!" << endl;
    map<uint16_t, pair<string, uint32_t> >::iterator m_it;
    for ( m_it=channel_info_.begin() ; m_it != channel_info_.end(); m_it++ )
    {
      cout << "    Channel: " << dec << (*m_it).first;
      cout << "  ...  Name: " << (*m_it).second.first;
      cout << "  ...  Type: " << cst.getSignalName((*m_it).second.second) << " (0x" << hex << (*m_it).second.second << ")" << endl;
    }
    cout << dec << endl;
  }

  map<uint16_t, pair<string, uint32_t> >::iterator m_it = channel_info_.begin();
  uint32_t sig_type = (*m_it).second.second;
  for (m_it++; m_it != channel_info_.end(); m_it++ )
    if(sig_type != (*m_it).second.second)
      homogenous_signal_type_ = 0;

  setChannelTypes();
}

//-----------------------------------------------------------------------------

void HWThread::setChannelTypes()
{
  #ifdef DEBUG
    cout << "HWThread: setChannelTypes" << endl;
  #endif

  channel_types_.clear();
  for(map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
      it != channel_info_.end(); it++ )
    channel_types_.push_back((*it).second.second);
}

//---------------------------------------------------------------------------------------

bool HWThread::equalsOnOrOff(const std::string& s)
{
  if(to_lower_copy(s) == "on"  || s == "1")
    return(true);
  if(to_lower_copy(s) == "off" || s == "0")
    return(false);
  else
  {
    string e = s + " -- Value equals neiter \"on, off, 0 or 1\"!";
    throw std::invalid_argument(e);
  }
}

//-----------------------------------------------------------------------------

bool HWThread::equalsYesOrNo(const std::string& s)
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

//-----------------------------------------------------------------------------

bool HWThread::equalsMaster(const std::string& s)
{
  return(to_lower_copy(s) == "master");
}

//-----------------------------------------------------------------------------

bool HWThread::equalsSlave(const std::string& s)
{
  return(to_lower_copy(s) == "slave");
}

//-----------------------------------------------------------------------------

bool HWThread::equalsAperiodic(const std::string& s)
{
  return(to_lower_copy(s) == "aperiodic");
}

//---------------------------------------------------------------------------------------

void HWThread::parseDeviceChannels(ticpp::Iterator<ticpp::Element>const &elem, boost::uint16_t& nr_ch,
                             std::string& naming, std::string& type)
{
  string nr_channels;

  if(!elem.Get()->HasAttribute(hw_ch_nr_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <" + hw_channels_ +"> given, number of channels ("+hw_ch_nr_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_ch_names_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+ hw_channels_ +"> given, channel names ("+hw_ch_names_ +") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_ch_type_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_channels_+"> given, channels type ("+hw_ch_type_+") not given!";
    throw(std::invalid_argument(ex_str));
  }

  nr_channels = elem.Get()->GetAttribute(hw_ch_nr_);
  naming      = elem.Get()->GetAttribute(hw_ch_names_);
  type        = elem.Get()->GetAttribute(hw_ch_type_);

  try
  {
    nr_ch = lexical_cast<int>(nr_channels);
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_channels_+"> given, but number of channels is not a number!";
    throw(std::invalid_argument(ex_str));
  }
  if(nr_ch == 0)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_channels_+"> given, but number of channels (nr) is 0!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void HWThread::parseChannelSelection(ticpp::Iterator<ticpp::Element>const &elem, boost::uint16_t& ch,
                               std::string& name, std::string& type)
{
  string channel;

  if(!elem.Get()->HasAttribute(hw_chset_nr_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_channels_+"> given, channel number ("+hw_ch_nr_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_chset_name_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_channels_+"> given, channel name ("+hw_ch_names_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_ch_type_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_channels_+"> given, channel type ("+hw_ch_type_+") not given!";
    throw(std::invalid_argument(ex_str));
  }

  channel = elem.Get()->GetAttribute(hw_chset_nr_);
  name    = elem.Get()->GetAttribute(hw_chset_name_);
  type    = elem.Get()->GetAttribute(hw_ch_type_);

  try
  {
    ch = lexical_cast<int>(channel);
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+ hw_chset_ + "> - "+ hw_chset_sel_ +": Channel is not a number!";
    throw(std::invalid_argument(ex_str));
  }
  if(ch == 0)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+ hw_chset_ + "> - "+ hw_chset_sel_ +": Channel is 0 --> First channel-nr is 1!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void HWThread::checkMandatoryHardwareTagsXML(ticpp::Iterator<ticpp::Element> hw)
{
  #ifdef DEBUG
    cout << "HWThread: checkMandatoryHardwareTags" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;

  elem = hw->FirstChildElement(hw_mode_, true);
  if( !(equalsMaster(elem->GetText(true)) || equalsSlave(elem->GetText(true))
        || equalsAperiodic(elem->GetText(true))))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Mode is neither master, slave or aperiodic!";
    throw(std::invalid_argument(ex_str));
  }

  elem = hw->FirstChildElement(hw_devset_, true);
  for(ticpp::Iterator<ticpp::Element> it(elem); ++it != it.end(); )
    if(it->Value() == hw_devset_)
    {
    string ex_str(type_ + " -- ");
      ex_str += "Multiple device_settings found!";
      throw(std::invalid_argument(ex_str));
    }
}

//-----------------------------------------------------------------------------

} // Namespace tobiss
