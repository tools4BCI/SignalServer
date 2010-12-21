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
* @file eeg_simulator.cpp
**/

#include "hardware/eeg_simulator.h"

#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>

namespace tobiss
{

using boost::uint16_t;
using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::make_pair;

using boost::lexical_cast;
using boost::bad_lexical_cast;

const HWThreadBuilderTemplateRegistrator<EEGSimulator> EEGSimulator::factory_registrator_ ("eegsim", "eegsimulator");

const std::string EEGSimulator::xml_eeg_sim_port_("port");
const std::string EEGSimulator::xml_eeg_config_("eeg_config");
const std::string EEGSimulator::xml_sine_config_("sine_config");
const std::string EEGSimulator::xml_scaling_("scaling");
const std::string EEGSimulator::xml_offset_("offset");
const std::string EEGSimulator::xml_frequ_("frequ");
const std::string EEGSimulator::xml_amplitude_("amplitude");
const std::string EEGSimulator::xml_phase_("phase");

static const float EEG_DIST_MEAN = 0;
static const float EEG_DIST_STD  = 11;  // lt. Martin Billinger, BCI Comp4, Graz Dataset A, 4.11.2010

static const unsigned int MESSAGE_BUFFER_SIZE_BYTE = 4096;

//-----------------------------------------------------------------------------

EEGSimulator::EEGSimulator(boost::asio::io_service& io,
                           ticpp::Iterator<ticpp::Element> hw)
 : ArtificialSignalSource(io, hw), twister_(static_cast<unsigned int>(std::time(0))),
   eeg_dist_(EEG_DIST_MEAN,EEG_DIST_STD), eeg_gen_(twister_,eeg_dist_),
   acceptor_(io), socket_(io), port_(0), connected_(0)
{

  setType("EEG Simulator");
  message_buffer_.resize(MESSAGE_BUFFER_SIZE_BYTE);

  setHardware(hw);

  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port_);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
  acceptor_.bind(endpoint);

  acceptor_.listen();
  acceptor_.async_accept(socket_, boost::bind(&EEGSimulator::acceptHandler, this,
                                              boost::asio::placeholders::error));
  init();
  cout << " * EEGSimulator sucessfully initialized" << endl;
  cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_  << ", blocksize: " << blocks_  << endl;
}

//-----------------------------------------------------------------------------

EEGSimulator::~EEGSimulator()
{

}

//-----------------------------------------------------------------------------

void EEGSimulator::generateSignal()
{
  #ifdef DEBUG
    cout << "EEGSimulator: generateSignal" << endl;
  #endif

  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // create EEG
  std::map<boost::uint16_t, EEGConfig>::iterator eeg_it;
  for(boost::uint16_t n = 0; n < nr_ch_ ; n++)
  {
    if( (eeg_it = eeg_config_.find(n)) != eeg_config_.end() )
      samples_[n] = eeg_gen_() * eeg_it->second.scaling_ + eeg_it->second.offset_;
    else
      samples_[n] = eeg_gen_();
  }

  // add sine waves
  std::multimap<boost::uint16_t, SineConfig>::iterator sine_it;

  for(boost::uint16_t n = 0; n < nr_ch_ ; n++)
  {
    for (sine_it = sine_configs_.equal_range(n).first;
         sine_it != sine_configs_.equal_range(n).second;
         ++sine_it)
    {
      samples_[n] += sine_it->second.amplitude_ *
                     sin(step_ * 2 * M_PI * sine_it->second.freq_ + sine_it->second.phase_);
    }
  }

  (step_ < 1-cycle_dur_ ? step_ += cycle_dur_ : step_ = 0);
  t_->expires_at(t_->expires_at() + td_);

  if(blocks_  == 1)
  {

    boost::unique_lock<boost::mutex> syn(sync_mut_);
    samples_available_ = true;
    data_.setSamples(samples_);
    lock.unlock();
    cond_.notify_all();
    if(isMaster() && acquiring_)
    {
      cond_.wait(sync_mut_);
      // if( !cond_.timed_wait(sync_mut_, td_))
      //   cerr << "Warning: New data was not fetched fast enough!" << endl;
      //   throw std::runtime_error("SineGenerator::genSine() -- Timeout; New data was not fetched fast enough!");
    }
    syn.unlock();
  }
  else
  {
    buffer_.appendBlock(samples_);
    current_block_++;

    if(current_block_ == blocks_ )
    {
      boost::unique_lock<boost::mutex> syn(sync_mut_);
      samples_available_ = true;
      data_ = buffer_;
      lock.unlock();
      cond_.notify_all();
      buffer_.reset();
      current_block_ = 0;
      if(isMaster() && acquiring_)
      {
        cond_.wait(sync_mut_);
        // if( !cond_.timed_wait(sync_mut_, td_))
        //   cerr << "Warning: New data was not fetched fast enough!" << endl;
        //   throw std::runtime_error("SineGenerator::genSine() -- Timeout; New data was not fetched fast enough!");
      }
      syn.unlock();
    }
  }

  if(running_)
    t_->async_wait(boost::bind(&EEGSimulator::generateSignal, this));
}

//-----------------------------------------------------------------------------

void EEGSimulator::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "EEGSimulator: setHardware" << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));

  setDeviceSettings(ds);

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_chset_, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_chset_)
      {
      string ex_str(type_ + " -- ");
        ex_str += "Multiple channel_settings found!";
        throw(std::invalid_argument(ex_str));
      }
      setChannelSettings(cs);
  }
}

//-----------------------------------------------------------------------------

void EEGSimulator::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "EEGSimulator: setDeviceSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs_,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);

  elem = father->FirstChildElement(xml_eeg_sim_port_,true);
  if(elem != elem.end())
    setPort(elem);

  elem = father->FirstChildElement(xml_eeg_config_,false);
  if(elem != elem.end())
    setDeviceEEGConfig(elem);

  elem = father->FirstChildElement(xml_sine_config_,false);
  if(elem != elem.end())
    setDeviceSineConfig(elem);
}

//---------------------------------------------------------------------------------------

void EEGSimulator::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "EEGSimulator: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);

  elem = father->FirstChildElement(xml_eeg_config_,false);
  if(elem != elem.end())
    setChannelEEGConfig(elem);

  elem = father->FirstChildElement(xml_sine_config_,false);
  if(elem != elem.end())
    setChannelSineConfig(elem);
}

//-----------------------------------------------------------------------------

void EEGSimulator::setPort(ticpp::Iterator<ticpp::Element>const &elem)
{
  boost::int16_t port = 0;
  try
  {
    port = lexical_cast<boost::int16_t>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- Port: value is not a number!");
    throw(std::invalid_argument(ex_str));
  }
  if(port <= 0)
  {
    string ex_str(type_ + " -- Port: value is <= 0!");
    throw(std::invalid_argument(ex_str));
  }
  port_ = port;
}

//-----------------------------------------------------------------------------

void EEGSimulator::setDeviceEEGConfig(ticpp::Iterator<ticpp::Element>const &elem)
{
  EEGConfig eeg_cfg = getEEGConfig(elem);

  for(unsigned int n = 0; n < channel_info_.size(); n++)
    eeg_config_.insert(  make_pair(n, eeg_cfg) );
}

//-----------------------------------------------------------------------------

void EEGSimulator::setDeviceSineConfig(ticpp::Iterator<ticpp::Element>const &elem)
{
  SineConfig sine_cfg = getSineConfig(elem);

  for(unsigned int n = 0; n < channel_info_.size(); n++)
    sine_configs_.insert(  make_pair(n, sine_cfg) );
}

//-----------------------------------------------------------------------------

void EEGSimulator::setChannelEEGConfig(ticpp::Iterator<ticpp::Element>const &father)
{
  ticpp::Iterator<ticpp::Element> elem;
  elem = father->FirstChildElement(hw_chset_ch_,false);
  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+xml_eeg_config_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      checkEEGConfigAttributes(elem);

      uint16_t ch = 0;
      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ xml_eeg_config_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ xml_eeg_config_ + "> - Channel "+ lexical_cast<string>(ch) +" not set!";
        throw(std::invalid_argument(ex_str));
      }

      eeg_config_[ch-1] =  getEEGConfig(elem) ;
    }
    else
      throw(std::invalid_argument("EEGSimulator::setChannelEEGConfig -- Tag not equal to \""+hw_chset_ch_+"\"!"));
}

//-----------------------------------------------------------------------------

void EEGSimulator::setChannelSineConfig(ticpp::Iterator<ticpp::Element>const &father)
{
  ticpp::Iterator<ticpp::Element> elem;
  elem = father->FirstChildElement(hw_chset_ch_,false);
  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+xml_sine_config_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      checkSineConfigAttributes(elem);

      uint16_t ch = 0;
      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ xml_sine_config_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ xml_sine_config_ + "> - Channel "+ lexical_cast<string>(ch) +" not set!";
        throw(std::invalid_argument(ex_str));
      }

      sine_configs_.insert(  make_pair(ch-1, getSineConfig(elem) ) );
    }
    else
      throw(std::invalid_argument("EEGSimulator::setChannelSineConfig -- Tag not equal to \""+hw_chset_ch_+"\"!"));
}

//---------------------------------------------------------------------------------------

void EEGSimulator::checkEEGConfigAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{

  if(!elem.Get()->HasAttribute(xml_scaling_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+xml_eeg_config_+"> given, EEG scaling ("+xml_scaling_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(xml_offset_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+xml_eeg_config_+"> given, EEG offset ("+xml_offset_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void EEGSimulator::checkSineConfigAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{

  if(!elem.Get()->HasAttribute(xml_frequ_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+xml_sine_config_+"> given, EEG scaling ("+xml_frequ_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(xml_amplitude_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+xml_sine_config_+"> given, EEG offset ("+xml_amplitude_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(xml_phase_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+xml_sine_config_+"> given, EEG offset ("+xml_phase_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
}

//-----------------------------------------------------------------------------

EEGSimulator::EEGConfig EEGSimulator::getEEGConfig(ticpp::Iterator<ticpp::Element>const &elem)
{
  checkEEGConfigAttributes(elem);

  EEGConfig eeg_cfg;
  try
  {
    eeg_cfg.scaling_ = lexical_cast<double>( elem.Get()->GetAttribute(xml_scaling_) );
    eeg_cfg.offset_  = lexical_cast<double>( elem.Get()->GetAttribute(xml_offset_) );
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+xml_eeg_config_+"> given, but scaling or offset is not a number!";
    throw(std::invalid_argument(ex_str));
  }

  return(eeg_cfg);
}

//-----------------------------------------------------------------------------

EEGSimulator::SineConfig EEGSimulator::getSineConfig(ticpp::Iterator<ticpp::Element>const &elem)
{

  checkSineConfigAttributes(elem);

  SineConfig sine_cfg;
  try
  {
    sine_cfg.freq_ = lexical_cast<double>( elem.Get()->GetAttribute(xml_frequ_) );
    sine_cfg.amplitude_  = lexical_cast<double>( elem.Get()->GetAttribute(xml_amplitude_) );
    sine_cfg.phase_  = lexical_cast<double>( elem.Get()->GetAttribute(xml_phase_) );
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+xml_sine_config_+"> given, but frequency, amplitude or phase is not a number!";
    throw(std::invalid_argument(ex_str));
  }

  return(sine_cfg);
}

//-----------------------------------------------------------------------------

void EEGSimulator::handleAsyncRead(const boost::system::error_code& ec,
                                     std::size_t bytes_transferred )
{
  std::cout << ec.message() << std::endl;
  if(ec)
    throw(std::runtime_error("EEGSimulator::handleAsyncRead() -- \
                             Error handling async read -- bytes transferred: " + bytes_transferred));

  // TODO: check if message is complete


  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // TODO: modify sine- or eeg config maps


  lock.unlock();
}

//-----------------------------------------------------------------------------

void EEGSimulator::acceptHandler(const boost::system::error_code& error)
{
  // if already conencted with a client --> abort connection
  // if not needed, delete connected_ member

  if (!error)
  {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_buffer_),
                            boost::bind(&EEGSimulator::handleAsyncRead,
                                        this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred)
                            );
  }

  acceptor_.async_accept(socket_, boost::bind(&EEGSimulator::acceptHandler, this,
                                              boost::asio::placeholders::error));
}


//-----------------------------------------------------------------------------


}  // tobiss
