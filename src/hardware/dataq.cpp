/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

/**
* @file dataq.cpp
**/

#include "hardware/dataq.h"
#include "dataq/err_codes.h"


#include <boost/asio/error.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/current_function.hpp>
#include <math.h>
#include <iostream>

using boost::lexical_cast;
using boost::bad_lexical_cast;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

namespace tobiss
{

const unsigned int DataQ::DMA_IN_SIZ = 0xF000;
const unsigned int DataQ::RING_BUFFER_SIZE = 512;

const std::string DataQ::hw_dataq_range_("range");
const std::string DataQ::hw_dataq_bipolar_("bipolar");
const std::string DataQ::hw_dataq_value_("value");

static const unsigned int   DI_720_MAX_SUPPORTED_CH  = 32;
static const unsigned int   DI_720_MIN_BURST_COUNT   = 80;  // --> 200000 Hz  (64 lt. DataQ SDK ... 250000Hz)
static const unsigned int   DI_720_MAX_BURST_COUNT   = 32000; // --> 500 Hz   (32767 lt. DataQ SDK ... 488Hz)
static const unsigned int   DI_720_BURST_RATE_VALUE  = 16000000;
static const unsigned int   DI_720_NR_VOLTAGE_RANGES = 4;
static const double         DI_720_VOLTAGE_RANGES[DI_720_NR_VOLTAGE_RANGES] = {10, 5, 2.5, 1.25};

const HWThreadBuilderTemplateRegistratorWithoutIOService<DataQ> DataQ::factory_registrator_ ("DI-720");

//-----------------------------------------------------------------------------

DataQ::DataQ(ticpp::Iterator<ticpp::Element> hw)
  : acq_thread_(0), dataq_dev_ptr_(0), dev_nr_(0), dev_buffer_(0), device_opened_(0),
    acqu_running_(0), burst_rate_count_(0), last_pos_(0), inlist_ptr_(0),
    buffer_(RING_BUFFER_SIZE), sleep_timer_(io_service_),  sleep_time_(0)
{
  #ifdef DEBUG
    std::cout << "DataQ: Constructor" << std::endl;
  #endif


  dataq_dev_ptr_ = new DataQWrapper("di720nt.dll");

  setType("DataQ -- DI-720");
  gain_index_.resize(1,0);
  bipolar_list_.resize(1,0);
  v_max_.resize(1, DI_720_VOLTAGE_RANGES[0] );
  v_min_.resize(1, DI_720_VOLTAGE_RANGES[0] *(-1) );
  setHardware(hw);

  burst_rate_count_ = DI_720_BURST_RATE_VALUE/(nr_ch_ * fs_);
  if(burst_rate_count_ > DI_720_MAX_BURST_COUNT)
    throw(std::invalid_argument("Burst rate too low -- choose a sampling rate > "
            + boost::lexical_cast<std::string>(DI_720_BURST_RATE_VALUE/(nr_ch_*DI_720_MAX_BURST_COUNT))
            +" or increase the number of channels!"));

  if(burst_rate_count_ < DI_720_MIN_BURST_COUNT)
    throw(std::invalid_argument("Burst rate too high -- choose a sampling rate < "
            + boost::lexical_cast<std::string>(DI_720_BURST_RATE_VALUE/(nr_ch_*DI_720_MIN_BURST_COUNT))
            +" or reduce the number of channels!"));

  sleep_time_ = boost::posix_time::microseconds(1000000/fs_);

  try
  {
    openDev(true);
    setInList();
    setDaqMode();
    //dataq_dev_ptr_->di_maximum_rate(40000);

    struct di_outlist_struct outlist[16] = {0};
    checkErrorCode( dataq_dev_ptr_->di_outlist(outlist), true );

    //std::cout << "  Burst rate: " << DI_720_BURST_RATE_VALUE/burst_rate_count_ << ", Burst rate count: " << burst_rate_count_ << ",  Fs: " << fs_ << std::endl;
    checkErrorCode( dataq_dev_ptr_->di_burst_rate(burst_rate_count_), true );

    dev_buffer_ = dataq_dev_ptr_->di_buffer_alloc(0, DMA_IN_SIZ);
    if(!dev_buffer_)
    {
      closeDev(true);
      throw(std::runtime_error("Failed to allocate buffer!"));
    }
    VirtualLock(dev_buffer_,(unsigned)DMA_IN_SIZ*2);

    #ifdef DEBUG
      //checkErrorCode(dataq_dev_ptr_->di_info( &info_ ), true);
      //printDataQInfo();
    #endif

    data_.init(blocks_ , nr_ch_ , channel_types_);
  }
  catch(...)
  {
    closeDev(false);
    throw;
  }
}

//-----------------------------------------------------------------------------

DataQ::~DataQ()
{
  #ifdef DEBUG
    std::cout << "DataQ: Destructor" << std::endl << std::flush;
  #endif

  VirtualLock(dev_buffer_,(unsigned)DMA_IN_SIZ*2);
  checkErrorCode(dataq_dev_ptr_->di_buffer_free(0), false);
  dev_buffer_ = 0;

  closeDev(false);
  if(dataq_dev_ptr_)
    delete dataq_dev_ptr_;
  dataq_dev_ptr_ = 0;

  if(inlist_ptr_)
    delete[] inlist_ptr_;
  inlist_ptr_ = 0;

}

//-----------------------------------------------------------------------------

SampleBlock<double> DataQ::getSyncData()
{
  #ifdef DEBUG
    std::cout << "DataQ: getSyncData" << std::endl << std::flush;
  #endif

  while((buffer_.getNumAvail() < (nr_ch_*blocks_) ) && acqu_running_)
    Sleep(1);
  //  {
  //    sleep_timer_.expires_from_now(sleep_time_, error_);
  //    if(error_)
  //      std::cerr << BOOST_CURRENT_FUNCTION << std::endl;

  //    sleep_timer_.wait();
  //  }

  if(!acqu_running_)
    return(data_);

  std::vector<double> vals(nr_ch_);
  short val = 0;
  data_.reset();
  for(unsigned int m = 0; m < blocks_; m++)
  {
    for(unsigned int n = 0; n < nr_ch_; n++)
    {
      buffer_.getNext_blocking(&val);
      vals[n] = getVoltageValue(val, n);
      //file_stream_ << vals[n] << ", ";
    }

    data_.appendBlock(vals,1);
  }
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> DataQ::getAsyncData()
{
  throw(std::runtime_error("DataQ::getAsyncData -- Asynchronous operation as slave not supported yet!"));
  return(data_);
}

//-----------------------------------------------------------------------------

void DataQ::run()
{
  #ifdef DEBUG
    std::cout << "DataQ: run()" << std::endl;
  #endif

  //  struct di_inlist_struct inlist[256] = {0};
  //for(int i = 0; i < 1; i++)
  //{
  //  inlist[i].chan = i;
  //  inlist[i].gain = 0;
  //  inlist[i].ave = 1;
  //  inlist[i].counter = 400 - 1;
  //}

  //struct di_outlist_struct outlist[16] = {0};
  //checkErrorCode( dataq_dev_ptr_->di_inlist(inlist), true );
  //checkErrorCode( dataq_dev_ptr_->di_outlist(outlist), true );

  startAcqu(true);
  acq_thread_ = new boost::thread( boost::bind(&DataQ::acquireData, this));
}

//-----------------------------------------------------------------------------

void DataQ::stop()
{
  #ifdef DEBUG
    std::cout << "DataQ: stop" << std::endl;
  #endif

  stopAcqu(true);
  //  boost::chrono::duration<long long, boost::nano> dur = boost::chrono::system_clock::now() - chrono_start_;

  //  boost::chrono::duration<double> sec = dur;

  //  std::cout << "   Spent " << dur << " ns acquiring from DataQ!" << std::endl;
  //  std::cout << "    ( sec: " << sec << " )" << std::endl;
  //  std::cout << "     ... acquired " << iteration_ << " samples from "<< nr_ch_ << " channel(s)." << std::endl;
  //  std::cout << "       --> estimated fs: " << iteration_/sec.count() << std::endl;
}

//-----------------------------------------------------------------------------

void DataQ::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "DataQ: setDeviceSettings" << std::endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs_,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  gain_index_.resize(nr_ch_,0);
  bipolar_list_.resize(nr_ch_,0);
  v_max_.resize(nr_ch_, DI_720_VOLTAGE_RANGES[0] );
  v_min_.resize(nr_ch_, DI_720_VOLTAGE_RANGES[0] *(-1) );

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);

  elem = father->FirstChildElement(hw_dataq_range_,false);
  if(elem != elem.end())
    setDeviceRange(elem);

  elem = father->FirstChildElement(hw_dataq_bipolar_,false);
  if(elem != elem.end())
    setDeviceBipolar(elem);

}

//-----------------------------------------------------------------------------

void DataQ::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "DataQ: setChannelSettings" << std::endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);


  gain_index_.resize(nr_ch_,gain_index_.at(0));
  bipolar_list_.resize(nr_ch_,bipolar_list_.at(0));
  v_max_.resize(nr_ch_, v_max_.at(0) );
  v_min_.resize(nr_ch_, v_min_.at(0) );

  elem = father->FirstChildElement(hw_dataq_range_,false);
  if(elem != elem.end())
    setChannelRange(elem);

  elem = father->FirstChildElement(hw_dataq_bipolar_,false);
  if(elem != elem.end())
    setChannelBipolar(elem);
}

//-----------------------------------------------------------------------------

void DataQ::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    std::cout << "DataQ: setHardware" << std::endl;
  #endif

  checkMandatoryHardwareTags(hw);
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));

  setDeviceSettings(ds);

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_chset_, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_chset_)
      {
        std::string ex_str(type_ + " -- ");
        ex_str += "Multiple channel_settings found!";
        throw(std::invalid_argument(ex_str));
      }
      setChannelSettings(cs);
  }
  checkChannels();
  checkBiploarConfig();
}

//-----------------------------------------------------------------------------

int DataQ::openDev(bool throw_on_error)
{
  #ifdef DEBUG
    std::cout << "DataQ: openDev" << std::endl;
  #endif

  if(device_opened_)
    return 0;

  int ec = checkErrorCode( dataq_dev_ptr_->di_open(dev_nr_), throw_on_error );
  device_opened_ = (!ec);
  return(ec);

  //  if( error_code == DI_ALREADY_OPEN_ERR)
  //  {
  //    std::cerr << "Device already running --> closing!" << std::endl;
  //    dataq_dev.di_close();
  //    checkErrorCode(dataq_dev, dataq_dev.di_open(0) );
  //    return;
  //  }
}

//-----------------------------------------------------------------------------

int DataQ::closeDev(bool throw_on_error)
{
  #ifdef DEBUG
    std::cout << "DataQ: closeDev" << std::endl;
  #endif

  if(!device_opened_)
    return 0;

  stopAcqu(throw_on_error);

  int ec = checkErrorCode( dataq_dev_ptr_->di_close(), throw_on_error );
  device_opened_ = (ec);
  return( ec );
}

//-----------------------------------------------------------------------------

int DataQ::startAcqu(bool throw_on_error)
{
  #ifdef DEBUG
    std::cout << "DataQ: startAcqu-- running: " << acqu_running_ << std::endl;
  #endif

  if( acqu_running_ )
    return 0;

  int ec = checkErrorCode(dataq_dev_ptr_->di_start_scan(), throw_on_error);
  acqu_running_ = (!ec);
  return( ec );
}

//-----------------------------------------------------------------------------

int DataQ::stopAcqu(bool throw_on_error)
{
  #ifdef DEBUG
    std::cout << "DataQ: stopAcqu -- running: " << acqu_running_ << std::endl;
  #endif

  if( !acqu_running_ )
    return 0;

  acqu_running_ = 0;
  acq_thread_->interrupt();

  while(buffer_.getNumAvail() )
    buffer_.dropOldest();


  Sleep(10);
  std::cout << "DataQ: stopAcqu -- waiting to join " << std::endl;
  acq_thread_->join();
  std::cout << "DataQ: stopAcqu -- joined " << std::endl;

  int ec = checkErrorCode(dataq_dev_ptr_->di_stop_scan(), throw_on_error);
  return( ec );
}

//-----------------------------------------------------------------------------

int DataQ::checkErrorCode(int error_code, bool throw_on_error)
{
  #ifdef DEBUG
    std::cout << "DataQ: checkErrorCode" << std::endl;
  #endif

  if( error_code == DI_NO_ERR)
    return error_code;

  char err_msg[256];
  dataq_dev_ptr_->di_strerr(error_code, err_msg);
  std::string ex_str( std::string("DataQ Error: ").append(err_msg) );

  if(error_code == DI_DRIVER_ERR)
    ex_str += " -- please check if the device is connected and turned on!";

  if(throw_on_error)
  {
    closeDev(false);
    throw(std::runtime_error( ex_str ) );
  }

  std::cerr << "  ###  " << ex_str << "  ###  " << std::endl;
  return error_code;
}

//-----------------------------------------------------------------------------

void DataQ::printDataQInfo()
{
  #ifdef DEBUG
    std::cout << "DataQ: printDataQInfo" << std::endl;
  #endif

  printf("\nPort: %04X\n",info_.port);
  printf("Input chn: %04X\n",info_.buf_in_chn);
  printf("Output chn: %04X\n",info_.buf_out_chn);
  printf("Sft lvl: %04X\n",info_.sft_lvl);
  printf("Hrd lvl: %04X\n",info_.hrd_lvl);
  printf("Input ptr: %08lX\n",info_.buf_in_ptr);
  printf("Output ptr: %08lX\n",info_.buf_out_ptr);
  printf("Input size: %04X\n",info_.buf_in_size);
  printf("Output size: %04X\n",info_.buf_out_size);
  printf("TSR Ver: %s\n",info_.tsr_version);
  printf("DSP Ver: %s\n",info_.dsp_version);
  printf("SDK Ver: %s\n",info_.sdk_version);
  printf("Serial #: %08lX\n",info_.serial_no);
  printf("Board ID: %s\n",info_.board_id);
  printf("PGH/PGL(1/0): %0x1\n",info_.pgh_pgl);
}

//-----------------------------------------------------------------------------

void DataQ::setInList()
{
  #ifdef DEBUG
    std::cout << "DataQ: setInList" << std::endl;
  #endif

  //struct di_inlist_struct inlist[256] = {0};
  //for(int i = 0; i < 2; i++)
  //{
  //  inlist[i].chan = i;
  //  inlist[i].gain = 0;
  //  inlist[i].ave = 0;
  //  inlist[i].counter = 0;
  //}

  inlist_ptr_ = new tobiss::di_inlist_struct[256];

  for(unsigned int n = 0; n < 256; n++)
  {
    inlist_ptr_[n].chan = 0;
    inlist_ptr_[n].diff = 0;
    inlist_ptr_[n].gain = 0;
    inlist_ptr_[n].unipolar = 0;
    inlist_ptr_[n].dig_out = 0;
    inlist_ptr_[n].dig_out_enable = 0;
    inlist_ptr_[n].ave = 0;
    inlist_ptr_[n].counter = 0;
  }

  unsigned int n = 0;
  for(std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::iterator it(channel_info_.begin());
      it != channel_info_.end(); it++)
  {
    inlist_ptr_[n].chan = it->first -1;
    inlist_ptr_[n].diff = bipolar_list_.at( getChannelPosition(it->first) );
    inlist_ptr_[n].gain = gain_index_.at( getChannelPosition(it->first) );
    inlist_ptr_[n].unipolar = 0;
    inlist_ptr_[n].dig_out = 0;
    inlist_ptr_[n].dig_out_enable = 0;
    inlist_ptr_[n].ave = 0;
    inlist_ptr_[n].counter = 0;
    n++;
  }

  checkErrorCode(dataq_dev_ptr_->di_list_length(nr_ch_, 0), true );
  checkErrorCode( dataq_dev_ptr_->di_inlist(inlist_ptr_), true );
}

//-----------------------------------------------------------------------------

void DataQ::setDaqMode()
{
  #ifdef DEBUG
    std::cout << "DataQ: setDaqMode" << std::endl;
  #endif

  mode_.mode = 0;
  mode_.hystx = 0;
  mode_.scnx = 0;
  mode_.trig_level =  0; //0x01
  mode_.trig_slope = 0; //1
  mode_.trig_pre = 0;
  mode_.trig_post = 0; //0xFF0

  checkErrorCode(dataq_dev_ptr_->di_mode( &mode_ ), true );
}

//-----------------------------------------------------------------------------

inline double DataQ::getVoltageValue(short val, boost::uint16_t ch_nr)
{
  return( val *(v_max_[ch_nr] - v_min_[ch_nr])/65536 );
}

//-----------------------------------------------------------------------------

void DataQ::acquireData()
{
  #ifdef DEBUG
    std::cout << "DataQ: acquireData" << std::endl;
  #endif

  unsigned int current_pos = 0;
  unsigned int num_vals = 0;

  boost::system::error_code error;

  while(acqu_running_)
  {
    current_pos = dataq_dev_ptr_->di_buffer_status(0);

    if(last_pos_ != current_pos)
    {
      if(current_pos > last_pos_)
      {
        num_vals = current_pos - last_pos_;
        for(unsigned int n = 0; (n < num_vals) && acqu_running_; n++)
        {
          buffer_.insert_blocking( *(dev_buffer_ + last_pos_ + n) );
          //file_stream_ <<  getVoltageValue( *(dev_buffer_ + last_pos_ + n) ) << ",";
        }
      }
      else
      {
        num_vals = DMA_IN_SIZ - last_pos_;
        for(unsigned int n = 0; (n < num_vals) && acqu_running_; n++)
          if(last_pos_ + n > DMA_IN_SIZ-1)
            std::cerr << "Error - would read beyond the buffer -- " << DMA_IN_SIZ << ", " << last_pos_ + n << std::endl;
          else
          {
            buffer_.insert_blocking( *(dev_buffer_ + last_pos_ + n) );
            //file_stream_ <<  getVoltageValue( *(dev_buffer_ + last_pos_ + n) ) << ",";
          }

        for(unsigned int n = 0; (n < current_pos) && acqu_running_; n++)
        {
            buffer_.insert_blocking( *(dev_buffer_ + n) );
            //file_stream_ <<  getVoltageValue( *(dev_buffer_ + n) ) << ",";
        }
      }

      last_pos_ = current_pos;
    }
    else
    {
      Sleep(1);
      //      sleep_timer_.expires_from_now(sleep_time_, error);

      //      if(error)
      //        std::cerr << BOOST_CURRENT_FUNCTION << std::endl;
      //      sleep_timer_.wait();
    }

  } // while(acqu_running)
}

//-----------------------------------------------------------------------------

void DataQ::setDeviceRange(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "DataQ: setDeviceRange" << std::endl;
  #endif

  double value = 0;
  try{
    value = lexical_cast<double>(boost::format("%d") % father->GetText(true) );
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+ hw_dataq_range_ + "> : Value is not a floating point one!";
    throw(std::invalid_argument(ex_str));
  }

  unsigned int n = 0;
  for( ; n < DI_720_NR_VOLTAGE_RANGES; n++)
  {
    if(value == DI_720_VOLTAGE_RANGES[n])
      break;
  }

  if(n >= DI_720_NR_VOLTAGE_RANGES)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+ hw_dataq_range_ + "> : Invalid range value: "+ lexical_cast<string>(value) +
      + " -- check supported values in the HW manual!";
  }

  for (unsigned int n = 0; n < nr_ch_; n++)
  {
    gain_index_[n] = n;
    v_max_[n] = DI_720_VOLTAGE_RANGES[n];
    //if(bipolar_list_.at(n))
      v_min_[n] = (-1)*DI_720_VOLTAGE_RANGES[n];
    //else
    //  v_min_[n] = 0;
  }
}

//-----------------------------------------------------------------------------

void DataQ::setDeviceBipolar(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "DataQ: setDeviceBipolar" << std::endl;
  #endif

  bool bipolar = equalsOnOrOff(father->GetText(true));

  for (unsigned int n = 0; n < nr_ch_; n++)
  {
    bipolar_list_[n] = bipolar;
    if(bipolar)
      v_min_[n] = (-1)*v_max_[n];
  }
}

//-----------------------------------------------------------------------------

void DataQ::setChannelRange(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "DataQ: setChannelRange" << std::endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem = father->FirstChildElement(hw_chset_ch_,false);

  cout << " * DataQ -- individual ranges activated on channels:" << endl;

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_dataq_range_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      if(!elem.Get()->HasAttribute(hw_dataq_value_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_dataq_range_+"> given, but attribute ("+hw_dataq_value_+") (on/off) not given!";
        throw(std::invalid_argument(ex_str));
      }

      uint16_t ch = 0;
      try{
        ch = lexical_cast<boost::uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_dataq_range_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_dataq_range_ + "> - Channel "+
                  boost::lexical_cast<std::string>(ch) +" not set for recording!";
        throw(std::invalid_argument(ex_str));
      }

      double value = 0;
      try{
        value = lexical_cast<double>( boost::format("%d") % elem.Get()->GetAttribute(hw_dataq_value_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_dataq_range_ + "> : Value is not a floating point one!";
        throw(std::invalid_argument(ex_str));
      }

      unsigned int n = 0;
      for( ; n < DI_720_NR_VOLTAGE_RANGES; n++)
      {
        if(value == DI_720_VOLTAGE_RANGES[n])
          break;
      }


      if(n >= DI_720_NR_VOLTAGE_RANGES)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_dataq_range_ + "> : Invalid range value: "+ lexical_cast<string>(value) +
          + " -- check supported values in the HW manual!";
        throw(std::invalid_argument(ex_str));
      }


      gain_index_[ getChannelPosition(ch) ] = n;
      v_max_[ getChannelPosition(ch) ] = value;

      //if( bipolar_list_[ getChannelPosition(ch) ] )
        v_min_[ getChannelPosition(ch) ] = value * (-1);
      //else
      //  v_min_[ getChannelPosition(ch) ] = 0;

      cout << "  ... ch " << ch << " (pos " << getChannelPosition(ch) << "): " << value;
      cout <<"/"<< v_min_[ getChannelPosition(ch) ] << "; ";
    }
    else
      throw(std::invalid_argument("DataQ::setChannelRange -- Tag not equal to \""+hw_chset_ch_+"\"!"));
}

//-----------------------------------------------------------------------------

void DataQ::setChannelBipolar(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "DataQ: setChannelBipolar" << std::endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem = father->FirstChildElement(hw_chset_ch_,false);

  cout << " * DataQ -- bipolar recording activated on channels:" << endl;

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_dataq_bipolar_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      if(!elem.Get()->HasAttribute(hw_dataq_value_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_dataq_bipolar_+"> given, but attribute ("+hw_dataq_value_+") (on/off) not given!";
        throw(std::invalid_argument(ex_str));
      }

      uint16_t ch = 0;
      try{
        ch = lexical_cast<boost::uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_dataq_bipolar_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_dataq_bipolar_ + "> - Channel "+
                  boost::lexical_cast<std::string>(ch) +" not set for recording!";
        throw(std::invalid_argument(ex_str));
      }

      bool value = equalsOnOrOff( elem.Get()->GetAttribute(hw_dataq_value_) );

      bipolar_list_[ getChannelPosition(ch) ] = value;
      cout << "  ... ch " << ch << " (pos " << getChannelPosition(ch) << "): " << value << "; ";

    }
    else
      throw(std::invalid_argument("DataQ::setChannelBipolar -- Tag not equal to \""+hw_chset_ch_+"\"!"));
}

//-----------------------------------------------------------------------------

void DataQ::checkBiploarConfig()
{
  #ifdef DEBUG
    std::cout << "DataQ: checkBiploarConfig" << std::endl;
  #endif

  for(std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::iterator it(channel_info_.begin());
      it != channel_info_.end(); it++)
  {
    boost::uint16_t ch_nr = it->first;
    int ch_pos = getChannelPosition(ch_nr);

    if( (ch_nr < 9) || ( ch_nr > 16 && ch_nr < 25 ) )
    {
      //std::cout << "CH pos: " << ch_pos << ", "<< bipolar_list_.size() << std::endl << std::flush;
      if(bipolar_list_.at( ch_pos ) )
        if(channel_info_.find(ch_nr + 8) != channel_info_.end())
        {
          string ex_str(type_ + " -- ");
          ex_str += "Tag <"+ hw_dataq_bipolar_ + "> - Channel "+
                    boost::lexical_cast<std::string>(ch_nr +8) +" must not be set for recording in a bipolar measurement!";
          throw(std::invalid_argument(ex_str));
        }
    }
  }
}

//-----------------------------------------------------------------------------

void DataQ::checkChannels()
{
  #ifdef DEBUG
    std::cout << "DataQ: checkChannels" << std::endl;
  #endif

  for(std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::iterator it(channel_info_.begin());
      it != channel_info_.end(); it++)
    if(it->first > DI_720_MAX_SUPPORTED_CH)
    {
      string ex_str(type_ + " -- ");
      ex_str += "Too many channels, maximum supported number:  "+
                boost::lexical_cast<std::string>(DI_720_MAX_SUPPORTED_CH) +"!";
      throw(std::invalid_argument(ex_str));
    }
}

//-----------------------------------------------------------------------------

}   // tobiss

