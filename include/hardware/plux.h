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
* @file plux.h
* @brief This file includes a class handling BioPlux Devices.
**/

#ifndef PLUX_H
#define PLUX_H

#include "hw_thread.h"
#include "hw_thread_builder.h"

#include "BioPlux.h"

#define PLUX_TRY try  
#define PLUX_CATCH catch( BP::Err err ) { rethrowPluxException( err ); }

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class Plux
* @brief A class to access BioPlux Devices.
* @todo Everything.
*/
class Plux : public HWThread
{
public:

	 /**
    * @brief Constructor
    */
    Plux (ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~Plux();
	
	/**
    * @brief Method to achieve synchronous data acquisition (method is blocking).
    * @return SampleBlock<double>
    *
    * This method returns after new data has been acquired, it is operating in blocking mode.
    * It is called only from the master device.
    */
    virtual SampleBlock<double> getSyncData();

	/**
    * @brief Method to achieve asynchronous data acquisition (method is non-blocking).
    * @return SampleBlock<double>
    *
    * This method return immediately after calling with a copy of the data stored in the device's
    * buffer.
    * It is called from all slave devices.
    */
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
    * @brief Set the configuration of the Plux Device with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw std::invalid_argument if \<channel_settings\> is defined multiple times.
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
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Automatically selects an available PLUX device.
    * @return std::string MAC adress of available device
    * @todo What if there is more than one device?.
    */
    static std::string findDevice( );

    /**
    * @brief Throws a standard exception with information obtained from a BioPlux excaption.
    */
    static void rethrowPluxException(  BP::Err &err );

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<Plux> FACTORY_REGISTRATOR_;


    std::map<std::string, std::string> m_;	/// Attributes map -- to be renamed
    BP::Device *device_;

    std::string devstr_;
    std::string devinfo_;
};

} // Namespace tobiss

#endif
