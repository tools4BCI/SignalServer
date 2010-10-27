#include "hardware/sine_generator.h"

#include <math.h>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

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


const HWThreadBuilderTemplateRegistrator<SineGenerator> SineGenerator::factory_registrator_ ("sinegen", "sinegenerator");

//-----------------------------------------------------------------------------

SineGenerator::SineGenerator(boost::asio::io_service& io, XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
: HWThread(parser), acquiring_(0), current_block_(0), td_(0)
{
  #ifdef DEBUG
    cout << "SineGenerator: Constructor" << endl;
  #endif

  setHardware(hw);

  step_ = 1/static_cast<float>(fs_);
  cycle_dur_ = 1/static_cast<float>(fs_);
  boost::posix_time::microseconds period(1000000/fs_);
  td_ += period;

  buffer_.init(blocks_ , nr_ch_ , channel_types_);
  data_.init(blocks_ , nr_ch_ , channel_types_);

  samples_.resize(nr_ch_ ,0);
  t_ = new boost::asio::deadline_timer(io, td_);

  cout << " * SineGenerator sucessfully initialized" << endl;
  cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_  << ", blocksize: " << blocks_  << endl;

}

//-----------------------------------------------------------------------------

void SineGenerator::run()
{
  #ifdef DEBUG
    cout << "SineGenerator: run" << endl;
  #endif

  running_ = 1;
  genSine();
  cout << " * SineGenerator sucessfully started" << endl;
}


//-----------------------------------------------------------------------------

void SineGenerator::stop()
{
  #ifdef DEBUG
    cout << "SineGenerator: stop" << endl;
  #endif

  running_ = 0;
  cond_.notify_all();
  cout << " * SineGenerator sucessfully stopped" << endl;
}

//-----------------------------------------------------------------------------

SampleBlock<double> SineGenerator::getSyncData()
{
  #ifdef DEBUG
    cout << "SineGenerator: getSyncData" << endl;
  #endif

  if(!acquiring_)
    acquiring_ = 1;

  boost::unique_lock<boost::mutex> syn(sync_mut_);
  while(!samples_available_ && running_)
    cond_.wait(syn);
  boost::shared_lock<boost::shared_mutex> lock(rw_);
  samples_available_ = false;
  lock.unlock();
  cond_.notify_all();
  syn.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> SineGenerator::getAsyncData()
{
  #ifdef DEBUG
    cout << "SineGenerator: getAsyncData" << endl;
  #endif
  boost::shared_lock<boost::shared_mutex> lock(rw_);
  samples_available_ = false;
  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

void SineGenerator::genSine()
{
  #ifdef DEBUG
    cout << "SineGenerator: genSine" << endl;
  #endif

  for(uint8_t n = 0; n < nr_ch_ ; n++)
    samples_[n] = sin(step_ * 2 * PI + n/4);

  (step_ < 1-cycle_dur_ ? step_ += cycle_dur_ : step_ = 0);
  t_->expires_at(t_->expires_at() + td_);

  if(blocks_  == 1)
  {
    boost::unique_lock<boost::shared_mutex> lock(rw_);
    boost::unique_lock<boost::mutex> syn(sync_mut_);
    samples_available_ = true;
    data_.setSamples(samples_);
    lock.unlock();
    cond_.notify_all();
    if(isMaster() && acquiring_)
    {
      cond_.wait(sync_mut_);
      // if( !cond_.timed_wait(sync_mut_, td_))
      //   cerr << "Warning: New data was not fetched fast enough!" << endl;
      //   throw std::runtime_error("SineGenerator::genSine() -- Timeout; New data was not fetched fast enough!");
    }
    syn.unlock();
  }
  else
  {
    buffer_.appendBlock(samples_);
    current_block_++;

    if(current_block_ == blocks_ )
    {
      boost::unique_lock<boost::shared_mutex> lock(rw_);
      boost::unique_lock<boost::mutex> syn(sync_mut_);
      samples_available_ = true;
      data_ = buffer_;
      lock.unlock();
      cond_.notify_all();
      buffer_.reset();
      current_block_ = 0;
      if(isMaster() && acquiring_)
      {
        cond_.wait(sync_mut_);
        // if( !cond_.timed_wait(sync_mut_, td_))
        //   cerr << "Warning: New data was not fetched fast enough!" << endl;
        //   throw std::runtime_error("SineGenerator::genSine() -- Timeout; New data was not fetched fast enough!");
      }
      syn.unlock();
    }
  }
  if(running_)
    t_->async_wait(boost::bind(&SineGenerator::genSine, this));
}

//-----------------------------------------------------------------------------

void SineGenerator::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "SineGenerator: setHardware" << endl;
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
}

//-----------------------------------------------------------------------------

void SineGenerator::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "SineGenerator: setDeviceSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_fs,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(cst_.hw_channels,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(cst_.hw_buffer,false);
  if(elem != elem.end())
    setBlocks(elem);
}

//---------------------------------------------------------------------------------------

void SineGenerator::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "SineGenerator: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_sel,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

} // Namespace tobiss
