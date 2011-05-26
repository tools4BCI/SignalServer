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
* @file eeg_sim_msg_parser.h
**/

#ifndef EEG_SIMULATOR_MSGMPARSER
#define EEG_SIMULATOR_MSGMPARSER

#include <map>
#include <string>
#include <boost/cstdint.hpp>

namespace tobiss
{

//-----------------------------------------------------------------------------

/**
* @class EEGSimMsgParser
*/

class EEGSimMsgParser
{
  public:

  static const unsigned int MESSAGE_VERSION;

    /**
    * @brief Constructor
    */
    EEGSimMsgParser(std::string& str_buffer);
    /**
    * @brief Destructor
    */
    virtual ~EEGSimMsgParser();

//------------------------------------------

  public:
    /**
    * @struct EEGConfig
    * @brief Simple struct holding parameters to configure the EEG signal.
    */
    struct EEGConfig
    {
      public:
        double scaling_;
        double offset_;
    };

    /**
    * @struct SineConfig
    * @brief Simple struct holding parameters to configure the sine waves.
    */
    struct SineConfig
    {
      public:
        double freq_;
        double amplitude_;
        double phase_;
    };

    enum MessageType
    {
      Invalid,
      GetConfig,
      Config
    };

    void parseMessage();
    MessageType getMessageType()
    {
      return type_;
    }

    void getConfigs(std::map<boost::uint16_t, EEGConfig>& eeg,
                    std::multimap<boost::uint16_t, SineConfig>& sine);
    std::string buildConfigMsgString(std::map<boost::uint16_t, EEGConfig>& eeg,
                                  std::multimap<boost::uint16_t, SineConfig>& sine);

    std::string getOKMsg();
    std::string getErrorMsg();

  private:
    MessageType parseMessageType();
    void checkMessage();
    void parseConfigMessage();

//------------------------------------------

  private:
    MessageType                                   type_;
    std::string&                                  str_buffer_;
    std::map<std::string, MessageType>            msg_types_map_;
    std::map<boost::uint16_t, EEGConfig>          eeg_config_;       /// <ch_nr, EEGCfg>
    std::multimap<boost::uint16_t, SineConfig>    sine_configs_;     /// <ch_nr, SineCfg>
};

}  // tobiss




#endif // EEG_SIMULATOR_MSGMPARSER
