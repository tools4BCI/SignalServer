#include "hardware/gBSamp_unix.h"
#include <comedilib.h>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

// STL
#include <iostream>

namespace tobiss
{
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

using boost::lexical_cast;
using boost::bad_lexical_cast;

using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;

#define SUBDEVICE_AI 0 //subdevice for analog input
#define PREF_RANGE  0 //range -10V - +10V
#define BUF_SIZE 10000
//-----------------------------------------------------------------------------

gBSamp::gBSamp(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
: HWThread(parser), acquiring_(0), current_block_(0), first_run_(1)
{
  #ifdef DEBUG
    cout << "gBSamp: Constructor" << endl;
  #endif

  setHardware(hw);

  expected_values_ = nr_ch_ * blocks_;

  buffer_.init(blocks_ , nr_ch_ , channel_types_);
  data_.init(blocks_, nr_ch_, channel_types_);
  samples_.resize(expected_values_, 0);

  initCard();

  cout << " * gBSamp sucessfully initialized" << endl;
  cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_  << ", blocksize: " << blocks_  << endl;

}

//-----------------------------------------------------------------------------

gBSamp::~gBSamp()
{
  #ifdef DEBUG
    cout << "gBSamp: Destructor" << endl;
  #endif

  if(device_)
  {
    comedi_cancel(device_, 0);
    comedi_close(device_);
  }
}

//-----------------------------------------------------------------------------

void gBSamp::run()
{
  #ifdef DEBUG
    cout << "gBSamp: run" << endl;
  #endif

  running_ = 1;

  cout << " * gBSamp sucessfully started" << endl;
}

//-----------------------------------------------------------------------------

void gBSamp::stop()
{
  #ifdef DEBUG
    cout << "gBSamp: stop" << endl;
  #endif

  running_ = 0;

  cond_.notify_all();
  cout << " * gBSamp sucessfully stopped" << endl;
}

//-----------------------------------------------------------------------------

SampleBlock<double> gBSamp::getSyncData()
{
  #ifdef DEBUG
    cout << "gBSamp: getSyncData" << endl;
  #endif

  if(!running_)
  {
    cout << "Not running!" << endl;
    return(data_);
  }

  if(!acquiring_)
    acquiring_ = 1;

  boost::shared_lock<boost::shared_mutex> lock(rw_);

  if(first_run_)
  {
    error = comedi_command_test(device_, &comedi_cmd_);
    if (error >= 0)
    {
      cout << "gBSamp: Error for 1st comedi_command_test: " << error << endl;
    }
    error = comedi_command_test(device_, &comedi_cmd_);
    if (error >= 0)
    {
      cout << "gBSamp: Error for 2nd comedi_command_test: " << error << endl;
    }
    error = comedi_command_test(device_, &comedi_cmd_);
    if (error >= 0)
    {
      cout << "gBSamp: Error for 3nd comedi_command_test: " << error << endl;
    }
    error = comedi_command(device_, &comedi_cmd_);
    first_run_ = 0;
    if (error < 0)
    {
//      cout << "gBSamp: Error for comedi_command: " << error << endl;
      first_run_=1;
    }
    if(!first_run_)
      cout << "gBSamp: comedi_command successful" << endl;
  }

  sampl_t buf[10000];

//    comedi_get_buffer_contents(device_,0);
  read(comedi_fileno(device_),buf, BUF_SIZE*sizeof(sampl_t));

  double phys_val_[10000];
  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  int chan=0;

  for(int ch=0; ch < nr_ch_; it++)
  {
    chan = it->first - 1;
//    if (chan == 0) cout << "Channel: " << chan << endl;
    range_info = comedi_get_range(device_, 0, chan, 0);
    maxdata = comedi_get_maxdata(device_, 0, chan);
    for(int i = 0; i <  (expected_values_/blocks_); i++)
    {
      phys_val_[i*ch+i] = comedi_to_phys(buf[i*ch+i], range_info, maxdata);
//      phys_val_[i] = comedi_to_phys(buf[i], range_info, maxdata);
//      fout << phys_val_[i] << ";" << endl;
//      cout << "RAW value [" << (i*ch+i) << "]: " << buf[i*ch+i];
//      cout << "Physical value [" << (i*ch+i) << "]: " << phys_val_[i*ch+i] << endl;
    }
    ch++;
  }

//  boost::shared_lock<boost::shared_mutex> lock(rw_);

  for(int i=0; i < (expected_values_/blocks_); i++)
    for(int j=0; j < blocks_; j++)
      samples_[i+j] = phys_val_[i+j];

//  cout << "gBSamp: getSyncData -- samples.size() " << samples_.size() << endl;
//  cout << "sampleblock size: " << data_.getNrOfSamples() << endl;

  data_.setSamples(samples_);

  samples_available_ = false;
  lock.unlock();

  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> gBSamp::getAsyncData()
{
  #ifdef DEBUG
    cout << "gBSamp: getAsyncData" << endl;
  #endif

  boost::shared_lock<boost::shared_mutex> lock(rw_);
//  data_.setSamples(samples_);
  samples_available_ = false;
  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

int gBSamp::initCard()
{
  #ifdef DEBUG
    cout << "gBSamp: getAsyncData" << endl;
  #endif

  device_ = comedi_open("/dev/comedi0");
  if(device_==0)
  {
      cerr << "Error: " << comedi_strerror( comedi_errno() ) << endl;
      return -1;
  }

#ifdef DEBUG
    cout << "gBSamp: device opened!" << endl;
  #endif

  comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

//  cout << sysconf(_SC_PAGE_SIZE) << endl;
//  int bufsize = comedi_get_max_buffer_size(device_,0);
//  cout << bufsize << endl;
//  comedi_set_buffer_size(device_,0,bufsize);

  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  int chan=0;
  unsigned int channel_list[nr_ch_];

//  for(int i=0; i < nr_ch_; it++)
//  {
//    chan = it->first - 1;
//    cout << "Channel: " << chan << endl;
//    channel_list[i] = CR_PACK(chan, 0, AREF_GROUND);
//    i++;
//  }
  channel_list[0] = CR_PACK(0, 0, AREF_GROUND);

  comedi_cmd_.subdev = 0;
  comedi_cmd_.flags = TRIG_ROUND_NEAREST | TRIG_WAKE_EOS;

  comedi_cmd_.start_src = TRIG_NOW;
  comedi_cmd_.start_arg = 0;

  comedi_cmd_.scan_begin_src = TRIG_TIMER;
//  comedi_cmd_.scan_begin_arg = 1e6/(fs_*10000); // (1/fs)=0,00irgendwas sekunden, sind irgendwas ms -> umrechnen !!!
  comedi_cmd_.scan_begin_arg = fs_*1000; //---------- funktioniert!!!!!!!!!!!!!!

  comedi_cmd_.convert_src = TRIG_TIMER;
//  comedi_cmd_.convert_arg = fs_*100000;
  comedi_cmd_.convert_arg = fs_*1000; //---------- funktioniert!!!!!!!!!!!!!!

  comedi_cmd_.scan_end_src = TRIG_COUNT;
  comedi_cmd_.scan_end_arg = nr_ch_;

  comedi_cmd_.stop_src = TRIG_NONE;
  comedi_cmd_.stop_arg = 0;

  comedi_cmd_.chanlist = channel_list;
  comedi_cmd_.chanlist_len = nr_ch_;

  return error;
}

//-----------------------------------------------------------------------------

void gBSamp::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "gBSamp: setHardware" << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(cst_.hw_ds, true));

  setDeviceSettings(ds);

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(cst_.hw_cs, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == cst_.hw_cs)
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware_name +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Multiple channel_settings found!";
        throw(ticpp::Exception(ex_str));
      }
      setChannelSettings(cs);
  }

  //TODO: set extra filtersettings per channel if wanted

}

//-----------------------------------------------------------------------------

void gBSamp::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "gBSamp: setDeviceSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_fs,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(cst_.hw_channels,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(cst_.hw_buffer,false);
  if(elem != elem.end())
    setBlocks(elem);

  ticpp::Iterator<ticpp::Element> filter(father->FirstChildElement(cst_.hw_fil, false));
  if (filter != filter.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(filter); ++it != it.end(); )
      if(it->Value() == cst_.hw_cs)
        setDeviceFilterSettings(filter);
  }

}

//---------------------------------------------------------------------------------------

void gBSamp::setDeviceFilterSettings(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "gBSamp: setDeviceFilterSettings" << endl;
  #endif

  checkFilterAttributes(elem);

  unsigned int type = 0;
  bool notch = 0;
  float f_low = 0;
  float f_high = 0;
  float sense = 0;

  getFilterParams(elem, type, notch, f_low, f_high, sense);

  cout << " * g.BSamp -- filters per type set to:" << endl;
  cout << "    ...  type: " << type << ", f_low: " << f_low << ", f_high: " << f_high << ", sense: " << sense << ", notch: " << notch << endl;
  cout << endl;

}

//---------------------------------------------------------------------------------------

void gBSamp::checkFilterAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: checkFilterAttributes" << endl;
  #endif

  if(!elem.Get()->HasAttribute(cst_.hw_fil_type))
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, filter type ("+cst_.hw_fil_type+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(cst_.hw_notch))
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, filter order ("+cst_.hw_notch+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(cst_.hw_fil_low))
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, lower cutoff frequency ("+cst_.hw_fil_low+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(cst_.hw_fil_high))
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, upper cutoff frequency ("+cst_.hw_fil_high+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(cst_.hw_fil_sense))
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, parameter ("+cst_.hw_fil_sense+") not given!";
    throw(ticpp::Exception(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void gBSamp::getFilterParams(ticpp::Iterator<ticpp::Element>const &elem,\
  unsigned int &type, bool &notch, float &f_low, float &f_high, float &sense)
{
  #ifdef DEBUG
    cout << "gBSamp: getFilterParams" << endl;
  #endif

  type = cst_.getSignalFlag(elem.Get()->GetAttribute(cst_.hw_fil_type));
  try
  {
    notch = cst_.equalsOnOrOff(elem.Get()->GetAttribute(cst_.hw_notch));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, but notch is not 'on' or 'off'";
    throw(ticpp::Exception(ex_str));
  }
  if(type == SIG_EEG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_low));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_high));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_sense));
      if((f_low != .05) || (f_low != .1))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '0.05' or '.1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EOG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_low));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_high));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_sense));
      if((f_low != .1) || (f_low != 1))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '0.1' or '1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EMG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_low));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_high));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_sense));
      if((f_low != 1) || (f_low != 5))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '1' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_ECG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_low));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_high));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(cst_.hw_fil_sense));
      if((f_low != 2) || (f_low != 5))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not '2' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
      ex_str += "Tag <"+cst_.hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }
}

//---------------------------------------------------------------------------------------

void gBSamp::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "gBSamp: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_sel,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

} // Namespace tobiss
