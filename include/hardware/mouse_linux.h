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
* @file mouse_linux.h
*
* @brief
*
**/


#ifndef _WIN32

#ifndef MOUSE_LINUX_H
#define MOUSE_LINUX_H

#include <usb.h>
#include "mouse.h"
//#include "extern/include/libusb/libusb.h"
#include <libusb-1.0/libusb.h>

namespace tobiss
{

/**
* @class Mouse
*
* @brief Class to connected mouse devices.
*/
class Mouse : public MouseBase
{
  public:
    /**
    * @brief Constructor
    */
    Mouse(ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~Mouse();

  //-----------------------------------------------
  protected:
    /**
    * @brief Method to detach kernel driver from the system and open connection to the mouse device.
    * @return ret<int>
    */
    virtual int blockKernelDriver();
    /**
    * @brief Method to attach kernel driver to the system and close connection to the mouse device.
    * @return ret<int>
    */
    virtual int freeKernelDriver();
    /**
    * @brief Method to acquire data from the mouse device.
    */
    virtual void acquireData();

    libusb_device **devs_; //pointer to pointer of device, used to retrieve a list of devices
    libusb_device_handle *dev_handle_; //a device handle
    libusb_context *ctx_; //a libusb session


    static const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> FACTORY_REGISTRATOR_;
};

  //-----------------------------------------------------------------------------

}
#endif // MOUSE_LINUX_H
#endif // __linux__
