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

#include <boost/cstdint.hpp>
#include <set>

#include "hw_thread.h"
#include "hw_thread_builder.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

class MouseBase : public HWThread
    {
    public:
      MouseBase(ticpp::Iterator<ticpp::Element> hw);

      /**
      * @brief Destructor
      */
      virtual ~MouseBase();

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
    protected:

      boost::uint32_t vid_;
      boost::uint32_t pid_;
      boost::uint32_t usb_port_;
	  ticpp::Iterator<ticpp::Element> DS;

      static const std::string hw_vid;
      static const std::string hw_pid;
      static const std::string usb_port;

      /**
      * @brief Sets vendorId for Mousedevice.
      */
      void setVendorId(ticpp::Iterator<ticpp::Element>const &elem);

      /**
      * @brief Sets productId for Mousedevice.
      */
      void setProductId(ticpp::Iterator<ticpp::Element>const &elem);

          /**
      * @brief Sets setUsbPort for Mousedevice.
      */
      void setUsbPort(ticpp::Iterator<ticpp::Element>const &elem);

      //     void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

      virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

      virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

      virtual SampleBlock<double> getSyncData()   {return data_; }

      virtual void initMouse();
      virtual int blockKernelDriver();
      virtual int freeKernelDriver();
      virtual void acquireData();

      //-----------------------------------------------

      static std::set<boost::uint16_t> used_ids_;

      boost::uint16_t id_;

      boost::uint16_t buttons_;
      std::vector<bool> buttons_values_;

      boost::uint16_t axes_;
      std::vector<boost::int16_t> axes_values_;

      std::string name_;

      bool user_interrupt_;

      SampleBlock<double> empty_block_;

      int x_,y_;


      boost::thread*  async_acqu_thread_;
      std::vector<boost::int16_t>  raw_data_;
      int async_data_x_;
      int async_data_y_;
      int async_data_buttons_;
      bool dirty_;

  };



} // Namespace tobiss

#endif // MOUSE_H

//-----------------------------------------------------------------------------
