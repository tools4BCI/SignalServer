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

#ifndef _WIN32

#include "hardware/mouse_linux.h"
#include <string>

namespace tobiss
{

const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> Mouse::FACTORY_REGISTRATOR_ ("mouse");

Mouse::Mouse(ticpp::Iterator<ticpp::Element> hw)
: MouseBase(hw)
{
  int ret = blockKernelDriver();
  if(ret)
    throw(std::runtime_error("MouseBase::initMouse -- Mouse device could not be connected (check rights)!"));

  struct libusb_device_descriptor desc;


  struct libusb_device* dev = libusb_get_device(dev_handle_);
  if(dev == 0)
    throw(std::runtime_error("MouseBase::initMouse -- Failed to get device pointer!"));

  int r = libusb_get_device_descriptor( dev, &desc);
  if(r < 0)
    throw(std::runtime_error("MouseBase::initMouse -- Failed to get device descriptor!"));

  std::string name;
  unsigned char str[256];

  ret = 0;
  ret = libusb_get_string_descriptor_ascii(dev_handle_, desc.iManufacturer, str, sizeof(str) );
  if(ret)
  {
    name += " -- ";
    name.append( reinterpret_cast<char*>(str));
  }
  ret = libusb_get_string_descriptor_ascii(dev_handle_, desc.iProduct, str, sizeof(str) );
  if(ret)
  {
    name += ", ";
    name.append( reinterpret_cast<char*>(str));
  }

  name += " (vid/pid: " + boost::lexical_cast<std::string>(vid_);
  name += "/"  + boost::lexical_cast<std::string>(pid_) + ")";


//  cout << " --> Mouse ID: " << id_ << ",  Name: " << name_;
//  cout<<", vid: "<<vid_<<", pid: "<<pid_<<endl;

  setType("Mouse" + name);
}

//-----------------------------------------------------------------------------

Mouse::~Mouse()
{
  running_ = false;
  async_acqu_thread_->join();
  if(async_acqu_thread_)
    delete async_acqu_thread_;
  freeKernelDriver();
}

//-----------------------------------------------------------------------------

int Mouse::blockKernelDriver()
{
  int ret;
  // setup interface
  ret = libusb_init(&ctx_);
  if(ret < 0)
    return -1;

  // create an interface to the right usb device
  dev_handle_ = libusb_open_device_with_vid_pid(ctx_, vid_, pid_);
  if(dev_handle_ == NULL)
    return -2;

  if(libusb_kernel_driver_active(dev_handle_, 0) == 1)
    libusb_detach_kernel_driver(dev_handle_, 0);

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
    return -1;
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
    bool unchanged = false;
    boost::unique_lock<boost::shared_mutex> lock(rw_);
    int actual_length;
    unsigned char async_data_[10];
    int r = libusb_interrupt_transfer(dev_handle_, usb_port_, async_data_,
                                      sizeof(async_data_), &actual_length, 100);

    if(r == LIBUSB_ERROR_TIMEOUT)
    {
      //cout<<"   Mouse: Timeout!"<<endl;
      unchanged = true;
    }
    else if (r == LIBUSB_ERROR_NO_DEVICE)
    {
      lock.unlock();
      throw(std::runtime_error("Mouse::acquireData -- Mouse device could not be read! Check usb-port!"));
    }
    else if(r<0)
    {
      lock.unlock();
      throw(std::runtime_error("Mouse::acquireData -- Mouse device could not be read! Problem with libusb!"));
    }
    if(!unchanged)
    {
      async_data_buttons_ = static_cast<int>(static_cast<char>(async_data_[0]));
      async_data_x_ = static_cast<int>(static_cast<char>(async_data_[1]));
      async_data_y_ = static_cast<int>(static_cast<char>(async_data_[2]));
    }
    lock.unlock();
  }
}

//-----------------------------------------------------------------------------


}
#endif //__linux__
