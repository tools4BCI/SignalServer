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
* @file eeg_simulator.cpp
**/

#include "hardware/eeg_simulator.h"

#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>

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


//static const std::string EEGSIM_MSG_STRING("eegsimconfig");
//static const std::string EEGSIM_EEG_STRING("eeg");
//static const std::string EEGSIM_SINE_STRING("sine");

//static const std::string EEGSIM_MSG_CMD_DELIMITER(":");
//static const std::string EEGSIM_MSG_PARAM_DELIMITER("/");
//static const std::string EEGSIM_MSG_CHANNEL_DELIMITER(",");

//static const std::string EEGSIM_MSG_GETCONFIG("getconfig");
//static const std::string EEGSIM_MSG_CONFIG("config");
//static const std::string EEGSIM_MSG_OK("ok");
//static const std::string EEGSIM_MSG_ERROR("error");


//static const unsigned int MESSAGE_BUFFER_SIZE_BYTE = 4096;

//-----------------------------------------------------------------------------

EEGSimulator::EEGSimulator(boost::asio::io_service& io,
                           ticpp::Iterator<ticpp::Element> hw)
 : ArtificialSignalSource(io, hw), twister_(static_cast<unsigned int>(std::time(0))),
   eeg_dist_(EEG_DIST_MEAN,EEG_DIST_STD), eeg_gen_(twister_,eeg_dist_),
   acceptor_(io), socket_(io), port_(0), connected_(0), parser_(str_buffer_)
{

//  msg_types_map_.insert(std::make_pair( EEGSIM_MSG_GETCONFIG, GetConfig ) );
//  msg_types_map_.insert(std::make_pair( EEGSIM_MSG_CONFIG, Config ) );

  setType("EEG Simulator");
//  message_buffer_.resize(MESSAGE_BUFFER_SIZE_BYTE);

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
  std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig>::iterator eeg_it;
  for(boost::uint16_t n = 0; n < nr_ch_ ; n++)
  {
    if( (eeg_it = eeg_config_.find(n)) != eeg_config_.end() )
      samples_[n] = eeg_gen_() * eeg_it->second.scaling_ + eeg_it->second.offset_;
    else
      samples_[n] = eeg_gen_();
  }

  // add sine waves
  std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig>::iterator sine_it;

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
    buffer_.appendBlock(samples_, 1);
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
  EEGSimMsgParser::EEGConfig eeg_cfg = getEEGConfig(elem);

  for(unsigned int n = 0; n < channel_info_.size(); n++)
    eeg_config_.insert(  make_pair(n, eeg_cfg) );
}

//-----------------------------------------------------------------------------

void EEGSimulator::setDeviceSineConfig(ticpp::Iterator<ticpp::Element>const &elem)
{
  EEGSimMsgParser::SineConfig sine_cfg = getSineConfig(elem);

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

      setSineConfigInMultimap(ch, getSineConfig(elem) );
    }
    else
      throw(std::invalid_argument("EEGSimulator::setChannelSineConfig -- Tag not equal to \""+hw_chset_ch_+"\"!"));
}

//---------------------------------------------------------------------------------------

void EEGSimulator::setSineConfigInMultimap(boost::uint16_t ch, EEGSimMsgParser::SineConfig config)
{

  std::pair< std::multimap<boost::uint16_t,EEGSimMsgParser::SineConfig>::iterator,
             std::multimap<boost::uint16_t,EEGSimMsgParser::SineConfig>::iterator > range(sine_configs_.equal_range(ch-1));

  bool found = 0;
  for(std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig> ::iterator it(range.first);
      it != range.second; it++)
  {
    if(config.freq_ == it->second.freq_)
    {
      it->second = config;
      found = 1;
    }
  }

  if(!found)
    sine_configs_.insert(  make_pair(ch-1, config ) );

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

EEGSimMsgParser::EEGConfig EEGSimulator::getEEGConfig(ticpp::Iterator<ticpp::Element>const &elem)
{
  checkEEGConfigAttributes(elem);

  EEGSimMsgParser::EEGConfig eeg_cfg;
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

EEGSimMsgParser::SineConfig EEGSimulator::getSineConfig(ticpp::Iterator<ticpp::Element>const &elem)
{

  checkSineConfigAttributes(elem);

  EEGSimMsgParser::SineConfig sine_cfg;
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
  try
  {
    if(ec)
      throw(std::runtime_error("EEGSimulator::handleAsyncRead() -- Error handling async read -- bytes transferred: " + bytes_transferred));

    str_buffer_.clear();
    std::istream is(&message_buffer_);
    std::getline(is, str_buffer_);
    is.get();


    boost::system::error_code  ec;

    parser_.parseMessage();
    EEGSimMsgParser::MessageType msg_type = parser_.getMessageType();
    std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig> eeg;
    std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig> sine;

    switch (msg_type)
    {
    case EEGSimMsgParser::Invalid:
      throw(std::invalid_argument("Error -- Got invalid message type!"));
      break;
    case EEGSimMsgParser::GetConfig:
      socket_.send(boost::asio::buffer(parser_.buildConfigMsgString(eeg_config_, sine_configs_)),
                   0,ec);
      if(ec)
        throw(std::runtime_error("Error -- " + ec.message()  ));
      boost::asio::async_read_until(socket_,
                                    message_buffer_, '\n',
                                    boost::bind(&EEGSimulator::handleAsyncRead, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred) );

      return;

    case EEGSimMsgParser::Config:

      parser_.getConfigs(eeg, sine);
      if(eeg.size())
        updateEEGConfig(eeg);
      if(sine.size())
        updateSineConfig(sine);
      break;

    default:
      throw(std::invalid_argument("Error -- Could not determine message type!"));
      break;
    }
    socket_.send(boost::asio::buffer( parser_.getOKMsg() ), 0,ec);
  }
  catch(std::exception& e)
  {
    boost::system::error_code  ec;
    socket_.send(boost::asio::buffer( parser_.getErrorMsg() + e.what() ), 0, ec);

    std::cerr << "  *** EEG Simulator -- Connection closed to client: " << peer_ip_ << std::endl;

    socket_.close(ec);
    peer_ip_.clear();
    acceptor_.async_accept(socket_, boost::bind(&EEGSimulator::acceptHandler, this,
                                                boost::asio::placeholders::error));

    return;
  }

  boost::asio::async_read_until(socket_,
                          message_buffer_, '\n',
                          boost::bind(&EEGSimulator::handleAsyncRead, this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred) );

}

//-----------------------------------------------------------------------------

void EEGSimulator::acceptHandler(const boost::system::error_code& error)
{
  // if already conencted with a client --> abort connection
  // if not needed, delete connected_ member

  if (!error)
  {
    peer_ip_ = socket_.remote_endpoint().address().to_string();
    boost::asio::async_read_until(socket_,
                            message_buffer_, '\n',
                            boost::bind(&EEGSimulator::handleAsyncRead,
                                        this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred)
                            );
  }
  else
  {
    std::cerr << "EEGSimulator::acceptHandler" << error << std::endl;
  }

  if(!socket_.is_open())
    acceptor_.async_accept(socket_, boost::bind(&EEGSimulator::acceptHandler, this,
                                                boost::asio::placeholders::error));
}

//-----------------------------------------------------------------------------

void EEGSimulator::updateEEGConfig(std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig>& eeg)
{

  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // TODO: modify sine- or eeg config maps
  std::cout << "updateEEGConfig" << std::endl;
  eeg_config_ = eeg;
  lock.unlock();

}

//-----------------------------------------------------------------------------

void EEGSimulator::updateSineConfig(std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig>& sine)
{

  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // TODO: modify sine- or eeg config maps
  std::cout << "updateSineConfig" << std::endl;
  sine_configs_ = sine;
  lock.unlock();

}

//-----------------------------------------------------------------------------

}  // tobiss
