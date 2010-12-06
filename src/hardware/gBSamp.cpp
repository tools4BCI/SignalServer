#ifdef WIN32

#include "hardware/gBSamp.h"
#include "extern/include/nidaqmx/nidaqmx.h"

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

#define DAQmxErrChk(functionCall)\
  if( DAQmxFailed(error=(functionCall)) )\
   {stopDAQ(error, taskHandle, errBuff); return(-1);}\
  else

//-----------------------------------------------------------------------------

gBSamp::gBSamp(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
: HWThread(parser), acquiring_(0), current_block_(0)
{
  #ifdef DEBUG
    cout << "gBSamp: Constructor" << endl;
  #endif

  #pragma comment(lib,"NIDAQmx.lib")
  
  setHardware(hw);

  expected_values_ = nr_ch_ * blocks_;
  
  DWORD driver_buffer_size_ = expected_values_ * sizeof(float);
  
  data_buffer.resize(driver_buffer_size_,0);

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
  
	if( taskHandle!=0 )
	{
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
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
    DAQmxGetExtendedErrorInfo(errBuff,2048);

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
	DAQmxErrChk (DAQmxStartTask(taskHandle));
	//DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,blocks_,0,DAQmx_Val_GroupByChannel,data,10000,&read,NULL));

//	cout << "Data read: " << data << endl;
	
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

  //cout << "gBSamp: getSyncData" << endl;
  
  if(!acquiring_)
    acquiring_ = 1;

  //boost::unique_lock<boost::mutex> syn(sync_mut_);
  //while(!samples_available_ && running_)
  //  cond_.wait(syn);
  boost::shared_lock<boost::shared_mutex> lock(rw_);
  
  DAQmxReadAnalogF64(taskHandle,blocks_,-1,DAQmx_Val_GroupByChannel,data,10000,&read,NULL);

  for(int i=0; i < expected_values_/blocks_; i++)
    for(int j=0; j < blocks_; j++)
      samples_[i+j] =data[i+j];
      
  //cout << "gBSamp: getSyncData -- samples.size() " << samples_.size() << endl;
  //cout << "sampleblock size: " << data_.getNrOfSamples() << endl;
      
  data_.setSamples(samples_);

  //cout << "gBSamp: getSyncData" << endl;
  samples_available_ = false;
  lock.unlock();
  //cond_.notify_all();
  //syn.unlock();
  
  //cout << "getSyncData called" << endl; 
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> gBSamp::getAsyncData()
{
  #ifdef DEBUG
    cout << "gBSamp: getAsyncData" << endl;
  #endif
  boost::shared_lock<boost::shared_mutex> lock(rw_);
  samples_available_ = false;
  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

void gBSamp::stopDAQ(boost::int32_t error, TaskHandle taskHandle, char errBuff[2048])
{
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 )
	{
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
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
  // now just uses all 16 channels
  const char channel_list[] = "Dev1/ai0";
  
	// DAQmx Configure Code
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,channel_list,"",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,NULL,fs_,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1));

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

#endif // WIN32