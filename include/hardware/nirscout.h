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

#ifndef NIRSCOUT_H
#define NIRSCOUT_H

/**
* @file nirscout.h
* @brief This file includes a class to gain access to the NIRScout.
**/

#include "hardware/hw_thread.h"
#include "hardware/hw_thread_builder.h"

#include "NIRScout/TomographyAPI.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

class NIRScout : public HWThread
{
  public:
    NIRScout(ticpp::Iterator<ticpp::Element> hw);
    ~NIRScout();

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
    /**
    * @brief Set configuration defined in the device settings xml section.
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set configuration defined in the channel settings xml section.
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set the NIRScout config structure.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    std::string getErrorMsg();

  private:
    boost::uint32_t                  port_;
    std::string                      target_ip_;

    char*                            string_buffer_;

    float*                           raw_data_;
    double*                          timestamps_;
    char*                            timing_bytes_;

    int                              sources_;
    int                              detectors_;
    int                              wavelengths_;

    int                              buffer_size_;
    int                              frame_count_;

    int                              error_code_;

    enum NameCodes
    {
      sources = 0, detectors, wavelengths
    };

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<NIRScout> factory_registrator_;


};


}  //tobiss


#endif // NIRSCOUT_H
