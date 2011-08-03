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

