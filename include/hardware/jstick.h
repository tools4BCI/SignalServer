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
* @file jstick.h
* @brief This file includes a class handling generic joysticks using SDL.
**/

#ifndef JSTICK_H
#define JSTICK_H

#ifdef WIN32
    #pragma comment(lib, "SDL.lib")
    #pragma comment(lib, "SDLmain.lib")
#endif

#include <boost/cstdint.hpp>

#include <set>

#include "hw_thread.h"
#include "hw_thread_builder.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class JStick
* @brief A class using SDL to access generic joysticks.
* @todo Get rid of using void* for working with SDL.
*/
class JStick : public HWThread
{
  public:
    /**
    * @brief Constructor
    */
    JStick (ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~JStick();

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

    //-----------------------------------------------
  private:
    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);
    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Method to achieve synchronous data acquisition.
    * @todo Implement this method.
    */
    virtual SampleBlock<double> getSyncData()   {return data_; }

    /**
    * @brief Initialize the Joystick.
    */
    void initJoystick();

    //-----------------------------------------------

  private:
    static std::set<boost::uint16_t> used_ids_;        ///<  every Joystick has a unique ID

    void* joy_;   //FIXME: should be of type SDL_Joystick  ... problems with includes
    boost::uint16_t id_;

    boost::uint16_t buttons_;
    std::vector<bool> buttons_values_;

    boost::uint16_t axes_;
    std::vector<boost::int16_t> axes_values_;

    boost::uint16_t balls_;
    std::vector< std::pair<int,int> > balls_values_;

    std::string name_;

    SampleBlock<double> empty_block_;

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<JStick> FACTORY_REGISTRATOR_;

};

} // Namespace tobiss

#endif // JSTICK_H

//-----------------------------------------------------------------------------
