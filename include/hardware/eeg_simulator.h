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

#ifndef EEG_SIMULATOR_H
#define EEG_SIMULATOR_H

#include "hardware/hw_thread.h"
#include "hardware/artificial_signal_source.h"
#include "hardware/hw_thread_builder.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#include <map>

namespace tobiss
{

//-----------------------------------------------------------------------------
class EEGSimulator : public ArtificialSignalSource
{
  public:
    EEGSimulator(boost::asio::io_service& io,
                 ticpp::Iterator<ticpp::Element> hw);
    virtual ~EEGSimulator();

//------------------------------------------

  private:
    /**
    * @brief TODO
    */
    void generateSignal();

    /**
    * @brief Set the configuration of the SineGenerator with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw ticpp::exception if \<channel_settings\> is defined multiple times.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);
    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<channel_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);




    void handleAsyncRead(const boost::system::error_code& ec, std::size_t bytes_transferred );
    void acceptHandler(const boost::system::error_code& error);

//------------------------------------------

  private:
    static const HWThreadBuilderTemplateRegistrator<EEGSimulator> factory_registrator_;
    boost::mt19937 twister_;
    boost::normal_distribution<> eeg_dist_;
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > eeg_gen_;

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::uint32_t port_;
    bool connected_;


    std::vector<boost::uint8_t> message_buffer_;

    class EEGCfg
    {
      public:
        double amplitude_;
        double offset_;
    };

    class SineCfg
    {
      public:
        double freq_;
        double amplitude_;
        double phase_;
    };

    std::multimap<boost::uint16_t, EEGCfg>  eeg_config_;       /// <ch_nr, EEGCfg>
    std::multimap<boost::uint16_t, SineCfg> sine_configs_;     /// <ch_nr, SineCfg>
};

}  // tobiss




#endif // EEG_SIMULATOR_H
