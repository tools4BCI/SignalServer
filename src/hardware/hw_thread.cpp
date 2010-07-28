#include "hardware/hw_thread.h"

#include <boost/lexical_cast.hpp>

#include "definitions/constants.h"

namespace tobiss
{

using boost::lexical_cast;
using boost::bad_lexical_cast;

using boost::uint16_t;
using boost::uint32_t;

using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------

void HWThread::checkMandatoryHardwareTags(ticpp::Iterator<ticpp::Element> hw)
{
  #ifdef DEBUG
    cout << "HWThread: checkMandatoryHardwareTags" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(hw);
  ticpp::Iterator< ticpp::Attribute > attribute;

  for(attribute = attribute.begin(elem.Get()); attribute != attribute.end();
      attribute++)
  m_.insert(pair<string, string>(attribute->Name(), attribute->Value()));

  try
  {
    parser_.checkMandatoryHardwareTags(hw);
  }
  catch(ticpp::Exception& e)
  {
    string ex_str("Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ");
    throw(ticpp::Exception(ex_str + e.what()));
  }

  elem = hw->FirstChildElement(cst_.hw_mode, true);
  if(cst_.equalsMaster(elem->GetText(false)))
    mode_ = MASTER;
  if(cst_.equalsSlave(elem->GetText(false)))
    mode_ = SLAVE;
  if(cst_.equalsAperiodic(elem->GetText(false)))
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

  try
  {
    fs_ = lexical_cast<int>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Sampling Rate is not a number!";
    throw(ticpp::Exception(ex_str));
  }
  if(fs_ == 0)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Sampling rate is 0!";
    throw(ticpp::Exception(ex_str));
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
    parser_.parseDeviceChannels(elem, nr_ch_, naming, type);
  }
  catch(ticpp::Exception& e)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    throw(ticpp::Exception(ex_str + e.what()));
  }

  for(uint16_t n = 1; n <= nr_ch_; n++)
    channel_info_.insert(pair<uint16_t, pair<string, uint32_t> >(n, pair<string, uint32_t>(naming, cst_.getSignalFlag(type))));
  homogenous_signal_type_ = 1;

  setChannelTypes();
}

//---------------------------------------------------------------------------------------
void HWThread::setBlocks(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "HWThread: setBlocks" << endl;
  #endif

  try
  {
    blocks_ = lexical_cast<int>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Buffersize: value is not a number!";
    throw(ticpp::Exception(ex_str));
  }
  if(blocks_ == 0)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Buffersize: value is 0!";
    throw(ticpp::Exception(ex_str));
  }
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
    elem = father->FirstChildElement(cst_.hw_cs_ch,true);
  else
    elem = father->FirstChildElement(cst_.hw_cs_ch,false);

  if (elem != elem.end())
  {
    if(channel_info_.size() != 0)
      nr_ch_dirty = 1;

    channel_info_.clear();

    for(ticpp::Iterator<ticpp::Element> it(elem); it != it.end(); it++)
      if(it->Value() == cst_.hw_cs_ch)
      {
        string name;
        string type;
        uint16_t ch = 0;
        try
        {
          parser_.parseChannelSelection(it, ch, name, type);
        }
        catch(ticpp::Exception& e)
        {
          string ex_str;
          ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
          throw(ticpp::Exception(ex_str + e.what()));
        }
        channel_info_.insert(pair<uint16_t, pair<string, uint32_t> >(ch, pair<string, uint32_t>(name, cst_.getSignalFlag(type))));
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
      cout << "  ...  Type: " << cst_.getSignalName((*m_it).second.second) << " (0x" << hex << (*m_it).second.second << ")" << endl;
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

//-----------------------------------------------------------------------------

} // Namespace tobiss
