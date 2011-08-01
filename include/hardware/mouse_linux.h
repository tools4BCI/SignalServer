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
    ~Mouse();

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
