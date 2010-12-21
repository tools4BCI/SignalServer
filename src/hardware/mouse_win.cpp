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

#ifdef _WIN32

#include "hardware/mouse_win.h"

namespace tobiss
{

const string Mouse::dc_path("devcon_path");
const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> Mouse::FACTORY_REGISTRATOR_ ("mouse");

//-----------------------------------------------------------------------------

Mouse::Mouse(ticpp::Iterator<ticpp::Element> hw)
     : MouseBase(hw)
    {
		Constants cst;
		ticpp::Iterator<ticpp::Element> elem(DS->FirstChildElement(dc_path,true));
		devcon_path_ = elem->GetText(true);
		
		string VID,PID;
		std::ostringstream v,p;
		v << setw(4) << setfill('0') << hex << vid_;
		p << setw(4) << setfill('0') << hex << pid_;
		VID = v.str();
		PID = p.str();
		hw_id_ = "\"USB\\VID_"+VID+"&PID_"+PID+"\"";
			
		int ret = blockKernelDriver();
        if(ret)
          throw(std::runtime_error("MouseBase::initMouse -- Mouse device could not be connected!"));
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
	STARTUPINFO         siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;
    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
    siStartupInfo.cb = sizeof(siStartupInfo);

	string command = " disable -r "+hw_id_;
    CreateProcess(const_cast<LPCSTR>(devcon_path_.c_str()),
         const_cast<LPSTR>(command.c_str()),
		 0,0,FALSE,CREATE_DEFAULT_ERROR_MODE,0,0,&siStartupInfo,&piProcessInfo);
    WaitForSingleObject(piProcessInfo.hProcess, 1000);

	const char *inf_file = "libusb/mouse.inf";
  
	int ret = usb_install_driver_np(inf_file);
	if(ret<0)
          throw(std::runtime_error("Mouse::blockKernelDriver -- No 'mouse.inf' file found in 'bin\\libusb\\' !"));
	
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

	cout<< "MouseDevice successfully connected "<<endl;
	return 0;

}

//-----------------------------------------------------------------------------

int Mouse::freeKernelDriver()
{
	usb_release_interface(dev_handle_,0);
	usb_close(dev_handle_);

	STARTUPINFO         siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	string command = " remove -r ";
	command += hw_id_;
    CreateProcess(const_cast<LPCSTR>(devcon_path_.c_str()),
         const_cast<LPSTR>(command.c_str()),
		 0,0,FALSE,CREATE_DEFAULT_ERROR_MODE,0,0,&siStartupInfo,&piProcessInfo);
    WaitForSingleObject(piProcessInfo.hProcess, 1000);

	CreateProcess(const_cast<LPCSTR>(devcon_path_.c_str()),
         " rescan",0,0,FALSE,CREATE_DEFAULT_ERROR_MODE,0,0,&siStartupInfo,&piProcessInfo);
    WaitForSingleObject(piProcessInfo.hProcess, 1000);    
	
	cout<< "MouseDevice disconnected"<<endl;
	return 0;
}

//-----------------------------------------------------------------------------


void Mouse::acquireData()
{
  while(running_)
  {
	    boost::unique_lock<boost::shared_mutex> lock(rw_);
        unsigned char async_data_[10];
		int r = usb_bulk_read(dev_handle_,usb_port_, (char *)async_data_, sizeof(async_data_), 100);
		if(r<0){
			cout<<"usb-device could not be read, please check usb_port"<<endl;
			break;
		}
		async_data_buttons_ = (int)(char)async_data_[0];
        async_data_x_ = (int)(char)async_data_[1];
        async_data_y_ = (int)(char)async_data_[2];
        lock.unlock();
  }
}

//-----------------------------------------------------------------------------



} // Namespace tobiss

#endif // _WIN32