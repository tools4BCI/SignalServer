#ifdef WIN32

#include "hardware/gBSamp.h"
//#include "extern/include/nidaqmx/nidaqmx.h"
#include "hardware/nidaqmx_wrapper.h"

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

//#define DAQmxErrChk(functionCall)\
//  if( DAQmxFailed(error=(functionCall)) )\
//   {stopDAQ(error, taskHandle, errBuff); return(-1);}\
//  else

//-----------------------------------------------------------------------------

gBSamp::gBSamp(ticpp::Iterator<ticpp::Element> hw)
: HWThread(), acquiring_(0), current_block_(0)
{
  #ifdef DEBUG
    cout << "gBSamp: Constructor" << endl;
  #endif

  #pragma comment(lib,"NIDAQmx.lib")

  cout << " dsffgfdsg " << endl;

  setHardware(hw);

  expected_values_ = nr_ch_ * blocks_;

  DWORD driver_buffer_size_ = expected_values_ * sizeof(float);

  data_buffer.resize(driver_buffer_size_,0);

  buffer_.init(blocks_ , nr_ch_ , channel_types_);
  data_.init(blocks_, nr_ch_, channel_types_);
  samples_.resize(expected_values_, 0);
  cout << " dsffgfdsg " << endl;
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

  if( taskHandle!=0 )
  {
    nidaqmx_.stopTask(taskHandle);
	nidaqmx_.clearTask(taskHandle);
  }

}

//-----------------------------------------------------------------------------

void gBSamp::run()
{
  #ifdef DEBUG
    cout << "gBSamp: run" << endl;
  #endif

  running_ = 1;

  if(readFromDAQCard() != 0)
	  nidaqmx_.getExtendedErrorInfo(errBuff,2048);

  cout << " * gBSamp sucessfully started" << endl;
}

//-----------------------------------------------------------------------------

void gBSamp::stop()
{
  #ifdef DEBUG
    cout << "gBSamp: stop" << endl;
  #endif

  running_ = 0;

  //if( taskHandle!=0 )
  //{
  //  DAQmxStopTask(taskHandle);
  //  DAQmxClearTask(taskHandle);
  //}

  cond_.notify_all();
  cout << " * gBSamp sucessfully stopped" << endl;
}

//-----------------------------------------------------------------------------

int gBSamp::readFromDAQCard()
{
	nidaqmx_.startTask(taskHandle);
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

    int32 nr_samples_received;
    float64 recv_buffer[1];

  boost::shared_lock<boost::shared_mutex> lock(rw_);

  nidaqmx_.readAnalogF64(taskHandle,blocks_,-1,DAQmx_Val_GroupByChannel,data,1000,&read,NULL);
  //DAQmxReadAnalogF64(taskHandle,1, -1, DAQmx_Val_GroupByChannel,
  //                   recv_buffer, 1, &nr_samples_received, NULL);
  




  for(int i=0; i < nr_ch_; i++)
    for(int j=0; j < blocks_; j++)
      samples_[i+j] =recv_buffer[i+j];

  //cout << "gBSamp: getSyncData -- samples.size() " << samples_.size() << endl;
  //cout << "sampleblock size: " << data_.getNrOfSamples() << endl;

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
  nidaqmx_.readAnalogF64(taskHandle,blocks_,-1,DAQmx_Val_GroupByChannel,data,10000,&read,NULL);

  for(int i=0; i < nr_ch_; i++)
    for(int j=0; j < blocks_; j++)
      samples_[i+j] =data[i+j];

  data_.setSamples(samples_);

  samples_available_ = false;
  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

void gBSamp::stopDAQ(boost::int32_t error, TaskHandle taskHandle, char errBuff[2048])
{
  if( DAQmxFailed(error) )
	  nidaqmx_.getExtendedErrorInfo(errBuff,2048);
  if( taskHandle!=0 )
  {
	  nidaqmx_.stopTask(taskHandle);
	  nidaqmx_.clearTask(taskHandle);
  }
  if( DAQmxFailed(error) )
    cout << "DAQmx Error: " << errBuff << endl;
}

//-----------------------------------------------------------------------------

int gBSamp::initCard()
{
  error=0;
  taskHandle=0;
  read = 0;
  errBuff[0]='\0';

  //TODO: make a list with needed channels
  // now just uses first channels
  //const char channel_list[] = "Dev1/ai0";

  std::stringstream str_of_channels;

  map<uint16_t, std::pair<string, uint32_t> >::iterator it = channel_info_.begin();

  for(int i = 0; i < nr_ch_; it++)
  {
	  int temp_channel = (it->first - 1);
	  str_of_channels << "Dev1/ai" << temp_channel;
	  i++;
  }

  char *channel_list;
  std::string chann = str_of_channels.str();
  channel_list = new char[(nr_ch_ * 8) + 1];
  strcpy(channel_list, chann.c_str());

  // DAQmx Configure Code
  nidaqmx_.createTask("",&taskHandle);
  nidaqmx_.createAIVoltageChan(taskHandle,channel_list,"",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL);
  //  DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle, NULL, fs_, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));
  nidaqmx_.cfgSampClkTiming(taskHandle, NULL, fs_, DAQmx_Val_Rising, DAQmx_Val_HWTimedSinglePoint, 1);
  //DAQmx_Val_HWTimedSinglePoint 

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

#endif // WIN32
