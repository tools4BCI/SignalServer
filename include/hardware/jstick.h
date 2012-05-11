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
* @file jstick.h
* @brief This file includes a class handling generic joysticks using SDL.
**/

#ifndef JSTICK_H
#define JSTICK_H

//#ifdef WIN32
//    #pragma comment(lib, "SDL.lib")
//#endif

#include <boost/cstdint.hpp>

#include <set>

#include "hw_thread.h"
#include "hw_thread_builder.h"

struct _SDL_Joystick;

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

    _SDL_Joystick* joy_;   //FIXME: should be of type SDL_Joystick  ... problems with includes
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
