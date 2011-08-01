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

#include "hardware/mouse.h"


namespace tobiss
{
using boost::lexical_cast;
using boost::bad_lexical_cast;

using std::vector;
using std::string;
using std::pair;
using std::cout;
using std::endl;
using std::set;

set<boost::uint16_t> MouseBase::used_ids_;

const string MouseBase::hw_vid("vendorid");
const string MouseBase::hw_pid("productid");
const string MouseBase::usb_port("usb_port");

//-----------------------------------------------------------------------------
MouseBase::MouseBase(ticpp::Iterator<ticpp::Element> hw)
: HWThread()
{
  #ifdef DEBUG
    cout << "MouseBase: Constructor" << endl;
  #endif
  setType("Mouse");
  checkMandatoryHardwareTags(hw);
  if(mode_ != APERIODIC)
    throw(std::invalid_argument("Mouse has to be started as aperiodic device!"));
  initMouse();

  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));
  setDeviceSettings(ds);
  DS = ds;
  data_.init(1, channel_types_.size() , channel_types_);

  vector<boost::uint32_t> v;
  empty_block_.init(0,0, v);

  //	cout << " * Mouse sucessfully initialized -- running as aperiodic: ";
  //	cout << (mode_ == APERIODIC) << ";  ";


  cout << "   Mouse ID: " << id_ << ",  Name: " << name_;
  cout<<", vid: "<<vid_<<", pid: "<<pid_<<endl;
}

//-----------------------------------------------------------------------------

MouseBase::~MouseBase()
{
}

//---------------------------------------------------------------------------------------

void MouseBase::setVendorId(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "HWThread: setVendorId" << endl;
  #endif

  try
  {
    vid_ = lexical_cast<int>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- VendorId: value is not a number!");
    throw(std::invalid_argument(ex_str));
  }
  if(blocks_ == 0)
  {
    string ex_str(type_ + " -- VendorId: value is 0!");
    throw(std::invalid_argument(ex_str));
  }
}
//-----------------------------------------------------------------------------

void MouseBase::setProductId(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "HWThread: setProductId" << endl;
  #endif

  try
  {
    pid_ = lexical_cast<int>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ProductId: value is not a number!");
    throw(std::invalid_argument(ex_str));
  }
  if(blocks_ == 0)
  {
    string ex_str(type_ + " -- ProductId: value is 0!");
    throw(std::invalid_argument(ex_str));
  }
}

//-----------------------------------------------------------------------------

void MouseBase::setUsbPort(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "HWThread: setUsbPort" << endl;
  #endif

  try
  {
    usb_port_ = lexical_cast<int>(elem->GetText(true));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- Usb port: value is not a number!");
    throw(std::invalid_argument(ex_str));
  }
  if(blocks_ == 0)
  {
    string ex_str(type_ + " -- Usb port: value is 0!");
    throw(std::invalid_argument(ex_str));
  }
}

//-----------------------------------------------------------------------------

void MouseBase::setDeviceSettings(ticpp::Iterator<ticpp::Element>const& father)
{
  #ifdef DEBUG
    cout << "MouseBase: setDeviceSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_vid,true));
  setVendorId(elem);

  ticpp::Iterator<ticpp::Element> elem2(father->FirstChildElement(hw_pid,true));
  setProductId(elem2);

  ticpp::Iterator<ticpp::Element> elem3(father->FirstChildElement(usb_port,true));
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

  boost::uint16_t n = 1;
  if(buttons_)
    for( ; n <= buttons_ +1; n++)
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_MBUTTON)));

  if(axes_)
    for( ; n <= axes_ + buttons_ +2; n++)
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_MOUSE)));


  homogenous_signal_type_ = 0;

}

//---------------------------------------------------------------------------------------

void MouseBase::setChannelSettings(ticpp::Iterator<ticpp::Element>const& )
{
  //  #ifdef DEBUG
  cout << "MouseBase: setChannelSettings" << endl;
  //  #endif

}

//---------------------------------------------------------------------------------------

SampleBlock<double> MouseBase::getAsyncData()
{
  #ifdef DEBUG
  cout << "MouseBase: getAsyncData" << endl;
  #endif
  if(running_)
  {
    bool dirty = 0;

    for(int n = 0; n < buttons_values_.size(); n++)
    {
      bool value = 0;
      double base = 2;
      int state_n = (async_data_buttons_ & static_cast<int>(pow(base,n)));
      if (state_n!=value)
        value = 1;
      if( value != buttons_values_[n])
      {
        dirty = 1;
        buttons_values_[n] = value;
      }
    }

    if(async_data_x_!=0 || async_data_y_!=0)
    {
      dirty = 1;
      axes_values_[0]+=async_data_x_;
      axes_values_[1]+=async_data_y_;

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

void MouseBase::run()  {
  if(mode_ == APERIODIC)
  {
    async_acqu_thread_ = new boost::thread( boost::bind(&MouseBase::acquireData, this) );
  }
  running_ = true;
}

//-----------------------------------------------------------------------------

void MouseBase::stop() {
  running_ =false;
}

//-----------------------------------------------------------------------------

void MouseBase::initMouse()
{
  set<boost::uint16_t>::iterator it(used_ids_.begin());

  axes_    = 2;
  buttons_ = 3;

  axes_values_.resize(axes_,0);
  buttons_values_.resize(buttons_,0);

  axes_values_[0] = 0;
  axes_values_[1] = 0;

  async_data_x_ = async_data_x_ = 0;
  async_data_buttons_ = 0;

  //ctx_ = NULL; //a libusb session
  user_interrupt_ = false;
}

//-----------------------------------------------------------------------------

int MouseBase::blockKernelDriver(){
  return 0;
}

//-----------------------------------------------------------------------------

int MouseBase::freeKernelDriver(){
  return 0;
}

//-----------------------------------------------------------------------------

void MouseBase::acquireData(){

}

//-----------------------------------------------------------------------------



} // Namespace tobiss
