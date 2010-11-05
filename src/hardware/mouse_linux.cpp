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

set<boost::uint16_t> Mouse::used_ids_;


const HWThreadBuilderTemplateRegistratorWithoutIOService<Mouse> Mouse::FACTORY_REGISTRATOR_ ("mouse");

//-----------------------------------------------------------------------------
Mouse::Mouse(ticpp::Iterator<ticpp::Element> hw)
  : HWThread()
{
  #ifdef DEBUG
    cout << "Mouse: Constructor" << endl;
  #endif

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
//  #ifdef DEBUG
    cout << "Mouse: setChannelSettings" << endl;
//  #endif

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

          int dx = (int)(char)async_data_[1];
          int dy = (int)(char)async_data_[2];

          for(uint n = 0; n < buttons_values_.size(); n++)
          {
            bool value = 0;
            int state_n = ((int)(char)async_data_[0] & (int)pow(2,n));
            if (state_n!=value)
                value = 1;
              if( value != buttons_values_[n])
            {
              dirty = 1;
              buttons_values_[n] = value;
            }
          }

          if(dx!=0 || dy!=0)
          {
              dirty = 1;
              axes_values_[0]+=dx;
              axes_values_[1]+=dy;

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
   running_ =false;
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

  ctx_ = NULL; //a libusb session
  user_interrupt_ = false;
}


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
        return-1;
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
    int r = libusb_interrupt_transfer(dev_handle_, usb_port_, async_data_, sizeof(async_data_), &actual_length, 0);
    if(r)
      throw(std::runtime_error("Mouse::Mouse -- Mouse device could not be connected! Check usb-port!"));
    lock.unlock();
  }
}

//-----------------------------------------------------------------------------



} // Namespace tobiss
