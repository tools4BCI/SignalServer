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

#ifndef G_BSAMP_BASE_H
#define G_BSAMP_BASE_H

/**
* @file gBSamp_base.h
* @brief This file includes a class providing basic, OS independent methods for the g.BSamp
**/

#include <string>

#include "hardware/hw_thread.h"

namespace tobiss
{

class gBSamp;

//-----------------------------------------------------------------------------

class gBSampBase : public HWThread
{
  protected:
    gBSampBase() {}
    virtual ~gBSampBase() {}

    virtual SampleBlock<double> getSyncData() = 0;
    virtual SampleBlock<double> getAsyncData() = 0;
    virtual void run() = 0;
    virtual void stop() = 0;

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

    /**
    * @brief Set the configuration of the device with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw ticpp::exception if \<channel_settings\> is defined multiple times.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    void setDeviceFilterSettings(ticpp::Iterator<ticpp::Element>const &elem);
    void checkFilterAttributes(ticpp::Iterator<ticpp::Element>const &elem);
    void getFilterParams(ticpp::Iterator<ticpp::Element>const &elem,\
                         unsigned int &type, bool &notch, float &f_low, float &f_high, float &sense);

    static const std::string hw_fil;   ///< xml-tag hardware: filter
    static const std::string hw_fil_type;   ///< xml-tag hardware: filter type
    static const std::string hw_fil_order;   ///< xml-tag hardware: filter order
    static const std::string hw_fil_low;   ///< xml-tag hardware: filter lower cutoff freq.
    static const std::string hw_fil_high;   ///< xml-tag hardware: filter upper cutoff freq.
    static const std::string hw_fil_sense;   ///< xml-tag hardware: mV for gBSamp
    static const std::string hw_notch;   ///< xml-tag hardware: notch_filter
    static const std::string hw_notch_center;    ///< xml-tag hardware: notch center freq.
};

//-----------------------------------------------------------------------------

}  // Namespace tobiss


#endif //G_BSAMP_BASE_H

