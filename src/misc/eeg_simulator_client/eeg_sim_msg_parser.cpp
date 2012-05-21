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

#include "eeg_sim_msg_parser.h"

#include <stdexcept>
#include <iostream>

#include <boost/lexical_cast.hpp>

using std::string;

namespace tobiss
{

static const std::string EEGSIM_MSG_STRING("eegsimconfig");
static const std::string EEGSIM_EEG_STRING("eeg");
static const std::string EEGSIM_SINE_STRING("sine");

static const std::string EEGSIM_MSG_CMD_DELIMITER(":");
static const std::string EEGSIM_MSG_PARAM_DELIMITER("/");
static const std::string EEGSIM_MSG_CHANNEL_DELIMITER(",");

static const std::string EEGSIM_MSG_GETCONFIG("getconfig");
static const std::string EEGSIM_MSG_CONFIG("config");
static const std::string EEGSIM_MSG_OK("ok");
static const std::string EEGSIM_MSG_ERROR("error");

const unsigned int EEGSimMsgParser::MESSAGE_VERSION = 1;

//-----------------------------------------------------------------------------

EEGSimMsgParser::EEGSimMsgParser(std::string& str_buffer)
  : type_(Invalid), str_buffer_(str_buffer)
{
  msg_types_map_.insert(std::make_pair( EEGSIM_MSG_GETCONFIG, GetConfig ) );
  msg_types_map_.insert(std::make_pair( EEGSIM_MSG_CONFIG, Config ) );
}

//-----------------------------------------------------------------------------

EEGSimMsgParser::~EEGSimMsgParser()
{

}

//-----------------------------------------------------------------------------

void EEGSimMsgParser::parseMessage()
{
  type_ = Invalid;
  checkMessage();
  type_ = parseMessageType();

  if(type_ == Config)
    parseConfigMessage();
  else
    return;
}

//-----------------------------------------------------------------------------

void EEGSimMsgParser::checkMessage()
{
  size_t pos = str_buffer_.find(EEGSIM_MSG_STRING);

  if(pos == std::string::npos || pos !=0)
    throw(std::runtime_error("Error -- Message is not an EEG Simulator message!"));

  str_buffer_.erase(0, EEGSIM_MSG_STRING.size());

  std::string version(EEGSIM_MSG_CMD_DELIMITER +
                      boost::lexical_cast<string>(EEGSimMsgParser::MESSAGE_VERSION));
  pos = str_buffer_.find( version );

  if(pos == std::string::npos || pos !=0)
    throw(std::runtime_error("Error -- Message version could not be determined!"));

  str_buffer_.erase(0, version.size());
}

//-----------------------------------------------------------------------------

EEGSimMsgParser::MessageType EEGSimMsgParser::parseMessageType()
{
  size_t pos = str_buffer_.find(EEGSIM_MSG_CMD_DELIMITER);

  if(pos == std::string::npos || pos !=0)
    throw(std::runtime_error("Error -- Message is not a valid EEG Simulator message!"));

  pos = str_buffer_.find(EEGSIM_MSG_CMD_DELIMITER,1);

  std::string type_str = str_buffer_.substr(1,pos-1);

//  std::cout << "  *** TYPE: " <<  type_str << std::endl;

//  std::cout << "  *** BUFFER: " <<  str_buffer_ << std::endl;

  if(pos != std::string::npos)
    str_buffer_.erase(0,pos+1);
  else
    str_buffer_.clear();

//  std::cout << "  *** BUFFER: " <<  str_buffer_ << std::endl;

  std::map<std::string, MessageType>::iterator it(msg_types_map_.find(type_str));

  if(it == msg_types_map_.end())
    return(Invalid);

  return( it->second );
}

//-----------------------------------------------------------------------------

// EEGSimConfig:1:GetConfig\n

// EEGSimConfig:1:Sine:chX/freq/amp/phase,chY/freq/amp/phase,chA-B/freq/amp/phase:
//              EEG:chX/scale/offset,chY/scale/offset,chA-B/scale/offset\n

void EEGSimMsgParser::parseConfigMessage()
{
  eeg_config_.clear();
  sine_configs_.clear();

  size_t eeg_pos = str_buffer_.find(EEGSIM_EEG_STRING);

  if(eeg_pos != std::string::npos)
  {

    size_t end_pos = str_buffer_.find(EEGSIM_MSG_CMD_DELIMITER, eeg_pos + EEGSIM_EEG_STRING.size() +1);

    std::string eeg_cfg = str_buffer_.substr(eeg_pos + EEGSIM_EEG_STRING.size() +1,
                                              end_pos -eeg_pos -  EEGSIM_EEG_STRING.size() -1);

    str_buffer_.erase(eeg_pos,end_pos);

    EEGConfig eeg_config;
    do
    {
      size_t par_pos = eeg_cfg.find(EEGSIM_MSG_PARAM_DELIMITER);
      boost::uint16_t ch = boost::lexical_cast<boost::uint16_t>( eeg_cfg.substr(0, par_pos) );
      eeg_cfg.erase(0,par_pos+1);

      par_pos = eeg_cfg.find(EEGSIM_MSG_PARAM_DELIMITER);
      double scale  = boost::lexical_cast<double>(eeg_cfg.substr(0, par_pos));
      eeg_cfg.erase(0,par_pos+1);

      par_pos = eeg_cfg.find(EEGSIM_MSG_CHANNEL_DELIMITER);
      double offset = boost::lexical_cast<double>(eeg_cfg.substr(0, par_pos));
      eeg_cfg.erase(0,par_pos+1);

      if(par_pos == std::string::npos)
        eeg_cfg.clear();
      //      std::cout << "ch: " << ch  << ", scale: " << scale << ", offset: " << offset << std::endl;

      eeg_config.offset_ = offset;
      eeg_config.scaling_ = scale;
      eeg_config_.insert( std::make_pair(ch, eeg_config) );

    }
    while(eeg_cfg.size() != 0);

  }

  size_t sine_pos = str_buffer_.find(EEGSIM_SINE_STRING);

  if(sine_pos != std::string::npos)
  {

    size_t end_pos = str_buffer_.find(EEGSIM_MSG_CMD_DELIMITER, sine_pos + EEGSIM_SINE_STRING.size() +1);

    std::string sine_cfg = str_buffer_.substr(sine_pos + EEGSIM_SINE_STRING.size() +1,
                                              end_pos -sine_pos -  EEGSIM_SINE_STRING.size() -1);

    str_buffer_.erase(sine_pos,end_pos);

    SineConfig sine_config;
    do
    {
      size_t par_pos = sine_cfg.find(EEGSIM_MSG_PARAM_DELIMITER);
      boost::uint16_t ch = boost::lexical_cast<boost::uint16_t>( sine_cfg.substr(0, par_pos) );
      sine_cfg.erase(0,par_pos+1);

      par_pos = sine_cfg.find(EEGSIM_MSG_PARAM_DELIMITER);
      double freq  = boost::lexical_cast<double>(sine_cfg.substr(0, par_pos));
      sine_cfg.erase(0,par_pos+1);

      par_pos = sine_cfg.find(EEGSIM_MSG_PARAM_DELIMITER);
      double ampl  = boost::lexical_cast<double>(sine_cfg.substr(0, par_pos));
      sine_cfg.erase(0,par_pos+1);

      par_pos = sine_cfg.find(EEGSIM_MSG_CHANNEL_DELIMITER);
      double phase = boost::lexical_cast<double>(sine_cfg.substr(0, par_pos));
      sine_cfg.erase(0,par_pos+1);

      if(par_pos == std::string::npos)
        sine_cfg.clear();
      //      std::cout << "ch: " << ch << ", freq: " << freq << ", ampl: " << ampl << ", phase: " << phase << std::endl;

      sine_config.freq_ = freq;
      sine_config.amplitude_ = ampl;
      sine_config.phase_ = phase;
      sine_configs_.insert( std::make_pair(ch, sine_config) );
    }
    while(sine_cfg.size() != 0);

  }

}

//-----------------------------------------------------------------------------

std::string EEGSimMsgParser::buildConfigMsgString(std::map<boost::uint16_t, EEGConfig>& eeg,
                                               std::multimap<boost::uint16_t, SineConfig>& sine)
{
  std::string config_str(EEGSIM_MSG_STRING + EEGSIM_MSG_CMD_DELIMITER +
                         boost::lexical_cast<string>(MESSAGE_VERSION) +
                         EEGSIM_MSG_CMD_DELIMITER + EEGSIM_MSG_CONFIG +
                         EEGSIM_MSG_CMD_DELIMITER);

  config_str += EEGSIM_EEG_STRING+ EEGSIM_MSG_CMD_DELIMITER;

  for(std::map<boost::uint16_t, EEGConfig>::iterator it(eeg.begin());
      it != eeg.end(); it++)
  {
    config_str += boost::lexical_cast<string>(it->first);
    config_str += EEGSIM_MSG_PARAM_DELIMITER + boost::lexical_cast<string>(it->second.scaling_);
    config_str += EEGSIM_MSG_PARAM_DELIMITER + boost::lexical_cast<string>(it->second.offset_);
    config_str += EEGSIM_MSG_CHANNEL_DELIMITER;
  }
  config_str.erase(--config_str.end(), config_str.end());

  config_str += EEGSIM_MSG_CMD_DELIMITER + EEGSIM_SINE_STRING+ EEGSIM_MSG_CMD_DELIMITER;

  for(std::multimap<boost::uint16_t, SineConfig>::iterator it2(sine.begin());
      it2 != sine.end(); it2++)
  {
    config_str += boost::lexical_cast<string>(it2->first);
    config_str += EEGSIM_MSG_PARAM_DELIMITER + boost::lexical_cast<string>(it2->second.freq_);
    config_str += EEGSIM_MSG_PARAM_DELIMITER + boost::lexical_cast<string>(it2->second.amplitude_);
    config_str += EEGSIM_MSG_PARAM_DELIMITER + boost::lexical_cast<string>(it2->second.phase_);
    config_str += EEGSIM_MSG_CHANNEL_DELIMITER;
  }
  config_str.erase(--config_str.end(), config_str.end());

  return(config_str + "\n");
}

//-----------------------------------------------------------------------------

void EEGSimMsgParser::getConfigs(std::map<boost::uint16_t, EEGConfig>& eeg,
                std::multimap<boost::uint16_t, SineConfig>& sine)
{
  eeg = eeg_config_;
  sine = sine_configs_;
}

//-----------------------------------------------------------------------------

std::string EEGSimMsgParser::getOKMsg()
{
  return(EEGSIM_MSG_STRING + EEGSIM_MSG_CMD_DELIMITER +
         boost::lexical_cast<string>(MESSAGE_VERSION) +
         EEGSIM_MSG_CMD_DELIMITER + EEGSIM_MSG_OK);
}

//-----------------------------------------------------------------------------

std::string EEGSimMsgParser::getErrorMsg()
{
  return(EEGSIM_MSG_STRING + EEGSIM_MSG_CMD_DELIMITER +
         boost::lexical_cast<string>(MESSAGE_VERSION) +
         EEGSIM_MSG_CMD_DELIMITER + EEGSIM_MSG_ERROR);
}

//-----------------------------------------------------------------------------

}  //tobiss
