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
    gBSampBase();
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

    static const std::string hw_jumper_;   ///< xml-tag hardware: filter
    static const std::string hw_jumper_hp_;   ///< xml-tag hardware: filter lower cutoff freq.
    static const std::string hw_jumper_lp_;   ///< xml-tag hardware: filter upper cutoff freq.
    static const std::string hw_jumper_sense_;   ///< xml-tag hardware: mV for gBSamp
    static const std::string hw_jumper_notch_;   ///< xml-tag hardware: notch_filter

    static const std::string hw_daq_mode_;    ///< xml-tag hardware: acquisition mode
    static const std::string hw_device_id_;    ///< xml-tag hardware: device name (e.g. Dev1 or comedi0)

    enum daq_mode_type
      { RSE = 0, NRSE, diff };
    daq_mode_type                        daq_mode_;
    std::string                          device_id_;

    std::map<boost::uint32_t, float>     global_scaling_factors_;

    std::vector<float>                   scaling_factors_;



  private:

    void setDeviceJumperSettings(ticpp::Iterator<ticpp::Element>const &elem);
    void setChannelJumperSettings(ticpp::Iterator<ticpp::Element>const &father);

    void checkJumperAttributes(ticpp::Iterator<ticpp::Element>const &elem);

    void getJumperParams(ticpp::Iterator<ticpp::Element>const &elem,
                         unsigned int &type, bool &notch, float &highpass,
                         float &lowpass, float &sense);

    void setAcquisitionMode(ticpp::Iterator<ticpp::Element>const &elem);

    void setDeviceName(ticpp::Iterator<ticpp::Element>const &elem);

    void setGlobalScalingValues();

    void throwXMLErrorTagNotGiven(std::string& tag_name);
    void throwXMLErrorWrongValue(const std::string& tag_name,const  std::string& attr_name, std::string type, float given, float cor1, float cor2);

    void checkEEGJumperValues(float highpass, float lowpass, float sense);
    void checkEOGJumperValues(float highpass, float lowpass, float sense);
    void checkEMGJumperValues(float highpass, float lowpass, float sense);
    void checkECGJumperValues(float highpass, float lowpass, float sense);

  private:
    std::map<std::string, daq_mode_type> daq_modes_map_;
};

//-----------------------------------------------------------------------------

}  // Namespace tobiss


#endif //G_BSAMP_BASE_H

