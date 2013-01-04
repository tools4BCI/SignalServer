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
#include <boost/current_function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

//#include <usb.h>
#include <libusb.h>
//#include "extern/include/libusb/libusb.h"

#include <X11/Xlib.h>
#include<X11/extensions/Xrandr.h>

#define STR_CHAR_ARRAY_SIZE 256
#define ACYNC_DATA_RECV_SIZE 16

using std::cout;
using std::endl;

namespace tobiss
{

const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> Mouse::FACTORY_REGISTRATOR_ ("mouse");

//-----------------------------------------------------------------------------

Mouse::Mouse(ticpp::Iterator<ticpp::Element> hw)
: MouseBase(hw),
  devs_(0), dev_handle_(0), ctx_(0), dsp_(0), event_(0),
  screen_number_(-1), screen_width_(-1), screen_height_(-1)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  std::string name;

  if(detach_from_os_)
  {
	name = "Mouse";
  
    int ret = blockKernelDriver();
    if(ret)
      throw(std::runtime_error("MouseBase::initMouse -- Mouse device could not be connected (check rights)!"));

    struct libusb_device_descriptor desc;

    struct libusb_device* dev = libusb_get_device(dev_handle_);
    if(dev == 0)
      throw(std::runtime_error("MouseBase::initMouse -- Failed to get device pointer!"));

    ret = libusb_get_device_descriptor( dev, &desc);
    if(ret < 0)
      throw(std::runtime_error("MouseBase::initMouse -- Failed to get device descriptor!"));


    unsigned char str[STR_CHAR_ARRAY_SIZE];

    ret = libusb_get_string_descriptor_ascii(dev_handle_, desc.iManufacturer, str, STR_CHAR_ARRAY_SIZE );
    if(ret)
      name += " -- " + std::string(reinterpret_cast<char*>(&str));

    ret = libusb_get_string_descriptor_ascii(dev_handle_, desc.iProduct, str, STR_CHAR_ARRAY_SIZE );
    if(ret)
      name += ", " + std::string(reinterpret_cast<char*>(&str));

    name += " (vid/pid: " + boost::lexical_cast<std::string>(vid_);
    name += "/"  + boost::lexical_cast<std::string>(pid_) + ")";
  }
  else
  {
	name = "MouseLogger";
  
    dsp_ = XOpenDisplay( NULL );
    if( !dsp_ )
      throw(std::runtime_error( "Error -- " + std::string(BOOST_CURRENT_FUNCTION) + "Unable to open display!" ));

    screen_number_ = DefaultScreen(dsp_);

    int num_sizes;


    Window root = DefaultRootWindow(dsp_);
    XRRScreenSize *xrrs = XRRSizes(dsp_, 0, &num_sizes);
    XRRScreenConfiguration *conf = XRRGetScreenInfo(dsp_, root);
    //short original_rate = XRRConfigCurrentRate(conf);
    Rotation original_rotation;
    SizeID original_size_id = XRRConfigCurrentConfiguration(conf, &original_rotation);

    screen_width_ = xrrs[original_size_id].width;
    screen_height_ = xrrs[original_size_id].height;

    event_ = new XEvent;
  }


//  cout << " --> Mouse ID: " << id_ << ",  Name: " << name_;
//  cout<<", vid: "<<vid_<<", pid: "<<pid_<<endl;

  setType(name);
  fs_ = 100;
}

//-----------------------------------------------------------------------------

Mouse::~Mouse()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  running_ = false;
  async_acqu_thread_->join();
  if(async_acqu_thread_)
    delete async_acqu_thread_;
  if(detach_from_os_)
    freeKernelDriver();
  else
    XCloseDisplay(dsp_);

  if(event_)
    delete(event_);
}

//-----------------------------------------------------------------------------

int Mouse::blockKernelDriver()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

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
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

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
  #ifdef DEBUG
    std::cout <<  BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  while(running_)
  {
//    cout <<  " -- " << dirty_ << std::flush;

    if(detach_from_os_)
      getDataFromLibUSB();
    else
    {
      boost::unique_lock<boost::shared_mutex> lock(rw_);

      XQueryPointer(dsp_, RootWindow(dsp_, screen_number_),
                    &event_->xbutton.root,    &event_->xbutton.window,
                    &event_->xbutton.x_root,  &event_->xbutton.y_root,
                    &event_->xbutton.x,       &event_->xbutton.y,
                    &event_->xbutton.state);

      bool button_pressed[PRE_DEFINED_NR_MOUSE_BUTTONS_] = {0};

      button_pressed[0] = ( event_->xbutton.state & (Button1Mask) ) >0;
      button_pressed[1] = ( event_->xbutton.state & (Button2Mask) ) >0;
      button_pressed[2] = ( event_->xbutton.state & (Button3Mask) ) >0;

      if( (buttons_values_[0] != button_pressed[0]) ||
          (buttons_values_[1] != button_pressed[1]) ||
          (buttons_values_[2] != button_pressed[2]) )
      {
        buttons_values_[0] = button_pressed[0];
        buttons_values_[1] = button_pressed[1];
        buttons_values_[2] = button_pressed[2];
        dirty_ = true;
      }

      if( (axes_values_[0] != event_->xbutton.x) || (axes_values_[1] != event_->xbutton.y) )
      {
        axes_values_[0] = event_->xbutton.x;
        axes_values_[1] = event_->xbutton.y;
        dirty_ = true;
      }

      //      if(dirty_)
      //      {
      //        cout << "Mouse Corodinates: "<<  axes_values_[0] << "," << axes_values_[1] << " -- ";
      //        cout << buttons_values_[0] << ",";
      //        cout << buttons_values_[1] << ",";
      //        cout << buttons_values_[2];
      //        cout << endl << std::flush;
      //      }

      lock.unlock();
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
  }
}

//-----------------------------------------------------------------------------

void Mouse::getDataFromLibUSB()
{
  boost::unique_lock<boost::shared_mutex> lock(rw_);
  dirty_ = false;

  int actual_length;
  unsigned char async_data_[ACYNC_DATA_RECV_SIZE];
  int r = libusb_interrupt_transfer(dev_handle_, usb_port_, async_data_,
                                    ACYNC_DATA_RECV_SIZE, &actual_length, 100);

  if(r == LIBUSB_ERROR_TIMEOUT)
  {
    //cout<<"   Mouse: Timeout!"<<endl;
    return;
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

  dirty_ = true;
  int async_data_buttons_ = static_cast<int>(async_data_[0] );
  axes_values_[0] = static_cast<int>( async_data_[1] );
  axes_values_[1] = static_cast<int>( async_data_[2] );

  for(unsigned int n = 0; n < buttons_values_.size(); n++)
  {
    bool value = 0;

    if ( async_data_buttons_ & static_cast<int>( 1 << n ) )
      value = 1;

    if( value != buttons_values_[n])
      buttons_values_[n] = value;
  }

  lock.unlock();
}

//-----------------------------------------------------------------------------


}
#endif //__linux__
