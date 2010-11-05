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

#ifndef G_MOBILAB_H
#define G_MOBILAB_H

/**
* @file g_mobilab.h
*
**/

#include <vector>
#include <string>

#include <boost/thread.hpp>

#include "serial_port_base.h"
#include "hardware/hw_thread.h"
#include "hardware/hw_thread_builder.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class GMobilab
*
* @brief
*/
class GMobilab : private SerialPortBase, public HWThread
{
  public:
    GMobilab(boost::asio::io_service& io, XMLParser& parser,
             ticpp::Iterator<ticpp::Element> hw);
    virtual ~GMobilab()
    {
      async_acqu_thread_->join();
      close();
      if(async_acqu_thread_)
        delete async_acqu_thread_;
    }


    virtual SampleBlock<double> getSyncData();
    virtual SampleBlock<double> getAsyncData();

    /**
    * @brief Method to start data acquisition.
    */
    virtual void run();
    /**
    * @brief Method to stop data acquisition.
    */
    virtual void stop();

  private:
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    void setScalingValues();
    unsigned char getChannelCode();
    void checkNrOfChannels();

    void acquireData();

    enum device_types_ { EEG, MULTI };

  private:
    device_types_ type_;
    std::map<unsigned int, unsigned char> channel_coding_;
    std::vector<double> scaling_factors_;
    std::vector<boost::int16_t>  raw_data_;
    std::vector<double>  samples_;

    boost::thread*  async_acqu_thread_;

    static const HWThreadBuilderTemplateRegistrator<GMobilab> factory_registrator_;

    //--------------------------------------
    // Constants:

    static const std::string hw_mobilab_serial_port_;
    static const std::string hw_mobilab_type_;
    static const std::string hw_mobilab_eeg_;
    static const std::string hw_mobilab_multi_;

};

//-----------------------------------------------------------------------------

}  // Namespace tobiss


#endif // G_MOBILAB_H
