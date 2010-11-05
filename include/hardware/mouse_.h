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
* @file mouse.h
*
* @brief
*
**/

#ifndef MOUSE_H
#define MOUSE_H

#ifdef WIN32
    #pragma comment(lib, "SDL.lib")
    #pragma comment(lib, "SDLmain.lib")
#endif

#include <boost/cstdint.hpp>
#include <set>
#include <SDL/SDL.h>
#include <cmath>

#include "hw_thread.h"
#include "hw_thread_builder.h"
#include "extern\lib\libusb\win\usb.h"

using namespace std;

namespace tobiss
{
//-----------------------------------------------------------------------------

class Mouse : public HWThread
    {
    public:
      Mouse(ticpp::Iterator<ticpp::Element> hw);

      /**
      * @brief Destructor
      */
      virtual ~Mouse();

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

  //     void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

      virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

      virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

      virtual SampleBlock<double> getSyncData()   {return data_; }

      void initMouse();
      int blockKernelDriver();
      int freeKernelDriver();
	  void acquireData();

      //-----------------------------------------------

    private:
    static set<boost::uint16_t> used_ids_;

      boost::uint16_t id_;

      boost::uint16_t buttons_;
      vector<bool> buttons_values_;

      boost::uint16_t axes_;
      vector<boost::int16_t> axes_values_;

      string name_;

      bool user_interrupt_;

      SampleBlock<double> empty_block_;

      int x_,y_;
      struct usb_device **devs_; //pointer to pointer of device, used to retrieve a list of devices
      usb_dev_handle *dev_handle_; //a device handle
	  char hw_id_[28];
	  //libusb_context *ctx_; //a libusb session

	  boost::thread*  async_acqu_thread_;
      std::vector<boost::int16_t>  raw_data_;
	  char async_data_[10];
	  bool dirty_;

	  static const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> FACTORY_REGISTRATOR_;
  };



} // Namespace tobiss

#endif // MOUSE_H

//-----------------------------------------------------------------------------
