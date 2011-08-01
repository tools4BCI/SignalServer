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
* @file eeg_simulator.h
* @brief eeg_simulator includes a class to simulate EEG signals
**/

#ifndef EEG_SIMULATOR_H
#define EEG_SIMULATOR_H

#include "hardware/hw_thread.h"
#include "hardware/artificial_signal_source.h"
#include "hardware/hw_thread_builder.h"
#include "hardware/eeg_sim_msg_parser.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#include <map>

namespace tobiss
{

//-----------------------------------------------------------------------------

/**
* @class EEGSimulator
*
* @brief A class to simulate EEG signals.
*
* This class is writtern to provide a very basic EEG simulator based on creating
* normal distributed noise. It provides the possibility to increase the amplitude of
* this noise or add an offset to it. Additionally it is possible to and sine waves
* (frequency, amplitude and phase adjustable) to the simulated EEG.
* This can be applied to every channel individually while running using network
* communication.
*
* @todo If needed: Provide the possibility to change the source-distribution during runtime.
* @todo Define and implement a configuration protocol.
* @todo Write an EEG simulator based on filtering rather than on adding just a Sine.
*/

class EEGSimulator : public ArtificialSignalSource
{
  public:

//  static const unsigned int MESSAGE_VERSION = 1;

    /**
    * @brief Constructor
    */
    EEGSimulator(boost::asio::io_service& io,
                 ticpp::Iterator<ticpp::Element> hw);
    /**
    * @brief Destructor
    */
    virtual ~EEGSimulator();

//------------------------------------------

  private:
//    /**
//    * @struct EEGConfig
//    * @brief Simple struct holding parameters to configure the EEG signal.
//    */
//    struct EEGConfig
//    {
//      public:
//        double scaling_;
//        double offset_;
//    };

//    /**
//    * @struct SineConfig
//    * @brief Simple struct holding parameters to configure the sine waves.
//    */
//    struct SineConfig
//    {
//      public:
//        double freq_;
//        double amplitude_;
//        double phase_;
//    };

//    enum MessageType
//    {
//      Invalid,
//      GetConfig,
//      Config
//    };

    /**
    * @brief Implementation of the abstract method creating the signals (sine and EEG).
    */
    virtual void generateSignal();

    /**
    * @brief Set the configuration of the SineGenerator with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw std::invalid_argument if \<channel_settings\> is defined multiple times.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);
    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    * @throw std::invalid_argument
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<channel_settings\> tag in the config file
    * @throw std::invalid_argument
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Set the port number used to remote control the EEGsimulator.
    */
    void setPort(ticpp::Iterator<ticpp::Element>const &elem);

    /**
    * @brief Set the configuration of the EEG signal from the device configuration section.
    */
    void setDeviceEEGConfig(ticpp::Iterator<ticpp::Element>const &elem);
    /**
    * @brief Set the configuration of the sine waves from the device configuration section.
    */
    void setDeviceSineConfig(ticpp::Iterator<ticpp::Element>const &elem);

    /**
    * @brief Set the configuration of individual EEG signals from the channel settings section.
    */
    void setChannelEEGConfig(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set the configuration of individual sine waves from the channel settings section.
    */
    void setChannelSineConfig(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Check if all arguments from the EEG config are OK.
    * @throws std::invalid_argument
    */
    void checkEEGConfigAttributes(ticpp::Iterator<ticpp::Element>const &elem);
    /**
    * @brief Check if all arguments from the sine config are OK.
    * @throws std::invalid_argument
    */
    void checkSineConfigAttributes(ticpp::Iterator<ticpp::Element>const &elem);

    /**
    * @brief Get an EEGConfig object out of the XML node.
    * @throws std::invalid_argument
    */
    EEGSimMsgParser::EEGConfig getEEGConfig(ticpp::Iterator<ticpp::Element>const &elem);
    /**
    * @brief Get a SineConfig object out of the XML node.
    * @throws std::invalid_argument
    */
    EEGSimMsgParser::SineConfig getSineConfig(ticpp::Iterator<ticpp::Element>const &elem);

    /**
    * @brief NOT IMPLEMENTED YET! (Handle incomming network packets asynchronously.)
    * @todo  Implement this method.
    */
    void handleAsyncRead(const boost::system::error_code& ec, std::size_t bytes_transferred );

    /**
    * @brief NOT IMPLEMENTED YET! (Accept handler handling incomming connections.)
    * @todo  Implement this method.
    */
    void acceptHandler(const boost::system::error_code& error);

//    void checkMessage();
//    MessageType getMessageType();
//    void parseConfigMessage(std::map<boost::uint16_t, EEGConfig>& eeg,
//                      std::multimap<boost::uint16_t, SineConfig>& sine);

    void updateEEGConfig(std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig>& eeg);
    void updateSineConfig(std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig>& sine);

//    std::string buildConfigString();
    void setSineConfigInMultimap(boost::uint16_t ch, EEGSimMsgParser::SineConfig config);

//------------------------------------------

  private:
    static const HWThreadBuilderTemplateRegistrator<EEGSimulator> factory_registrator_;
    boost::mt19937 twister_;
    boost::normal_distribution<> eeg_dist_;
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > eeg_gen_;

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::uint32_t port_;
    std::string     peer_ip_;
    bool connected_;


    boost::asio::streambuf message_buffer_;
    std::string str_buffer_;

//    std::map<std::string, MessageType>            msg_types_map_;

    std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig>          eeg_config_;       /// <ch_nr, EEGCfg>
    std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig>    sine_configs_;     /// <ch_nr, SineCfg>

    EEGSimMsgParser                               parser_;

    //--------------------------------------
    // Constants:

    static const std::string xml_eeg_sim_port_;
    static const std::string xml_eeg_config_;
    static const std::string xml_sine_config_;
    static const std::string xml_scaling_;
    static const std::string xml_offset_;
    static const std::string xml_frequ_;
    static const std::string xml_amplitude_;
    static const std::string xml_phase_;

};

}  // tobiss




#endif // EEG_SIMULATOR_H
