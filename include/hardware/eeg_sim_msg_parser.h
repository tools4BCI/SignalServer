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
