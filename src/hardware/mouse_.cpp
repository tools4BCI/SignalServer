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

#include "hardware/mouse_.h"

namespace tobiss
{

using std::vector;
using std::string;
using std::map;
using std::pair;
using std::cout;
using std::endl;
using std::set;

set<boost::uint16_t> Mouse::used_ids_;

const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> Mouse::FACTORY_REGISTRATOR_ ("mouse");

//-----------------------------------------------------------------------------

Mouse::Mouse(ticpp::Iterator<ticpp::Element> hw)
  : HWThread()
{
  #ifdef DEBUG
    cout << "Mouse: Constructor" << endl;
  #endif

  setType("Mouse");
  checkMandatoryHardwareTags(hw);
  if(mode_ != APERIODIC)
    throw(std::invalid_argument("Mouse has to be started as aperiodic device!"));
  //ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(cst_.hw_ds, true));



//   ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(cst_.hw_cs, false));
//   if (cs != cs.end())
//   {
//     for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
//       if(it->Value() == cst_.hw_cs)
//       {
//         string ex_str;
//         ex_str = "Error in "+ cst_.hardware_name +" - " + m_.find(cst_.hardware_name)->second + " -- ";
//         ex_str += "Multiple channel_settings found!";
//         throw(ticpp::Exception(ex_str));
//       }
//       setChannelSettings(cs);
//   }

  initMouse();

  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));
  setDeviceSettings(ds);

  data_.init(1, channel_types_.size() , channel_types_);

  vector<boost::uint32_t> v;
  empty_block_.init(0,0, v);

  int ret = blockKernelDriver();
  if(ret)
    throw(std::runtime_error("Mouse::Mouse -- Mouse device could not be connected!"));

  cout<<"vid: "<<vid_<<"pid: "<<pid_<<endl;
  cout << " * Mouse sucessfully initialized -- running as aperiodic: ";
  cout << (mode_ == APERIODIC) << ";  ";
  cout << "Mouse ID: " << id_ << ",  Name: " << name_;
   cout<<", vid: "<<vid_<<", pid: "<<pid_<<endl;
}

//-----------------------------------------------------------------------------

Mouse::~Mouse()
{
  freeKernelDriver();
}

//-----------------------------------------------------------------------------

void Mouse::setDeviceSettings(ticpp::Iterator<ticpp::Element>const& father)
{
  #ifdef DEBUG
    cout << "Mouse: setDeviceSettings" << endl;
  #endif

    //   ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_fs,true));
    //   setSamplingRate(elem);

    //ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_channels,true));
    //   elem = father->FirstChildElement(cst_.hw_channels,true);

  Constants cst;

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst.hw_vid,true));
  setVendorId(elem);

  ticpp::Iterator<ticpp::Element> elem2(father->FirstChildElement(cst.hw_pid,true));
  setProductId(elem2);

	ticpp::Iterator<ticpp::Element> elem3(father->FirstChildElement(cst.usb_port,true));
  setUsbPort(elem3);

  string naming;
  string type;

  if(buttons_)
    channel_types_.push_back(SIG_MBUTTON);
  for(boost::uint32_t n = 0; n < buttons_; n++)
    channel_types_.push_back(SIG_MBUTTON);

  if(axes_)
    channel_types_.push_back(SIG_MOUSE);
  for(boost::uint32_t n = 0; n < axes_; n++)
    channel_types_.push_back(SIG_MOUSE);
  nr_ch_= channel_types_.size();

  char VID[4];
  char PID[4];
  strcpy_s(hw_id_,"");
  strcat_s(hw_id_,"\"USB\\VID_");
  itoa (vid_,VID,16);
  if((vid_ & 0x1000) == 0)
  	strcat_s(hw_id_,"0");
  if((vid_ & 0x1100) == 0)
    strcat_s(hw_id_,"0");
  if((vid_ & 0x1110) == 0)
  	strcat_s(hw_id_,"0");
  if((vid_ & 0x1111) == 0)
  	strcat_s(hw_id_,"0");
  strcat_s(hw_id_,VID);
  strcat_s(hw_id_,"&PID_");
  itoa (pid_,PID,16);
  if((pid_ & 0x1000) == 0)
  	strcat_s(hw_id_,"0");
  if((pid_ & 0x1100) == 0)
  	strcat_s(hw_id_,"0");
  if((pid_ & 0x1110) == 0)
  	strcat_s(hw_id_,"0");
  if((pid_ & 0x1111) == 0)
  	strcat_s(hw_id_,"0");
  strcat_s(hw_id_,PID);
  strcat_s(hw_id_,"\"");


  //try
  //{
  //  parser_.parseDeviceChannels(elem, nr_ch_, naming, type);
  //}
  //catch(ticpp::Exception& e)
  //{
  //  string ex_str;
  //  ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
  //  throw(ticpp::Exception(ex_str + e.what()));
  //}

  boost::uint16_t n = 1;
  if(buttons_)
    for( ; n <= buttons_ +1; n++)
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_BUTTON)));

  if(axes_)
    for( ; n <= axes_ + buttons_ +2; n++)
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_MOUSE)));


  homogenous_signal_type_ = 0;

}

//---------------------------------------------------------------------------------------

void Mouse::setChannelSettings(ticpp::Iterator<ticpp::Element>const& )
{
  #ifdef DEBUG
    cout << "Mouse: setChannelSettings" << endl;
  #endif

//   ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_sel,false));
//   if (elem != elem.end())
//     setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Mouse::getAsyncData()
{
  #ifdef DEBUG
    cout << "Mouse: getAsyncData" << endl;
  #endif
  if(!user_interrupt_)
  {
    bool dirty = 0;
    int x,y;
    x=y=0;

	int dx = (int)async_data_[1];
	int dy = (int)async_data_[2];
	x += dx;
	y += dy;

	for(int n = 0; n < buttons_values_.size(); n++)
	{
		bool value = 0;
		int state_n = ((int)async_data_[0] & (int)pow((double)2,n));
		if (state_n!=value)
		value = 1;
		if( value != buttons_values_[n])
		{
			dirty = 1;
			buttons_values_[n] = value;
		}
	 }

	 if(x!=0 || y!=0)
	 {
		dirty = 1;
		axes_values_[0]+=x;
		axes_values_[1]+=y;
	 }

	 if(!dirty)
		return(empty_block_);

     vector<double> v;

     if(buttons_)
	 v.push_back(id_);
	 for(boost::uint8_t n = 0; n < buttons_values_.size(); n++)
	 v.push_back(buttons_values_[n]);

	 if(axes_)
	 v.push_back(id_);
	 for(boost::uint8_t n = 0; n < axes_values_.size(); n++)
		v.push_back(axes_values_[n]);
		data_.setSamples(v);
		return(data_);
    }
    else
        return(empty_block_);
}

//-----------------------------------------------------------------------------

void Mouse::run()  {

	if(mode_ == APERIODIC)
		async_acqu_thread_ = new boost::thread( boost::bind(&Mouse::acquireData, this) );
     running_ = true;
}

//-----------------------------------------------------------------------------

void Mouse::stop() {
  running_ = false;
}

//-----------------------------------------------------------------------------

void Mouse::initMouse()
{
  set<boost::uint16_t>::iterator it(used_ids_.begin());

  axes_    = 2;
  buttons_ = 3;

  axes_values_.resize(axes_,0);
  buttons_values_.resize(buttons_,0);

  axes_values_[0] = 0;
  axes_values_[1] = 0;
  async_data_[0] = 0;
  async_data_[1] = 0;
  async_data_[2] = 0;
//  ctx_ = NULL; //a libusb session
  user_interrupt_ = false;
  dirty_ = false;

}


//-----------------------------------------------------------------------------

int Mouse::blockKernelDriver()
{
	STARTUPINFO         siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;

    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));

    siStartupInfo.cb = sizeof(siStartupInfo);

	char command[100];
	strcpy_s(command," disable -r ");
	strcat_s(command,hw_id_);
	cout<<endl<<endl<<"command: "<<command<<endl<<endl;
    CreateProcess("F:\\WinDDK\\7600.16385.1\\tools\\devcon\\i386\\devcon.exe",     // Application name
                     (char*)command,                 // Application arguments
                     0,
                     0,
                     FALSE,
                     CREATE_DEFAULT_ERROR_MODE,
                     0,
                     0,                              // Working directory
                     &siStartupInfo,
                     &piProcessInfo);

    WaitForSingleObject(piProcessInfo.hProcess, 10000);

	const char *inf_file = "libusb/mouse.inf";
	int test = usb_install_driver_np(inf_file);

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
		if (!UsbDevice) return -1;
        dev_handle_ = usb_open(UsbDevice);

        if (usb_set_configuration (dev_handle_, 1) < 0) {
                usb_close(dev_handle_);
                return -2;
        }

        if (usb_claim_interface (dev_handle_, 0) < 0) {
                usb_close(dev_handle_);
                return -3;
        }

		cout<< "MouseDevice successfully connected: "<<endl;
		return 0;

}

//-----------------------------------------------------------------------------

int Mouse::freeKernelDriver()
{
    //int ret;
    //ret = libusb_release_interface(dev_handle_, 0);
    //if(ret)
    //    return-1;
    //ret = libusb_attach_kernel_driver(dev_handle_, 0);
    //if(ret)
    //    return -2;
    //libusb_close(dev_handle_);
    //libusb_exit(ctx_);
    //return 0;
	usb_release_interface(dev_handle_,0);
	usb_close(dev_handle_);

STARTUPINFO         siStartupInfo;
    PROCESS_INFORMATION piProcessInfo;

    memset(&siStartupInfo, 0, sizeof(siStartupInfo));
    memset(&piProcessInfo, 0, sizeof(piProcessInfo));

    siStartupInfo.cb = sizeof(siStartupInfo);
	char command[100];
	strcpy_s(command," remove -r ");
	strcat_s(command,hw_id_);
    CreateProcess("C:\\WinDDK\\7600.16385.1\\tools\\devcon\\i386\\devcon.exe",     // Application name
                     (char*)command,                 // Application arguments
                     0,
                     0,
                     FALSE,
                     CREATE_DEFAULT_ERROR_MODE,
                     0,
                     0,                              // Working directory
                     &siStartupInfo,
                     &piProcessInfo);

    WaitForSingleObject(piProcessInfo.hProcess, 10000);
	WinExec("\"F:\\WinDDK\\7600.16385.1\\tools\\devcon\\i386\\devcon.exe\" rescan", 1);

	cout<< "MouseDevice disconnected"<<endl;
	return 0;
}

//-----------------------------------------------------------------------------


void Mouse::acquireData()
{
  while(running_)
  {

    boost::unique_lock<boost::shared_mutex> lock(rw_);
	int ret = usb_interrupt_read(dev_handle_,usb_port_, (char *)async_data_, sizeof(async_data_), 100);
    lock.unlock();
  }
}

//-----------------------------------------------------------------------------



} // Namespace tobiss
