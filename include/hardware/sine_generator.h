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
* @file sine_generator.h
*
* @brief sine_generator is a dummy hardware device for simulation purposes.
*
* sine_generator can be used the same way like a normal data acquisition device with reduced
* configuration possibilities.
* It produces a sine with a frequency of 1Hz on a selectable amount of channels in a buffered
* or unbuffered way. Generating a sine on multiple channels, an incrementing, channel dependent
* phase value is added to the sine to facilitate distinction.
* The sine generator can be operated in synchronous an asynchronous manner.
* It uses boost::asio and boost::thread to ensure proper timing.
*
**/

#ifndef SINEGENERATOR_H
#define SINEGENERATOR_H

#include <vector>
#include <map>

#include <boost/cstdint.hpp>

#include "hw_thread.h"
#include "hardware/hw_thread_builder.h"
#include "hardware/artificial_signal_source.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class SineGenerator
*
* @brief A dummy hardware device for simulation and testing purposes.
*
* SineGenerator provides a fully functional and equivalent dummy hardware device for testing
* purposes without any external data acquisition device.
* It supports synchronous and asynchronous data acquisition. Data can be produced with freely
* selectable frequencies in a block-based or non-block based way.
* To achive a consistent access to hardware devices, this class is a derived class.
*
* @bug Sometimes the SineGenerator crashes in Windows, especially using high sampling rates.
*/
class SineGenerator : public ArtificialSignalSource
{
  public:
     /**
    * @brief Constructor for initialization with an XML object
    * @param[in] io_service boost::asio::io_service to create a proper timing
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    */
    SineGenerator(boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~SineGenerator();

//-----------------------------------------------

  private:
    /**
    * @brief Set the configuration of the SineGenerator with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw ticpp::exception if \<channel_settings\> is defined multiple times.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);
    /**
    * @brief Generate a sine curve and store it into the objects data vector.
    *
    * This method is used to gnerate a sine with 1Hz on eligible channels wether in
    * block oriented mode or not. Settings have to be given by initialization of the
    * object.
    * In non block oriented mode (buffersize = 1) data will be directly stored in the
    * "data" SampleBlock. Using block oriented data creation (buffersize > 1) data is
    * stored in the buffer first and if the desired number of blocks to store is achieved,
    * the buffer is copied into "data".
    * If new data is set, "samples_available" is set to true.
    */
    void genSine();

    /**
    * @brief Calls genSine()
    */
    void generateSignal()
    {
      genSine();
    }

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

//-----------------------------------------------

  private:
    static const HWThreadBuilderTemplateRegistrator<SineGenerator> factory_registrator_;
};

} // Namespace tobiss

#endif // SINEGENERATOR_H

//-----------------------------------------------------------------------------
