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

#include <boost/thread.hpp>
#include <iomanip>

#include "extern\include\libusb\usb.h"


#include "hardware/mouse_win.h"
#include "tia/constants.h"
#include "boost/filesystem.hpp"

#define ACYNC_DATA_RECV_SIZE 16

namespace tobiss
{
using std::string;

static const int LIBUSB_ERROR_PORT  = -22;      // by trial, no definitions where found in libusb-win32
static const int LIBUSB_ERROR_TIMEOUT  = -116;  // by trial, no definitions where found in libusb-win32

const std::string Mouse::hw_dc_path_("devcon_path");
const std::string Mouse::hw_inf_file_path_("inf_file_path");

const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> Mouse::FACTORY_REGISTRATOR_ ("mouse");

//-----------------------------------------------------------------------------

Mouse::Mouse(ticpp::Iterator<ticpp::Element> hw)
: MouseBase(hw), dev_handle_(0)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  tia::Constants cst;
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));

  if(detach_from_os_)
  {
    ticpp::Iterator<ticpp::Element> elem(ds->FirstChildElement(hw_dc_path_,true));
    devcon_path_ = elem->GetText(true);
    size_t found = devcon_path_.find(" ");

    if(found != string::npos)
      throw(std::runtime_error("MouseBase::Constructor -- Spaces in the devcon_path are forbidden!"));

    if(!boost::filesystem::exists(devcon_path_.c_str()))
      throw(std::invalid_argument("Mouse::Constructor -- Devcon-path <" + devcon_path_ +  "> does not exist!"));

    elem = ds->FirstChildElement(hw_inf_file_path_,true);
    inf_file_path_ = elem->GetText(true);

    if(!boost::filesystem::exists(inf_file_path_.c_str()))
      throw(std::invalid_argument("Mouse::Constructor -- Inf-file path: <" + inf_file_path_ +  "> does not exist!"));

    string VID, PID;
    std::ostringstream v,p;
    v << std::setw(4) << std::setfill('0') << std::hex << vid_;
    p << std::setw(4) << std::setfill('0') << std::hex << pid_;
    VID = v.str();
    PID = p.str();
    hw_id_ = "\"USB\\VID_"+VID+"&PID_"+PID+"\"";

    if(blockKernelDriver())
      throw(std::runtime_error("MouseBase::initMouse -- Mouse device could not be connected (check rights)!"));
  }

  setType("Mouse");

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
}

//-----------------------------------------------------------------------------

int Mouse::blockKernelDriver()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
  // create all parameter needed to open an extern tool (CreateProcess)
  STARTUPINFO         siStartupInfo;
  PROCESS_INFORMATION piProcessInfo;
  memset(&siStartupInfo, 0, sizeof(siStartupInfo));
  memset(&piProcessInfo, 0, sizeof(piProcessInfo));
  siStartupInfo.cb = sizeof(siStartupInfo);

  //disable the standard windows driver for the specific mouse device
  string command = "disable -r "+hw_id_;
  CreateProcess(const_cast<LPCSTR>(devcon_path_.c_str()),
  const_cast<LPSTR>(command.c_str()),
  0,0,FALSE,CREATE_NEW_CONSOLE,0,0,&siStartupInfo,&piProcessInfo);
  WaitForSingleObject(piProcessInfo.hProcess, 1000);

  //load the new driver from the mouse.inf file

  //  const char *inf_file = "libusb/mouse.inf";

  int ret = usb_install_driver_np( inf_file_path_.c_str() );
  if(ret<0)
    throw(std::runtime_error("Mouse::blockKernelDriver -- Could not install given .inf file!"));

  // find the right usb-device and open it
  struct usb_bus *UsbBus = NULL;
  struct usb_device *UsbDevice = NULL;
  usb_find_busses();
  usb_find_devices();
  for (UsbBus = usb_get_busses(); UsbBus; UsbBus = UsbBus->next) {
    for (UsbDevice = UsbBus->devices; UsbDevice; UsbDevice = UsbDevice->next) {
      if (UsbDevice->descriptor.idVendor == vid_ && UsbDevice->descriptor.idProduct== pid_) {
        break;
      }
    }
  }
  if (!UsbDevice)
    return -1;
  dev_handle_ = usb_open(UsbDevice);
  if(!dev_handle_)
    return -11;

  if (usb_set_configuration (dev_handle_, 1) < 0) {
    usb_close(dev_handle_);
    return -2;
  }

  if (usb_claim_interface (dev_handle_, 0) < 0) {
    usb_close(dev_handle_);
    return -3;
  }

  return 0;

}

//-----------------------------------------------------------------------------

int Mouse::freeKernelDriver()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
  // release the interface end close the usb-handle
  usb_release_interface(dev_handle_,0);
  usb_close(dev_handle_);

  // create all parameter needed to open an extern tool (CreateProcess)
  STARTUPINFO         siStartupInfo;
  PROCESS_INFORMATION piProcessInfo;
  memset(&siStartupInfo, 0, sizeof(siStartupInfo));
  memset(&piProcessInfo, 0, sizeof(piProcessInfo));
  siStartupInfo.cb = sizeof(siStartupInfo);

  // remove the temporal driver from the mouse device
  string command = " remove -r ";
  command += hw_id_;
  CreateProcess(const_cast<LPCSTR>(devcon_path_.c_str()),
  const_cast<LPSTR>(command.c_str()),
  0,0,FALSE,CREATE_NEW_CONSOLE,0,0,&siStartupInfo,&piProcessInfo);
  WaitForSingleObject(piProcessInfo.hProcess, 1000);

  // rescan for standard driver of the mouse device
  CreateProcess(const_cast<LPCSTR>(devcon_path_.c_str()),
  " rescan",0,0,FALSE,CREATE_NEW_CONSOLE,0,0,&siStartupInfo,&piProcessInfo);
  WaitForSingleObject(piProcessInfo.hProcess, 1000);

  //  CreateProcess(const_cast<LPCSTR>(devcon_path_.c_str()),
  //" update",0,0,FALSE,CREATE_NEW_CONSOLE,0,0,&siStartupInfo,&piProcessInfo);
  //WaitForSingleObject(piProcessInfo.hProcess, 1000);

  return 0;
}

//-----------------------------------------------------------------------------


void Mouse::acquireData()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
  while(running_)
  {
    if(detach_from_os_)
      getDataFromLibUSB();
    else
    {
      dirty_ = false;
      boost::unique_lock<boost::shared_mutex> lock(rw_);

      bool button_pressed[PRE_DEFINED_NR_MOUSE_BUTTONS_] = {0};

      button_pressed[0] = (GetAsyncKeyState(VK_LBUTTON) != 0);
      button_pressed[1] = (GetAsyncKeyState(VK_MBUTTON) != 0);
      button_pressed[2] = (GetAsyncKeyState(VK_RBUTTON) != 0);

      if( (buttons_values_[0] != button_pressed[0]) ||
          (buttons_values_[1] != button_pressed[1]) ||
          (buttons_values_[2] != button_pressed[2]) )
      {
        buttons_values_[0] = button_pressed[0];
        buttons_values_[1] = button_pressed[1];
        buttons_values_[2] = button_pressed[2];
        dirty_ = true;
      }

      POINT cursorPos;
      GetCursorPos(&cursorPos);

      if( (axes_values_[0] != cursorPos.x) || (axes_values_[1] != cursorPos.y) )
      {
        axes_values_[0] = cursorPos.x;
        axes_values_[1] = cursorPos.y;
        dirty_ = true;
      }
    }
  }
}

//-----------------------------------------------------------------------------

void Mouse::getDataFromLibUSB()
{
  boost::unique_lock<boost::shared_mutex> lock(rw_);

  dirty_ = false;
  char async_data_[ACYNC_DATA_RECV_SIZE];
  int r = usb_interrupt_read(dev_handle_,usb_port_, async_data_, ACYNC_DATA_RECV_SIZE, 100);

  if(r == LIBUSB_ERROR_TIMEOUT)
  {
    //cout<<"   Mouse: Timeout!"<<endl;
    return;
  }
  else if(r == LIBUSB_ERROR_PORT)
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

} // Namespace tobiss

