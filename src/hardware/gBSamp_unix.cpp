#include "hardware/gBSamp_unix.h"
#include <comedilib.h>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>

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

#define SUBDEVICE_FOR_ANALOG_INPUT 0;
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
      comedi_close(device_);
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

int gBSamp::readFromDAQCard()
{
  return error;
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
    cout << "gBSamp: Command-Test 1" << endl;
    error = comedi_command_test(device_, &comedi_cmd_);
    if(error != 0)
    {
      cerr << "gBSamp: comedi_command_test - Error nr " << error  << endl;
    }
    cout << "gBSamp: Command-Test 2" << endl;
    error = comedi_command_test(device_, &comedi_cmd_);
    if(error != 0)
    {
      cerr << "gBSamp: comedi_command_test - Error nr " << error  << endl;
    }
    cout << "gBSamp: comediCommand" << endl;
    error = comedi_command(device_, &comedi_cmd_);
    if (error < 0)
      comedi_perror("gBSamp: comedi_command");
    first_run_ = 0;
  }

  lsampl_t buf[1000];
  error = read(comedi_fileno(device_),buf,sizeof(sampl_t));

//  error = comedi_do_insnlist(device_, &insn_list_);
//  if(error<0)
//    comedi_perror("gBSamp: comedi_do_insnlist");

    comedi_range* range_info = comedi_get_range(device_, 0, 0, 0);
    int maxdata = comedi_get_maxdata(device_, 0, 0);
    double phys_val_[1000];
    for(int i = 0; i < expected_values_/blocks_; i++)
    {
      cout << "RAW value [" << i << "]: " << buf[i] << endl;
      phys_val_[i] = comedi_to_phys(buf[i], range_info, maxdata);
      cout << "Physical value [" << i << "]: " << phys_val_[i] << endl;
    }
//  if (error){
//  for(int i = 0; i < (blocks_*nr_ch_); i++)
//    cout << " " << buf[i];
//  }

  //  cout << "gBSamp: " << buf << endl;
  for(int i=0; i < expected_values_/blocks_; i++)
    for(int j=0; j < blocks_; j++)
      samples_[i+j] = phys_val_[i+j];

//  cout << "gBSamp: getSyncData -- samples.size() " << samples_.size() << endl;
//  cout << "sampleblock size: " << data_.getNrOfSamples() << endl;

  data_.setSamples(samples_);
//  cout << "sampleblock size: " << data_.getNrOfSamples() << endl;

  //cout << "gBSamp: getSyncData" << endl;
  samples_available_ = false;
  lock.unlock();

  //cout << "getSyncData called" << endl;
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> gBSamp::getAsyncData()
{
  #ifdef DEBUG
    cout << "gBSamp: getAsyncData" << endl;
  #endif

  if(first_run_)
  {
    cout << "gBSamp: Command-Test 1" << endl;
    error = comedi_command_test(device_, &comedi_cmd_);
    if(error != 0)
    {
      cerr << "gBSamp: comedi_command_test - Error nr " << error  << endl;
    }
    cout << "gBSamp: Command-Test 2" << endl;
    error = comedi_command_test(device_, &comedi_cmd_);
    if(error != 0)
    {
      cerr << "gBSamp: comedi_command_test - Error nr " << error  << endl;
    }
    cout << "gBSamp: comediCommand" << endl;
    error = comedi_command(device_, &comedi_cmd_);
    if (error < 0)
      comedi_perror("gBSamp: comedi_command");
    first_run_ = 0;
  }

  sampl_t buf[10000];
  read(comedi_fileno(device_),buf,sizeof(sampl_t));

  for(int i=0; i < expected_values_/blocks_; i++)
    for(int j=0; j < blocks_; j++)
      samples_[i+j] = buf[i+j];

  boost::shared_lock<boost::shared_mutex> lock(rw_);
  data_.setSamples(samples_);
  samples_available_ = false;
  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

//void gBSamp::stopDAQ(boost::int32_t error, TaskHandle taskHandle, char errBuff[2048])
//{
//
//}

//-----------------------------------------------------------------------------

int gBSamp::initCard()
{
  device_ = comedi_open("/dev/comedi0");
  if(device_==0)
  {
      cerr << "Error: " << comedi_strerror( comedi_errno() ) << endl;
      return -1;
  }
  else
      cout << "gBSamp: device opened!" << endl;

  comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

  //TODO: channel list
//  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  unsigned int channel_list[nr_ch_];
  for(int i=0; i < nr_ch_; i++)
      channel_list[i] = CR_PACK(i, 0, AREF_GROUND);

  // prepare for comedi_command (if asynchronous read)
//  unsigned int channel_list[1];
//  channel_list[0] = CR_PACK(0, 10, AREF_GROUND);
//  channel_list[1] = CR_PACK(1, 10, AREF_GROUND);

//  int flags = comedi_get_subdevice_flags(device_, 0);
//  cout << "Flags: " << flags << endl;
  comedi_cmd_.subdev = SUBDEVICE_FOR_ANALOG_INPUT;
//  comedi_cmd_.flags = TRIG_WAKE_EOS;
  comedi_cmd_.flags = TRIG_ROUND_NEAREST | TRIG_WAKE_EOS;
//  comedi_cmd_.flags = TRIG_RT;

  comedi_cmd_.start_src = TRIG_NOW;
  comedi_cmd_.start_arg = 0;
  comedi_cmd_.scan_begin_src = TRIG_TIMER;
  comedi_cmd_.scan_begin_arg = 1e6;
//  comedi_cmd_.scan_begin_arg = 2048000;
//  comedi_cmd_.scan_begin_arg = 976562;
  comedi_cmd_.convert_src = TRIG_TIMER;
  comedi_cmd_.convert_arg = 1;
//  comedi_cmd_.convert_arg = 800;
  comedi_cmd_.scan_end_src = TRIG_COUNT;
  comedi_cmd_.scan_end_arg = nr_ch_;
  comedi_cmd_.stop_src = TRIG_NONE;
  comedi_cmd_.stop_arg = 0;
  comedi_cmd_.chanlist = channel_list;
  comedi_cmd_.chanlist_len = nr_ch_;

  // prepare for comedi_insn (if synchronous read)
//  comedi_insn insn[2];
//  lsampl_t data_buf[fs_ ];

//  insn_list_.n_insns=2;
//  insn_list_.insns=insn;
//
//  for(int i=0; i < nr_ch_; i++)
//  {
//    insn[i].subdev = 0;
//    insn[i].insn = INSN_READ;
//    insn[i].data = data_buf_;
//    insn[i].chanspec = CR_PACK(i,10,AREF_GROUND);
//    insn[i].n = fs_;
//  }

//    insn[0].subdev = SUBDEVICE_FOR_ANALOG_INPUT;
//    insn[0].insn = INSN_READ;
//    insn[0].data = data_buf_;
//    insn[0].chanspec = CR_PACK(0,10,AREF_GROUND);
//    insn[0].n = fs_;
//    insn[1].subdev = 0;
//    insn[1].insn = INSN_READ;
//    insn[1].data = data_buf_;
//    insn[1].chanspec = CR_PACK(1,10,AREF_GROUND);
//    insn[1].n = fs_;

//  error = comedi_do_insnlist(device_, &insn_list_);
//  error = comedi_do_insn(device_, &insn[0]);
//  if(error<0)
//    comedi_perror("gBSamp: comedi_do_insn");
//  error = comedi_do_insn(device_, &insn[1]);
//  if(error<0)
//    comedi_perror("gBSamp: comedi_do_insn");

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
