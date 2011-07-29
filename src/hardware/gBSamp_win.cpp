#ifdef WIN32

#include "hardware/gBSamp_win.h"
//#include "extern/include/nidaqmx/nidaqmx.h"
#include <Windows.h>

namespace tobiss
{
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;


using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<gBSamp> gBSamp::factory_registrator_ ("gbsamp", "g.bsamp");


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// THIS IS A VERY VERY BAD CONSTRUCT!!!
// NEVER EVER USE DEFINE-STUFF FOR FUNCTIONS-LIKE BEHAVIOUR

#define DAQmxErrChk(functionCall)\
  if( DAQmxFailed(error_=(functionCall)) )\
   {stopDAQ(error_, errBuff); return(-1);}\
  else
  
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

gBSamp::gBSamp(ticpp::Iterator<ticpp::Element> hw)
: acquiring_(0), current_block_(0)
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

  if( taskHandle_!=0 )
  {
    DAQmxStopTask(taskHandle_);
    DAQmxClearTask(taskHandle_);
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
  DAQmxErrChk (DAQmxStartTask(taskHandle_));
  return error_;
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

  DAQmxReadAnalogF64(taskHandle_,blocks_,-1,DAQmx_Val_GroupByChannel,recv_buffer,1000,&read,NULL);

  //  What is read used for?? -- it is just initialized to 0 and then ignored!


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

  throw(std::runtime_error("gBSamp::getAsyncData -- Async data acquisition not available for g.BSamp yet!"));
}

//-----------------------------------------------------------------------------

void gBSamp::stopDAQ(boost::int32_t error, char errBuff[2048])
{
  if( DAQmxFailed(error) )
    DAQmxGetExtendedErrorInfo(errBuff,2048);
  if( taskHandle_!=0 )
  {
    DAQmxStopTask(taskHandle_);
    DAQmxClearTask(taskHandle_);
  }
  if( DAQmxFailed(error) )
    cout << "DAQmx Error: " << errBuff << endl;
}

//-----------------------------------------------------------------------------

int gBSamp::initCard()
{
  error_=0;
  taskHandle_=0;
  read = 0;  //  What is read used for?? -- it is just initialized to 0 and then ignored!
  errBuff[0]='\0';

  //TODO: make a list with needed channels
  // now just uses first channels
  const char channel_list[] = "Dev1/ai0";

  // DAQmx Configure Code
  DAQmxErrChk (DAQmxCreateTask("",&taskHandle_));
  DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle_,channel_list,"",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
  //  DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle, NULL, fs_, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1));
  DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle_, NULL, fs_, DAQmx_Val_Rising, DAQmx_Val_HWTimedSinglePoint, 1));
  //DAQmx_Val_HWTimedSinglePoint

  return error_;
}

//-----------------------------------------------------------------------------


} // Namespace tobiss

#endif // WIN32
