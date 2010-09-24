
#include "hardware/mouse_linux.h"


namespace tobiss
{

set<boost::uint16_t> Mouse::used_ids_;

//-----------------------------------------------------------------------------

Mouse::Mouse(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
  : HWThread(parser)
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

  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(cst_.hw_ds, true));
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

    ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_vid,true));
    setVendorId(elem);

    ticpp::Iterator<ticpp::Element> elem2(father->FirstChildElement(cst_.hw_pid,true));
    setProductId(elem2);

    ticpp::Iterator<ticpp::Element> elem3(father->FirstChildElement(cst_.usb_port,true));
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
          x=y=0;

          unsigned char data[3];
          int actual_length;
          int r = libusb_bulk_transfer(dev_handle_, usb_port_, data, sizeof(data), &actual_length, 0);
          if (r == 0 && actual_length == sizeof(data))
          {
              int dx = (int)(char)data[1];
              int dy = (int)(char)data[2];
              x += dx;
              y += dy;
          }


          for(uint n = 0; n < buttons_values_.size(); n++)
          {
            bool value = 0;
            int state_n = ((int)data[0] & (int)pow(2,n));
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

}

//-----------------------------------------------------------------------------

void Mouse::stop() {

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



} // Namespace tobiss
