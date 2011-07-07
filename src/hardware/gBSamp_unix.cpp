#include "hardware/gBSamp_unix.h"
#include "hardware/hw_thread_builder.h"
#include "tia/constants.h"
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
using std::pair;
using std::cerr;

//-----------------------------------------------------------------------------
const string gBSamp::hardware("hardware");
const string gBSamp::hardware_name("name");
const string gBSamp::hardware_version("version");
const string gBSamp::hardware_serial("serial");
const string gBSamp::hw_mode("mode");
const string gBSamp::hw_ds("device_settings");
const string gBSamp::hw_fs("sampling_rate");
const string gBSamp::hw_channels("measurement_channels");
const string gBSamp::hw_ch_nr("nr");
const string gBSamp::hw_ch_names("names");
const string gBSamp::hw_ch_type("type");
const string gBSamp::hw_buffer("blocksize");
const string gBSamp::hw_fil("filter");
const string gBSamp::hw_fil_type("type");
const string gBSamp::hw_fil_order("order");
const string gBSamp::hw_fil_low("f_low");
const string gBSamp::hw_fil_high("f_high");
const string gBSamp::hw_fil_sense("sense");
const string gBSamp::hw_notch("notch");
const string gBSamp::hw_notch_center("f_center");
const string gBSamp::hw_cs("channel_settings");
const string gBSamp::hw_sel("selection");
const string gBSamp::hw_cs_ch("ch");
const string gBSamp::hw_cs_nr("nr");
const string gBSamp::hw_cs_name("name");
//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<gBSamp> gBSamp::factory_registrator_ ("gbsamp", "g.bsamp");

#define SUBDEVICE_AI 0 //subdevice for analog input
#define PREF_RANGE  0 //range -10V - +10V
static const int buf_size = 1000;
//-----------------------------------------------------------------------------

//gBSamp::gBSamp(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
//: HWThread(parser), acquiring_(0), current_block_(0), first_run_(1)
gBSamp::gBSamp(ticpp::Iterator<ticpp::Element> hw)
: HWThread(), acquiring_(0), current_block_(0), first_run_(1)
{
  #ifdef DEBUG
    cout << "gBSamp: Constructor" << endl;
  #endif

  setType("g.BSamp");
  setHardware(hw);

  expected_values_ = nr_ch_ * blocks_;

  buffer_.init(blocks_ , nr_ch_ , channel_types_);
  data_.init(blocks_, nr_ch_, channel_types_);
  samples_.resize(expected_values_, 0);

  if( initCard() != 0)
    throw(std::invalid_argument("gBSamp: not initialized, please restart") );

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
//    comedi_cancel(device_, 0);
    comedi_close(device_);
  }
  delete [] channel_list_;
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

  if(device_)
  {
//    comedi_cancel(device_, 0);
    comedi_close(device_);
  }

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
  sampl_t buf[buf_size];

  comedi_get_buffer_contents(device_,0);
  int temp = expected_values_;
  int bytes_read = 0;
  int samples_read = 0;
  while(temp>0)
  {
    bytes_read = read(comedi_fileno(device_), buf+samples_read, temp*sizeof(sampl_t));
    temp -= bytes_read/sizeof(sampl_t);
    samples_read += bytes_read/sizeof(sampl_t);
//    if((bytes_read < 0) || (bytes_read % 2 != 0)) cout << "Error: Bytes read are " << bytes_read << endl;
  }

  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  int k=0;
  for(int i=0; i < nr_ch_; it++)
  {
    range_info_ = comedi_get_range(device_, SUBDEVICE_AI, (it->first - 1), 0);
    maxdata_ = comedi_get_maxdata(device_, SUBDEVICE_AI, (it->first - 1));
    for(int j = 0; j <  blocks_; j++)
    {
     samples_[k++] = comedi_to_phys(buf[j*nr_ch_+i], range_info_, maxdata_);
    }
    i++;
  }

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
  sampl_t buf[buf_size];

  comedi_get_buffer_contents(device_,0);
  int temp = expected_values_;
  int bytes_read = 0;
  int samples_read = 0;
  while(temp>0)
  {
    bytes_read = read(comedi_fileno(device_), buf+samples_read, temp*sizeof(sampl_t));
    temp -= bytes_read/sizeof(sampl_t);
    samples_read += bytes_read/sizeof(sampl_t);
//    if((bytes_read < 0) || (bytes_read % 2 != 0)) cout << "Error: Bytes read are " << bytes_read << endl;
  }

  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  int k=0;
  for(int i=0; i < nr_ch_; it++)
  {
    range_info_ = comedi_get_range(device_, SUBDEVICE_AI, (it->first - 1), 0);
    maxdata_ = comedi_get_maxdata(device_, SUBDEVICE_AI, (it->first - 1));
    for(int j = 0; j <  blocks_; j++)
    {
     samples_[k++] = comedi_to_phys(buf[j*nr_ch_+i], range_info_, maxdata_);
    }
    i++;
  }

  data_.setSamples(samples_);
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

  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  channel_list_ = new unsigned int[nr_ch_];

  for(int i=0; i < nr_ch_; it++)
  {
    int temp_channel = (it->first - 1);
    channel_list_[i] = CR_PACK(temp_channel, PREF_RANGE, AREF_GROUND);
    i++;
  }

  comedi_cmd_.subdev = 0;
  comedi_cmd_.flags = TRIG_ROUND_NEAREST | TRIG_WAKE_EOS;

  comedi_cmd_.start_src = TRIG_NOW;
  comedi_cmd_.start_arg = 0;

  comedi_cmd_.scan_begin_src = TRIG_TIMER;
  comedi_cmd_.scan_begin_arg = (1e9/fs_);

  comedi_cmd_.convert_src = TRIG_TIMER;
  comedi_cmd_.convert_arg = (1e9/(fs_*nr_ch_));

  comedi_cmd_.scan_end_src = TRIG_COUNT;
  comedi_cmd_.scan_end_arg = nr_ch_;

  comedi_cmd_.stop_src = TRIG_NONE;
  comedi_cmd_.stop_arg = 0;

  comedi_cmd_.chanlist = channel_list_;
  comedi_cmd_.chanlist_len = nr_ch_;

  int i=10;
  while(first_run_ && i>0)
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
      cout << "gBSamp: Error for comedi_command: " << error << endl;
      first_run_=1;
      i--;
    }
    if(!first_run_)
      cout << "gBSamp: comedi_command successful" << endl;
  }

  return error;
}

//-----------------------------------------------------------------------------

void gBSamp::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "gBSamp: setHardware" << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_ds, true));

  setDeviceSettings(ds);

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_cs, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_cs)
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
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

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(hw_channels,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_buffer,false);
  if(elem != elem.end())
    setBlocks(elem);

  ticpp::Iterator<ticpp::Element> filter(father->FirstChildElement(hw_fil, false));
  if (filter != filter.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(filter); ++it != it.end(); )
      if(it->Value() == hw_cs)
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

  if(!elem.Get()->HasAttribute(hw_fil_type))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, filter type ("+hw_fil_type+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_notch))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, filter order ("+hw_notch+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_low))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, lower cutoff frequency ("+hw_fil_low+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_high))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, upper cutoff frequency ("+hw_fil_high+") not given!";
    throw(ticpp::Exception(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_fil_sense))
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, parameter ("+hw_fil_sense+") not given!";
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

  Constants cst;
  type = cst.getSignalFlag(elem.Get()->GetAttribute(hw_fil_type));
  try
  {
    notch = equalsOnOrOff(elem.Get()->GetAttribute(hw_notch));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "gBSamp: Error -- ";
    ex_str += "Tag <"+hw_fil+"> given, but notch is not 'on' or 'off'";
    throw(ticpp::Exception(ex_str));
  }
  if(type == SIG_EEG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != .05) || (f_low != .1))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.05' or '.1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EOG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .5) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.5' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 30) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '30' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != .1) || (f_low != 1))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.1' or '1'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_EMG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != 1) || (f_low != 5))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '1' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
  }

  if(type == SIG_ECG)
  {
    try
    {
      f_low  = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_low));
      if((f_low != .01) || (f_low != 2))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '0.01' or '2'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      f_high = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_high));
      if((f_high != 60) || (f_high != 100))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '60' or '100'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
      throw(ticpp::Exception(ex_str));
    }
    try
    {
      sense = lexical_cast<float>(elem.Get()->GetAttribute(hw_fil_sense));
      if((f_low != 2) || (f_low != 5))
      {
        string ex_str;
        ex_str = "gBSamp: Error -- ";
        ex_str += "Tag <"+hw_fil+"> given, but lower filter is not '2' or '5'";
        throw(ticpp::Exception(ex_str));
      }
    }
    catch(bad_lexical_cast &)
    {
      string ex_str;
      ex_str = "gBSamp: Error -- ";
      ex_str += "Tag <"+hw_fil+"> given, but lower filter is not given";
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

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_sel,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

} // Namespace tobiss
