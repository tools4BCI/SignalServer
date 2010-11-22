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

#include "hardware/mouse_linux.h"

namespace tobiss
{

    const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> Mouse::FACTORY_REGISTRATOR_ ("mouse");

    Mouse::Mouse(ticpp::Iterator<ticpp::Element> hw)
     : MouseBase(hw)
    {
        int ret = blockKernelDriver();
        if(ret)
          throw(std::runtime_error("MouseBase::initMouse -- Mouse device could not be connected!"));
    }

//
    Mouse::~Mouse()
    {
        running_ = false;
        async_acqu_thread_->join();
        if(async_acqu_thread_)
          delete async_acqu_thread_;
        freeKernelDriver();
    }

    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------

    int Mouse::blockKernelDriver()
    {
        int ret;
        ret = libusb_init(&ctx_);
        if(ret < 0) {
          return -1;
        }
        dev_handle_ = libusb_open_device_with_vid_pid(ctx_, vid_, pid_);
        if(dev_handle_ == NULL)
            return -2;

        if(libusb_kernel_driver_active(dev_handle_, 0) == 1) {
            libusb_detach_kernel_driver(dev_handle_, 0);
        }
        ret = libusb_claim_interface(dev_handle_, 0);
        if(ret)
            return -3;
        return 0;
    }

    //-----------------------------------------------------------------------------

    int Mouse::freeKernelDriver()
    {
        int ret;
        ret = libusb_release_interface(dev_handle_, 0);
        if(ret)
        ret = libusb_attach_kernel_driver(dev_handle_, 0);
        if(ret)
            return -2;
        libusb_close(dev_handle_);
        libusb_exit(ctx_);
        return 0;
    }

    //-----------------------------------------------------------------------------



    void Mouse::acquireData()
    {
      while(running_)
      {
        boost::unique_lock<boost::shared_mutex> lock(rw_);
        int actual_length;
        unsigned char async_data_[10];
        int r = libusb_interrupt_transfer(dev_handle_, usb_port_, async_data_, sizeof(async_data_), &actual_length, 0);
        if(r)
          throw(std::runtime_error("Mouse::acquireData -- Mouse device could not be connected! Check usb-port!"));
        async_data_buttons_ = (int)(char)async_data_[0];
        async_data_x_ = (int)(char)async_data_[1];
        async_data_y_ = (int)(char)async_data_[2];
        lock.unlock();
      }
    }

//-----------------------------------------------------------------------------


}
