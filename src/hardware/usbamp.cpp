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
* @file usbamp.cpp
**/

#include "winsock2.h"
#include "hardware/usbamp.h"
#include "tia/constants.h"

#include <boost/bind.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace tobiss
{
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

using std::cout;
using std::cerr;
using std::endl;
using std::left;
using std::right;
using std::string;
using std::vector;
using std::map;
using std::pair;

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::algorithm::to_lower_copy;


std::set<std::string> USBamp::serials_;
bool USBamp::is_usbamp_master_(0);

USBamp* USBamp::master_device_(0);
std::vector<USBamp*>  USBamp::slave_devices_;

static const unsigned int USBAMP_MAX_NR_OF_AMPS   = 8;
static const unsigned int USBAMP_MAX_NR_OF_CHANNELS   = 17;
static const unsigned int USBAMP_NR_OF_CHANNEL_GROUPS = 4;
static const unsigned int USBAMP_NOTCH_HALF_WIDTH = 2;   // to one side  ...  e.g.  f_center = 50 Hz -->  48/52 Hz
static const unsigned int USBAMP_ERROR_MSG_SIZE = 256;
static const unsigned int USBAMP_NR_OF_OVERLAPPED = 8;


const HWThreadBuilderTemplateRegistratorWithoutIOService<USBamp> USBamp::FACTORY_REGISTRATOR_ ("usbamp", "g.usbamp");

const string USBamp::hw_filter_("filter");
const string USBamp::hw_filter_type_("type");
const string USBamp::hw_filter_order_("order");
const string USBamp::hw_filter_low_("f_low");
const string USBamp::hw_filter_high_("f_high");

const string USBamp::hw_notch_("notch");
const string USBamp::hw_notch_center_("f_center");

const string USBamp::hw_opmode_("operation_mode");
const string USBamp::hw_sc_("shortcut");
const string USBamp::hw_trigger_line_("trigger_line");
const string USBamp::hw_usbampmaster_("usbamp_master");
const string USBamp::hw_comgnd_("common_ground");
const string USBamp::hw_gnd_("gnd");
const string USBamp::hw_gnd_block_("block");
const string USBamp::hw_gnd_value_("value");

const string USBamp::hw_comref_("common_reference");
const string USBamp::hw_cr_("cr");
const string USBamp::hw_cr_block_("block");
const string USBamp::hw_cr_value_("value");

const string USBamp::hw_bipolar_("bipolar");
const string USBamp::hw_bipolar_with_("with");
const string USBamp::hw_drl_("driven_right_leg");
const string USBamp::hw_drl_value_("value");

//-----------------------------------------------------------------------------

template<typename T> inline bool isnan(T value)
{
  return value != value;
}

//-----------------------------------------------------------------------------

USBamp::USBamp(ticpp::Iterator<ticpp::Element> hw)
  : HWThread(), enable_sc_(0),
    external_sync_(0), trigger_line_(0), trigger_line_sig_type_(SIG_UNDEFINED),
    sample_count_(0), error_count_(0) ,error_code_(0), error_msg_(0),
    driver_buffer_size_(0), timeout_(0), expected_values_(0),
    first_run_(1), current_overlapped_(0),
    h_(0), nr_of_bp_filters_(0), bp_filters_(0), nr_of_notch_filters_(0),
    notch_filters_(0)

{
  #ifdef DEBUG
    cout << "USBamp: Constructor" << endl;
  #endif

  #pragma comment(lib,"gUSBamp.lib")

  usbamp_filter_types_.insert(pair <string,int>("chebyshev", 1));
  usbamp_filter_types_.insert(pair <string,int>("cheby", 1));
  usbamp_filter_types_.insert(pair <string,int>("butterworth", 2));
  usbamp_filter_types_.insert(pair <string,int>("butter", 2));

  usbamp_opModes_.insert(pair <string,string>("normal", "M_NORMAL"));
  usbamp_opModes_.insert(pair <string,string>("calibrate", "M_CALIBRATE"));
  usbamp_opModes_.insert(pair <string,string>("impedance", "M_IMPEDANCE"));

  usbamp_block_names_.insert(pair <string,int>("a", 0));
  usbamp_block_names_.insert(pair <string,int>("b", 1));
  usbamp_block_names_.insert(pair <string,int>("c", 2));
  usbamp_block_names_.insert(pair <string,int>("d", 3));

  setType("g.USBamp");

  ticpp::Iterator<ticpp::Element> elem(hw);
  ticpp::Iterator< ticpp::Attribute > attribute;

  for(attribute = attribute.begin(elem.Get()); attribute != attribute.end();
      attribute++)
    m_.insert(pair<string, string>(attribute->Name(), attribute->Value()));

  checkMandatoryHardwareTags(hw);

  ov_.resize(USBAMP_NR_OF_OVERLAPPED);
  data_Ev_.resize(USBAMP_NR_OF_OVERLAPPED);
  bytes_received_.resize(USBAMP_NR_OF_OVERLAPPED);

  getHandles();
  initFilterPtrs();
  setHardware(hw);

  expected_values_ = nr_ch_ * blocks_;

  driver_buffer_size_ = expected_values_ * sizeof(float) + HEADER_SIZE;

  for(unsigned int n = 0; n < ov_.size(); n++)
    driver_buffer_.push_back( new BYTE[driver_buffer_size_] );

  // raw_buffer_.reserve(USBAMP_SAMPLE_BUFFER_MULTIPLIER *
                        // (expected_values_ * sizeof(float)) );

  error_msg_ = new CHAR[USBAMP_ERROR_MSG_SIZE];

  data_.init(blocks_, nr_ch_, channel_types_);
  samples_.resize(expected_values_, 0);

  initUSBamp();

  if(external_sync_)
    slave_devices_.push_back(this);
  else
    master_device_ = this;

  cout << endl;
  cout << " --> g.USBamp " << serial_;
  cout << "  (driver version: " << usb_amp_.getDriverVersion ();
  cout << ", hardware version: " << usb_amp_.getHWVersion (h_) << ")" <<  endl;

  if(!homogenous_signal_type_)
  {
    cout << "   ... NOTICE: Device is acquiring different signal types" << endl;
    cout << "     --  ensure that reference and ground settings are correctly set!" << endl;
  }
}

//-----------------------------------------------------------------------------

USBamp::~USBamp()
{
  #ifdef DEBUG
    cout << "USBamp: Destructor" << endl;
  #endif

  delete[] bp_filters_;
  delete[] notch_filters_;
  for(unsigned int n = 0; n < driver_buffer_.size(); n++)
    delete[] driver_buffer_[n];
  delete[] error_msg_;
}

//-----------------------------------------------------------------------------

void USBamp::setHardware(ticpp::Iterator<ticpp::Element>const& hw)
{
  #ifdef DEBUG
    cout << "USBamp: setHardware" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));

  setDeviceSettings(ds);

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_chset_, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_chset_)
      {
        string ex_str(type_ + " -- Multiple channel_settings found!");
        throw(std::invalid_argument(ex_str));
      }
      setChannelSettings(cs);
  }

  checkTriggerLineChannel();
}

//-----------------------------------------------------------------------------

SampleBlock<double> USBamp::getSyncData()
{
  #ifdef DEBUG
    cout << "USBamp: getSyncData" << endl;
  #endif

  if(!running_)
  {
    cout << "Not running!" << endl;
    return(data_);
  }

  boost::unique_lock<boost::shared_mutex> lock(rw_);
  bytes_received_[current_overlapped_] = 0;

  if(!first_run_)
  {
    for(uint32_t n = 0; n < slave_devices_.size(); n++)
      slave_devices_[n]->callGT_GetData();
    callGT_GetData();
  }
  else
    first_run_ = false;

  for(uint32_t n = 0; n < slave_devices_.size(); n++)
    slave_devices_[n]->fillSyncBuffer();
  fillSyncBuffer();

  fillSampleBlock();
  for(uint32_t n = 0; n < slave_devices_.size(); n++)
    slave_devices_[n]->fillSampleBlock();

  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> USBamp::getAsyncData()
{
  #ifdef DEBUG
    cout << "USBamp: getAsyncData" << endl;
  #endif

  if(!running_)
  {
    // cout << "Not running!" << endl;
    return(data_);
  }

//   cout << " --> getting Slave data ..." << endl;
  if(external_sync_)
    return(data_);

  throw(std::runtime_error("USBamp::getAsyncData -- Async data acquisition not available for g.USBamp yet!"));

//   boost::shared_lock<boost::shared_mutex> lock(rw_);
//   vector<float> tmp(buffer);
//   samples_available_ = false;
//   lock.unlock();
//   return(tmp);
  return(data_);
}

//-----------------------------------------------------------------------------

inline void USBamp::callGT_GetData()
{
  // if(first_run_)
  // {
    // for(unsigned int n = 0; n < ov_.size(); n++)
      // if( !GT_GetData(h_, driver_buffer_[n], driver_buffer_size_, &ov_[n]))
        // throw(std::runtime_error("USBamp::getSyncData -- Error getting data!"));

    // first_run_ = false;
    // return;
  // }

  if( !usb_amp_.getData(h_, driver_buffer_[current_overlapped_], driver_buffer_size_, &ov_[current_overlapped_]))
    throw(std::runtime_error("USBamp::getSyncData -- Error getting data!"));

  // GT_GetData(h_, driver_buffer_[current_overlapped_], driver_buffer_size_, &ov_[current_overlapped_]);

}

//-----------------------------------------------------------------------------

void USBamp::callGT_ResetTransfer()
{
  if( !usb_amp_.resetTransfer (h_))
    throw(std::runtime_error("USBamp::run -- Error resetting transfer!"));

  for(unsigned int n = 0; n < ov_.size(); n++)
    ResetEvent(data_Ev_[n]);
}

//-----------------------------------------------------------------------------

inline void USBamp::callGT_Start()
{
  if( !usb_amp_.start (h_))
    throw(std::runtime_error("USBamp::run -- Error starting g.USBamp!"));

}

//-----------------------------------------------------------------------------

void USBamp::fillSyncBuffer()
{
  if(!running_)
    return;

  samples_available_ = true;
  check4USBampError();

  timeout_ = WaitForSingleObject(data_Ev_[current_overlapped_],1000);
  if(timeout_ == WAIT_TIMEOUT)
  {
    cerr << "Timeout!" << endl;
  if( !usb_amp_.resetTransfer(h_))
      throw(std::runtime_error("USBamp::getSyncData -- Error resetting transfer!"));
    return;
  }

  GetOverlappedResult(h_, &ov_[current_overlapped_], &bytes_received_[current_overlapped_], false);
  sample_count_++;

  bytes_received_[current_overlapped_] -= HEADER_SIZE;

  if(bytes_received_[current_overlapped_] != expected_values_ * sizeof(float) )
  {
    cerr << "Received not enough data  at sample " << sample_count_ << "... " << bytes_received_[current_overlapped_] << ";  expected: " << expected_values_ * sizeof(float) << endl;

    for(unsigned int n = 0; n < ( (expected_values_ * sizeof(float) ) - bytes_received_[current_overlapped_] ) ; n++)
      driver_buffer_[current_overlapped_][ bytes_received_[current_overlapped_] + n ]  = 0;
  }

}

//-----------------------------------------------------------------------------

void USBamp::fillSampleBlock()
{
  unsigned int pos = 0;
  for(unsigned int k = 0; k < expected_values_/blocks_ ; k++)
    for(unsigned int j = 0; j < blocks_; j++)
    {
      pos = (k*blocks_) + j;
      samples_[ pos ] = *(reinterpret_cast<float*>(driver_buffer_[current_overlapped_] + HEADER_SIZE + (k +(j* expected_values_/blocks_) )*sizeof(float) ));

      if( isnan( samples_[pos] ) )
         cerr << "Received NaNs at block " << sample_count_ << "!" << endl;
    }

  data_.setSamples(samples_);

  (current_overlapped_ == ov_.size() -1)?(current_overlapped_ = 0):(current_overlapped_++);
}

//-----------------------------------------------------------------------------

void USBamp::check4USBampError()
{
  #ifdef DEBUG
    cout << "USBamp: check4USBampError" << endl;
  #endif

  error_code_ = 0;
  CHAR* error_ptr = error_msg_;

  if( !usb_amp_.getLastError(&error_code_, error_ptr))
    throw(std::runtime_error("USBamp::getSyncData -- Error getting last error message!"));

  if(error_code_)
    cerr << "Error code: " << error_code_ << ";  Error message: " << error_msg_ << endl;
}

//-----------------------------------------------------------------------------

void USBamp::getHandles()
{
  #ifdef DEBUG
    cout << "USBamp: getHandles" << endl;
  #endif

  if(serials_.find(m_.find(hardware_serial_)->second) != serials_.end())
    throw(std::invalid_argument("USBamp::getHandles -- g.USBamp with serial "\
          +m_.find(hardware_serial_)->second+" already in use!"));

  if(m_.find(hardware_serial_)->second == "")
    throw(std::invalid_argument("USBamp::getHandles -- No serial number for g.USBamp given!"));

  serials_.insert(m_.find(hardware_serial_)->second);
  serial_ = m_.find(hardware_serial_)->second;

  HANDLE h_tmp;
  h_tmp = usb_amp_.openDeviceEx( const_cast<LPSTR>(  m_.find(hardware_serial_)->second.c_str() ));
  if(h_tmp != 0)
  {
    h_ = (h_tmp);
    for(unsigned int n = 0; n < ov_.size(); n++)
    {
      memset(&ov_[n], 0, sizeof(OVERLAPPED) );
      data_Ev_[n] = CreateEvent(0, false, false, 0);

      ov_[n].hEvent = data_Ev_[n];
      ov_[n].Offset = 0;
      ov_[n].OffsetHigh = 0;
      // ResetEvent(data_Ev_[n]);   ... done in callGT_RestetTransfer
    }
  }
  else
  {
  printAvailableAmps();
    throw(std::invalid_argument("USBamp::getHandles -- g.USBamp with serial "\
          +m_.find(hardware_serial_)->second+" not connected!"));
  }
}

//-----------------------------------------------------------------------------

void USBamp::initFilterPtrs()
{
  #ifdef DEBUG
    cout << "USBamp: initFilterPtrs" << endl;
  #endif

  if( !usb_amp_.getNumberOfFilter(&nr_of_bp_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting number of filter!"));
  bp_filters_ = new FILT[nr_of_bp_filters_];
  if( !usb_amp_.getFilterSpec(bp_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting filter specifications!"));

  if( !usb_amp_.getNumberOfNotch(&nr_of_notch_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting number of notch!"));
  notch_filters_ = new FILT[nr_of_notch_filters_];
  if( !usb_amp_.getNotchSpec(notch_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting notch specifications!"));
}

//-----------------------------------------------------------------------------

void USBamp::run()
{
  #ifdef DEBUG
    cout << "USBamp: run" << endl;
  #endif

  if(!external_sync_)
  {
    for(uint32_t n = 0; n < slave_devices_.size(); n++)
      slave_devices_[n]->callGT_ResetTransfer();

    callGT_ResetTransfer();

    for(uint32_t n = 0; n < slave_devices_.size(); n++)
      slave_devices_[n]->callGT_Start();

    callGT_Start();

    for(unsigned int m = 0; m < ov_.size(); m++)
    {
      current_overlapped_ = m;

      for(uint32_t n = 0; n < slave_devices_.size(); n++)
        slave_devices_[n]->callGT_GetData();
      callGT_GetData();
    }
    current_overlapped_ = 0;
    first_run_ = true;


    for(uint32_t n = 0; n < slave_devices_.size(); n++)
    {
      slave_devices_[n]->running_ = 1;
      cout << " * g.USBamp " << slave_devices_[n]->serial_ << " sucessfully started by the master" << endl;
    }

    running_ = 1;
    cout << " * g.USBamp " << serial_ << " sucessfully started" << endl;

  }
  else
    cout << " * g.USBamp " << serial_ << " will be started by the master ..." << endl;
}

//-----------------------------------------------------------------------------

void USBamp::stop()
{
  #ifdef DEBUG
    cout << "USBamp: stop" << endl;
  #endif

  if(!running_)
    return;
  cout << " * stopping USBamp "  << m_.find(hardware_serial_)->second << " ... ";
  boost::unique_lock<boost::shared_mutex> lock(rw_);
  running_ = 0;

  if(!external_sync_)
    for(uint32_t n = 0; n < slave_devices_.size(); n++)
      slave_devices_[n]->stop();

  if( !usb_amp_.stop (h_))
    throw(std::runtime_error("USBamp::stop -- Error stopping the device!"));

  if( !usb_amp_.closeDevice (&h_))
    throw(std::runtime_error("USBamp::stop -- Error closing the device!"));

  h_ = 0;
  serials_.erase(m_.find(hardware_serial_)->second);

  for(unsigned int n = 0; n < ov_.size(); n++)
    CloseHandle(data_Ev_[n]);
  lock.unlock();


  serials_.erase(m_.find(hardware_serial_)->second);
}

//-----------------------------------------------------------------------------

void USBamp::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setDeviceSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs_,true));
    setSamplingRate(elem);

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);
  checkNrOfChannels();

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);

  setDefaultSettings();

  //---- optional ---

  elem = father->FirstChildElement(hw_filter_,false);
  if(elem != elem.end())
    setDeviceFilterSettings(elem);

  elem = father->FirstChildElement(hw_notch_,false);
  if(elem != elem.end())
    setDeviceNotchSettings(elem);

  elem = father->FirstChildElement(hw_opmode_,false);
  if(elem != elem.end())
    setOperationMode(elem);

  elem = father->FirstChildElement(hw_sc_,false);
  if(elem != elem.end())
    setShortCut(elem);

  elem = father->FirstChildElement(hw_trigger_line_,false);
  if(elem != elem.end())
    setTriggerLine(elem);

  elem = father->FirstChildElement(hw_usbampmaster_,false);
  if(elem != elem.end())
    setUSBampMasterOrSlave(elem);

  elem = father->FirstChildElement(hw_comgnd_,false);
  if(elem != elem.end())
    setCommonGround(elem);

  elem = father->FirstChildElement(hw_comref_,false);
  if(elem != elem.end())
    setCommonReference(elem);

  elem = father->FirstChildElement(hw_drl_,false);
  if(elem != elem.end())
    setDrivenRightLeg(elem);
}

//---------------------------------------------------------------------------------------

void USBamp::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
  checkNrOfChannels();

  adjustSettingsToChannelSelection();

  elem = father->FirstChildElement(hw_filter_,false);
  if(elem != elem.end())
    setChannelFilterSettings(elem);

  elem = father->FirstChildElement(hw_notch_,false);
  if(elem != elem.end())
    setChannelNotchSettings(elem);

  elem = father->FirstChildElement(hw_bipolar_,false);
  if(elem != elem.end())
    setBipolar(elem);

  elem = father->FirstChildElement(hw_drl_,false);
  if(elem != elem.end())
    setDrivenRightLeg(elem);
}

//---------------------------------------------------------------------------------------

void USBamp::setDefaultSettings()
{
  #ifdef DEBUG
    cout << "USBamp: setDefaultSettings" << endl;
  #endif

  int id = -1;
  for(unsigned int n = 0; n < channel_info_.size(); n++)
    filter_id_.push_back(id);

  for(unsigned int n = 0; n < channel_info_.size(); n++)
    notch_id_.push_back(id);

  //FIXME   --  if needed, implementation of other operation modes

  is_usbamp_master_ = 0;
  enable_sc_ = 1;

  ground_.GND1 = 1;
  ground_.GND2 = 1;
  ground_.GND3 = 1;
  ground_.GND4 = 1;

  reference_.ref1 = 1;
  reference_.ref2 = 1;
  reference_.ref3 = 1;
  reference_.ref4 = 1;

  bipolar_channels_.Channel1 = 0;
  bipolar_channels_.Channel2 = 0;
  bipolar_channels_.Channel3 = 0;
  bipolar_channels_.Channel4 = 0;
  bipolar_channels_.Channel5 = 0;
  bipolar_channels_.Channel6 = 0;
  bipolar_channels_.Channel7 = 0;
  bipolar_channels_.Channel8 = 0;
  bipolar_channels_.Channel9 = 0;
  bipolar_channels_.Channel10 = 0;
  bipolar_channels_.Channel11 = 0;
  bipolar_channels_.Channel12 = 0;
  bipolar_channels_.Channel13 = 0;
  bipolar_channels_.Channel14 = 0;
  bipolar_channels_.Channel15 = 0;
  bipolar_channels_.Channel16 = 0;

  drl_channels_.Channel1 = 0;
  drl_channels_.Channel2 = 0;
  drl_channels_.Channel3 = 0;
  drl_channels_.Channel4 = 0;
  drl_channels_.Channel5 = 0;
  drl_channels_.Channel6 = 0;
  drl_channels_.Channel7 = 0;
  drl_channels_.Channel8 = 0;
  drl_channels_.Channel9 = 0;
  drl_channels_.Channel10 = 0;
  drl_channels_.Channel11 = 0;
  drl_channels_.Channel12 = 0;
  drl_channels_.Channel13 = 0;
  drl_channels_.Channel14 = 0;
  drl_channels_.Channel15 = 0;
  drl_channels_.Channel16 = 0;
}

//---------------------------------------------------------------------------------------

void USBamp::setDeviceFilterSettings(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setDeviceFilterSettings" << endl;
  #endif

  checkFilterAttributes(elem);

  unsigned int type = 0;
  unsigned int order = 0;
  double f_low = 0;
  double f_high = 0;

  getFilterParams(elem, type, order, f_low, f_high);

  int id = search4FilterID(type, order, f_low, f_high);
  for(unsigned int n = 0; n < channel_info_.size(); n++)
    filter_id_[n] = id;

  cout << " * g.USBamp -- filter set to:" << endl;
  cout << "    ...  order: " << order << ", f_low: " << f_low << ", f_high: " << f_high << endl;
  cout << endl;

}

//---------------------------------------------------------------------------------------

void USBamp::setChannelFilterSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setChannelFilterSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;
  elem = father->FirstChildElement(hw_chset_ch_,false);

  cout << " * g.USBamp -- channels specific filters set to:" << endl;

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_filter_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));


      }
      checkFilterAttributes(elem);

      uint16_t ch = 0;
      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_filter_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_filter_ + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(std::invalid_argument(ex_str));
      }

      unsigned int type = 0;
      unsigned int order = 0;
      double f_low = 0;
      double f_high = 0;

      getFilterParams(elem, type, order, f_low, f_high);

      uint16_t ch_pos = 0;
      map<uint16_t, pair<string, uint32_t> >::iterator it;
      for(it = channel_info_.begin(); it !=channel_info_.find(ch); it++)
        ch_pos++;
      filter_id_.at(ch_pos) = search4FilterID(type, order, f_low, f_high);

      cout << "  ... channel: " << ch << ", order: " << order << ", f_low: " << f_low << ", f_high: " << f_high << endl;
    }
    else
      throw(std::invalid_argument("USBamp::setChannelFilterSettings -- Tag not equal to \""+hw_chset_ch_+"\"!"));
}

//---------------------------------------------------------------------------------------

void USBamp::getFilterParams(ticpp::Iterator<ticpp::Element>const &elem,\
  unsigned int &type, unsigned int &order, double &f_low, double &f_high)
{
  #ifdef DEBUG
    cout << "USBamp: getFilterParams" << endl;
  #endif

  type = getUSBampFilterType( elem.Get()->GetAttribute(hw_filter_type_) );
  try
  {
    order = lexical_cast<unsigned int>( elem.Get()->GetAttribute(hw_filter_order_) );

    f_low  = lexical_cast<double>( elem.Get()->GetAttribute(hw_filter_low_) );
    f_high = lexical_cast<double>( elem.Get()->GetAttribute(hw_filter_high_ ));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_filter_+"> given, but order, lower or upper cutoff frequency is not a number!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void USBamp::checkFilterAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: checkFilterAttributes" << endl;
  #endif

  if(!elem.Get()->HasAttribute(hw_filter_type_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_filter_+"> given, filter type ("+hw_filter_type_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_filter_order_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_filter_+"> given, filter order ("+hw_filter_order_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_filter_low_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_filter_+"> given, lower cutoff frequency ("+hw_filter_low_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
  if(!elem.Get()->HasAttribute(hw_filter_high_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_filter_+"> given, upper cutoff frequency ("+hw_filter_high_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

int USBamp::search4FilterID(unsigned int type, unsigned int order, double f_low, double f_high)
{
  #ifdef DEBUG
    cout << "USBamp: search4FilterID" << endl;
  #endif

  int id = -1;

  for(int n = 0; n < nr_of_bp_filters_; n++)
  {
    if(( roundD(bp_filters_[n].fs)  == fs_)      &&  ( roundD(bp_filters_[n].type) == type) && \
       ( roundD(bp_filters_[n].order) == order) &&  ( roundD(bp_filters_[n].fu)  == f_low) && \
       ( roundD(bp_filters_[n].fo)    == f_high) )
      id = n;
  }

  if(id < 0)
  {
    printPossibleBandPassFilters();
    string ex_str = "USBamp::search4FilterID -- Filter settings not possible -- ";
    ex_str = ex_str + "Fs: "   + lexical_cast<string>(boost::format("%d") % fs_)  +  ", ";
    ex_str = ex_str + "Type: "   + getUSBampFilterName(type)  +  ", ";
    ex_str = ex_str + "Order: "  + lexical_cast<string>(order) +  ", ";
    ex_str = ex_str + "f_low: "  + lexical_cast<string>(boost::format("%d") % f_low) +  ", ";
    ex_str = ex_str + "f_high: " + lexical_cast<string>(boost::format("%d") % f_high);
    throw(std::invalid_argument(ex_str));
  }

  return(id);
}

//---------------------------------------------------------------------------------------

void USBamp::printPossibleBandPassFilters()
{
  if(!bp_filters_)
    return;

  cout << left;
  cout << endl;
  cout.width(9);
  cout << "     Fs: "<< " | ";

  cout.width(10);
  cout << "Type: "<< "|  ";
  cout << "Order: "<< "|  ";

  cout.width(6);
  cout << " f_low: "<< "| ";


  cout << "f_high: ";

  cout << endl;
  cout << endl;


  for(int n = 0; n < nr_of_bp_filters_; n++)
    if(fs_ == bp_filters_[n].fs)
    {
    cout << right;
    cout.width(9);
    cout << bp_filters_[n].fs << " | ";

    cout.width(9);
    cout <<  getUSBampFilterName(bp_filters_[n].type)  << " | ";
    cout.width(7);
    cout <<  bp_filters_[n].order << " | ";

    cout.width(8);
    cout << roundD(bp_filters_[n].fu) << " | ";

    cout << left;
    cout.width(8);
    cout << roundD(bp_filters_[n].fo);
    cout << endl;
  }

  cout << endl;
  cout << endl;
}

//---------------------------------------------------------------------------------------

void USBamp::printPossibleNotchFilters()
{
  if(!notch_filters_)
    return;

  cout << left;
  cout << endl;
  cout.width(9);
  cout << "     Fs: "<< " | ";

  cout.width(10);
  cout << "Type: "<< "|  ";
  cout << "Order: "<< "|  ";

  cout.width(6);
  cout << " f_low: "<< "| ";


  cout << "f_high: ";

  cout << endl;
  cout << endl;

  for(int n = 0; n < nr_of_notch_filters_; n++)
    if(fs_ == notch_filters_[n].fs)
    {
      cout << right;
      cout.width(9);
      cout << notch_filters_[n].fs << " | ";

      cout.width(8);
      cout <<  getUSBampFilterName(notch_filters_[n].type)  << " | ";
      cout.width(7);
      cout <<  notch_filters_[n].order << " | ";

      cout.width(8);
      cout << roundD(notch_filters_[n].fu) << " | ";

      cout << left;
      cout.width(8);
      cout << roundD(notch_filters_[n].fo);
      cout << endl;
    }

}

//---------------------------------------------------------------------------------------

void USBamp::printAvailableAmps()
{
  HANDLE dev = 0;
  string serial_str("\0", 16);    // at least 16 ... 256 g.tec ref. implem. !!
  cout << "   Available serials: " << endl;

  for(unsigned int n = 0; n < USBAMP_MAX_NR_OF_AMPS; n++)
  {
    dev = usb_amp_.openDevice(n);
    if(dev != 0)
    {
      usb_amp_.getSerial(dev, const_cast<LPSTR>(serial_str.c_str()) , serial_str.size() );
      cout << "    * " << serial_str.c_str() << endl;
      usb_amp_.closeDevice(&dev);
    }
  }
  cout << std::flush;
}

//---------------------------------------------------------------------------------------

void USBamp::setDeviceNotchSettings(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setDeviceNotchSettings" << endl;
  #endif

  float f_center = 0;

  checkNotchAttributes(elem);
  getNotchParams(elem, f_center);

  int id = search4NotchID(f_center);
  for(unsigned int n = 0; n < channel_info_.size(); n++)
    notch_id_[n] = id;
}

//---------------------------------------------------------------------------------------

void USBamp::setChannelNotchSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setChannelNotchSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;
  elem = father->FirstChildElement(hw_chset_ch_,false);
  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_notch_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      checkNotchAttributes(elem);

      uint16_t ch = 0;
      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_notch_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_notch_ + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(std::invalid_argument(ex_str));
      }

      float f_center = 0;

      getNotchParams(elem, f_center);

      uint16_t ch_pos = 0;
      map<uint16_t, pair<string, uint32_t> >::iterator it;
      for(it = channel_info_.begin(); it !=channel_info_.find(ch); it++)
        ch_pos++;
      notch_id_.at(ch_pos) = search4NotchID(f_center);
    }
    else
      throw(std::invalid_argument("USBamp::setChannelNotchSettings -- Tag not equal to \""+hw_chset_ch_+"\"!"));
}

//---------------------------------------------------------------------------------------

void USBamp::checkNotchAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: checkNotchAttributes" << endl;
  #endif

  if(!elem.Get()->HasAttribute(hw_notch_center_))
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_notch_+"> given, notch center frequency ("+hw_notch_center_+") not given!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void USBamp::getNotchParams(ticpp::Iterator<ticpp::Element>const &elem, float &f_center)
{
  #ifdef DEBUG
    cout << "USBamp: getNotchParams" << endl;
  #endif

  try
  {
    f_center = lexical_cast<float>( elem.Get()->GetAttribute(hw_notch_center_));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str(type_ + " -- ");
    ex_str += "Tag <"+hw_notch_+"> given, but center frequency is not a number!";
    throw(std::invalid_argument(ex_str));
  }
}

//---------------------------------------------------------------------------------------

int USBamp::search4NotchID(float f_center)
{
  #ifdef DEBUG
    cout << "USBamp: search4NotchID" << endl;
  #endif

  int id = -1;

  for(int n = 0; n < nr_of_notch_filters_; n++)
  {
    if( ( roundD(notch_filters_[n].fs)  == fs_) && \
      ( roundD(notch_filters_[n].fu) == f_center - USBAMP_NOTCH_HALF_WIDTH ) && \
      ( roundD(notch_filters_[n].fo) == f_center + USBAMP_NOTCH_HALF_WIDTH ) )
      id = n;
  }

  if(id < 0)
  {
    printPossibleNotchFilters();
    string ex_str = "USBamp::search4NotchID -- Notch settings not possible -- ";
    ex_str = ex_str + "f_center: " + lexical_cast<string>(boost::format("%d") % f_center);
    throw(std::invalid_argument(ex_str));
  }

  return(id);
}

//---------------------------------------------------------------------------------------

void USBamp::setOperationMode(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setOperationMode" << endl;
  #endif

  //FIXME   --  if needed, implementation of other operation modes
  if(elem->GetText(true) != "normal")
    throw(std::invalid_argument("USBamp::setOperationMode -- So far only normal operation mode supported!"));

}

//---------------------------------------------------------------------------------------

void USBamp::setShortCut(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setShortCut" << endl;
  #endif

  enable_sc_ = equalsOnOrOff(elem->GetText(true));
}

//---------------------------------------------------------------------------------------

void USBamp::setTriggerLine(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setTriggerLine" << endl;
  #endif

  trigger_line_ = equalsOnOrOff(elem->GetText(true));

  Constants cst;

  if(elem->HasAttribute(hw_ch_type_))
    trigger_line_sig_type_ = cst.getSignalFlag( elem->GetAttribute(hw_ch_type_) );
}

//---------------------------------------------------------------------------------------

void USBamp::setUSBampMasterOrSlave(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setUSBampMasterOrSlave" << endl;
  #endif

  external_sync_ = !(equalsYesOrNo(elem->GetText(true)));

  if(is_usbamp_master_ && !external_sync_)
    throw(std::runtime_error("USBamp::setUSBampMasterOrSlave -- Only one USBamp master allowed!"));

  is_usbamp_master_ = true;
}

//---------------------------------------------------------------------------------------

void USBamp::setCommonGround(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setCommonGround" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;
  vector<uint8_t> v(USBAMP_NR_OF_CHANNEL_GROUPS,1);
  elem = father->FirstChildElement(false);

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_gnd_)
    {
      if(!elem.Get()->HasAttribute(hw_gnd_block_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_comgnd_+"> given, but block identifier ("+hw_gnd_block_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      if(!elem.Get()->HasAttribute(hw_gnd_value_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_comgnd_+"> given, but block value ("+hw_gnd_value_+") not given!";
        throw(std::invalid_argument(ex_str));
      }

      int gnd = getUSBampBlockNr( elem.Get()->GetAttribute(hw_gnd_block_) );
      bool b = equalsYesOrNo( elem.Get()->GetAttribute(hw_gnd_value_) );

      v.at(gnd) = b;
    }
    else
      throw(std::invalid_argument("USBamp::setCommonGround -- Tag not equal to \""+hw_gnd_+"\"!"));

  //FIXME   --  hardcoded values
  ground_.GND1 = v.at(0);
  ground_.GND2 = v.at(1);
  ground_.GND3 = v.at(2);
  ground_.GND4 = v.at(3);
}

//---------------------------------------------------------------------------------------

void USBamp::setCommonReference(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setCommonReference" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;
  vector<uint8_t> v(USBAMP_NR_OF_CHANNEL_GROUPS,1);
  elem = father->FirstChildElement(false);

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_cr_)
    {
      if(!elem.Get()->HasAttribute(hw_cr_block_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_comref_+"> given, but block identifier ("+hw_cr_block_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      if(!elem.Get()->HasAttribute(hw_cr_value_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_comref_+"> given, but block value ("+hw_cr_value_+") not given!";
        throw(std::invalid_argument(ex_str));
      }

      int ref = getUSBampBlockNr( elem.Get()->GetAttribute(hw_cr_block_) );
      bool b = equalsYesOrNo( elem.Get()->GetAttribute(hw_cr_value_) );

      v.at(ref) = b;
    }
    else
      throw(std::invalid_argument("USBamp::setCommonReference -- Tag not equal to \""+hw_cr_+"\"!"));

  //FIXME   --  hardcoded values
  reference_.ref1 = v.at(0);
  reference_.ref2 = v.at(1);
  reference_.ref3 = v.at(2);
  reference_.ref4 = v.at(3);

}

//---------------------------------------------------------------------------------------

void USBamp::setBipolar(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setBipolar" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;
  vector<uint8_t> v(USBAMP_MAX_NR_OF_CHANNELS,0);
  elem = father->FirstChildElement(hw_chset_ch_,false);

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_bipolar_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      if(!elem.Get()->HasAttribute(hw_bipolar_with_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_bipolar_+"> given, but bipolar combination ("+hw_bipolar_with_+") not given!";
        throw(std::invalid_argument(ex_str));
      }

      uint16_t with = 0;
      uint16_t ch   = 0;

      try{
        with = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_bipolar_with_) );
        ch   = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_bipolar_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( (channel_info_.find(ch) ==  channel_info_.end()) || (channel_info_.find(with) ==  channel_info_.end()) )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_bipolar_ + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(std::invalid_argument(ex_str));
      }

      // FIXME  --  check channels, if in range ( USBAMP_MAX_NR... )

      v.at(ch) = boost::numeric_cast<uint8_t>(with);
    }
    else
      throw(std::invalid_argument("USBamp::setBipolar -- Tag not equal to \""+hw_chset_ch_+"\"!"));

  //FIXME   --  hardcoded values
  bipolar_channels_.Channel1 = v.at(0);
  bipolar_channels_.Channel2 = v.at(1);
  bipolar_channels_.Channel3 = v.at(2);
  bipolar_channels_.Channel4 = v.at(3);
  bipolar_channels_.Channel5 = v.at(4);
  bipolar_channels_.Channel6 = v.at(5);
  bipolar_channels_.Channel7 = v.at(6);
  bipolar_channels_.Channel8 = v.at(7);
  bipolar_channels_.Channel9 = v.at(8);
  bipolar_channels_.Channel10 = v.at(9);
  bipolar_channels_.Channel11 = v.at(10);
  bipolar_channels_.Channel12 = v.at(11);
  bipolar_channels_.Channel13 = v.at(12);
  bipolar_channels_.Channel14 = v.at(13);
  bipolar_channels_.Channel15 = v.at(14);
  bipolar_channels_.Channel16 = v.at(15);
}

//---------------------------------------------------------------------------------------

void USBamp::setDrivenRightLeg(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setDrivenRightLeg" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;
  elem = father->FirstChildElement(hw_chset_ch_,false);
  if(elem != elem.end())
  {
    setIndividualDrivenRightLeg(elem);
    return;
  }

  if(equalsYesOrNo(father->GetText(true)))
  {
    drl_channels_.Channel1 = 1;
    drl_channels_.Channel2 = 1;
    drl_channels_.Channel3 = 1;
    drl_channels_.Channel4 = 1;
    drl_channels_.Channel5 = 1;
    drl_channels_.Channel6 = 1;
    drl_channels_.Channel7 = 1;
    drl_channels_.Channel8 = 1;
    drl_channels_.Channel9 = 1;
    drl_channels_.Channel10 = 1;
    drl_channels_.Channel11 = 1;
    drl_channels_.Channel12 = 1;
    drl_channels_.Channel13 = 1;
    drl_channels_.Channel14 = 1;
    drl_channels_.Channel15 = 1;
    drl_channels_.Channel16 = 1;
  }
}

//---------------------------------------------------------------------------------------

void USBamp::setIndividualDrivenRightLeg(ticpp::Iterator<ticpp::Element> &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setIndividualDrivenRightLeg" << endl;
  #endif

  vector<uint8_t> v(USBAMP_MAX_NR_OF_CHANNELS,0);

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == hw_chset_ch_)
    {
      if(!elem.Get()->HasAttribute(hw_ch_nr_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_drl_+"> given, but channel number ("+hw_ch_nr_+") not given!";
        throw(std::invalid_argument(ex_str));
      }
      if(!elem.Get()->HasAttribute(hw_drl_value_))
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+hw_drl_+"> given, but DRL value ("+hw_drl_value_+") not given!";
        throw(std::invalid_argument(ex_str));
      }

      bool b = equalsOnOrOff( elem.Get()->GetAttribute(hw_drl_value_) );
      uint16_t ch   = 0;

      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(hw_ch_nr_) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_drl_ + "> : Channel is not a number!";
        throw(std::invalid_argument(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str(type_ + " -- ");
        ex_str += "Tag <"+ hw_drl_ + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(std::invalid_argument(ex_str));
      }

      v.at(ch-1) = b;
    }
    else
      throw(std::invalid_argument("USBamp::setIndividualDrivenRightLeg -- Tag not equal to \""+hw_chset_ch_+"\"!"));

    //FIXME   --  hardcoded values
    drl_channels_.Channel1 = v.at(0);
    drl_channels_.Channel2 = v.at(1);
    drl_channels_.Channel3 = v.at(2);
    drl_channels_.Channel4 = v.at(3);
    drl_channels_.Channel5 = v.at(4);
    drl_channels_.Channel6 = v.at(5);
    drl_channels_.Channel7 = v.at(6);
    drl_channels_.Channel8 = v.at(7);
    drl_channels_.Channel9 = v.at(8);
    drl_channels_.Channel10 = v.at(9);
    drl_channels_.Channel11 = v.at(10);
    drl_channels_.Channel12 = v.at(11);
    drl_channels_.Channel13 = v.at(12);
    drl_channels_.Channel14 = v.at(13);
    drl_channels_.Channel15 = v.at(14);
    drl_channels_.Channel16 = v.at(15);
}

//---------------------------------------------------------------------------------------

void USBamp::checkNrOfChannels()
{
  #ifdef DEBUG
    cout << "USBamp: checkNrOfChannels" << endl;
  #endif

  if(nr_ch_ > USBAMP_MAX_NR_OF_CHANNELS )
    throw(std::invalid_argument("Too many channels defined -- maximum nr of channels: "\
                                +lexical_cast<string>(USBAMP_MAX_NR_OF_CHANNELS)) );

  map<uint16_t, pair<string, uint32_t> >::iterator it(channel_info_.begin());

  for(  ; it != channel_info_.end(); it++ )
    if(it->first > USBAMP_MAX_NR_OF_CHANNELS )
      throw(std::invalid_argument("Channel number too high -- maximum nr of channels: "\
      +lexical_cast<string>(USBAMP_MAX_NR_OF_CHANNELS)) );
}

//---------------------------------------------------------------------------------------

void USBamp::adjustSettingsToChannelSelection()
{
  unsigned int new_size = channel_info_.size();

  boost::uint16_t notch  = notch_id_.at(0);
  boost::uint16_t filter = filter_id_.at(0);
  notch_id_.resize(new_size);
  filter_id_.resize(new_size);

  for(unsigned int n = 0; n < new_size; n++)
  {
    filter_id_[n] = filter;
    notch_id_[n]  = notch;
  }
}

//---------------------------------------------------------------------------------------

void USBamp::setUSBampChannels()
{
  #ifdef DEBUG
    cout << "USBamp: setUSBampChannels" << endl;
  #endif

  vector<uint16_t> channels;
  map<uint16_t, pair<string, uint32_t> >::iterator it(channel_info_.begin());
  map<uint16_t, pair<string, uint32_t> >::iterator stop(channel_info_.end());

  if(trigger_line_)
    stop--;

  for(  ; it != stop; it++)
      channels.push_back(it->first);

  UCHAR* uc_channels = new UCHAR[channels.size()];
  for(unsigned int n = 0; n < channels.size(); n++)
    uc_channels[n] = boost::numeric_cast<UCHAR>( channels[n] );

  if( !usb_amp_.setChannels (h_, uc_channels, channels.size()))
    throw(std::runtime_error("USBamp::setUSBampChannels -- Error setting channels!"));

  delete[] uc_channels;
}

//---------------------------------------------------------------------------------------

void USBamp::setUSBampFilter()
{
  #ifdef DEBUG
    cout << "USBamp: setUSBampFilter" << endl;
  #endif

  bool check = 1;
  unsigned int count = 0;
  map<uint16_t, pair<string, uint32_t> >::iterator it;
  map<uint16_t, pair<string, uint32_t> >::iterator stop(channel_info_.end());

  if(trigger_line_)
    stop--;

  for( it=channel_info_.begin() ; it != stop; it++)
  {
    if(check)
    {
      if(filter_id_[count] > 0)
      check = usb_amp_.setBandPass (h_, boost::numeric_cast<UCHAR>((*it).first)   , filter_id_[count]);
      else
        cout << "Filter for channel " << (*it).first << " NOT set!" << endl;

      count++;
    }
  }

  if(!check)
    throw(std::runtime_error("USBamp::setUSBampFilter -- Error setting filter!"));

}

//---------------------------------------------------------------------------------------

void USBamp::setUSBampNotch()
{
  #ifdef DEBUG
    cout << "USBamp: setUSBampNotch" << endl;
  #endif

  bool check = 1;
  unsigned int count = 0;
  map<uint16_t, pair<string, uint32_t> >::iterator it;
  map<uint16_t, pair<string, uint32_t> >::iterator stop(channel_info_.end());

  if(trigger_line_)
    stop--;

  for( it=channel_info_.begin() ; it != stop; it++)
  {
    if(check)
    {
      if(notch_id_[count] > 0)
      check = usb_amp_.setNotch (h_, boost::numeric_cast<UCHAR>((*it).first), notch_id_[count]);
      else
        cout << "Notch for channel " << (*it).first << " NOT set!" << endl;
      count++;
    }
  }

  if(!check)
    throw(std::runtime_error("USBamp::setUSBampNotch -- Error setting notch!"));

}

//---------------------------------------------------------------------------------------

void USBamp::checkTriggerLineChannel()
{
  #ifdef DEBUG
    cout << "USBamp: checkTriggerLineChannel" << endl;
  #endif

  if(trigger_line_)
  {
    boost::uint32_t last_ch = (--channel_info_.end())->first;

    channel_info_[last_ch + 1] = make_pair(hw_trigger_line_, trigger_line_sig_type_);
    nr_ch_ += 1;
    homogenous_signal_type_ = 0;
    setChannelTypes();
  }
}

//---------------------------------------------------------------------------------------

void USBamp::initUSBamp()
{
  #ifdef DEBUG
    cout << "USBamp: initUSBamp" << endl;
  #endif

  if( !usb_amp_.setMode (h_, M_NORMAL) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting mode!"));

  if( !usb_amp_.setSampleRate(h_, boost::numeric_cast<WORD>(fs_) ))
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting sampling rate!"));

  if( !usb_amp_.setBufferSize(h_, blocks_))
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting buffer size!"));

  setUSBampChannels();

  if( !usb_amp_.setSlave (h_, external_sync_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting synchronization mode (master or slave)!"));

  if( !usb_amp_.enableTriggerLine(h_, trigger_line_))
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting trigger line!"));

  if( !usb_amp_.enableSC (h_, enable_sc_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting shortcut!"));

  if( !usb_amp_.setBipolar (h_, bipolar_channels_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting bipolar channels!"));

  if( !usb_amp_.setReference (h_, reference_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting reference!"));

  if( !usb_amp_.setGround (h_, ground_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting ground!"));

  if( !usb_amp_.setDRLChannel(h_, drl_channels_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting driven right leg channels!"));

  setUSBampFilter();
  setUSBampNotch();
}

//-----------------------------------------------------------------------------

int USBamp::getUSBampFilterType(const string& s)
{
  map<string, unsigned int>::iterator it;
  it = usbamp_filter_types_.find(to_lower_copy(s));
  if(it == usbamp_filter_types_.end())
  {
    string e = "USBamp filter type \"" + s + "\" not found -- please also check spelling!";
    throw(std::invalid_argument(e));
  }
  return(it->second);
}

//-----------------------------------------------------------------------------

string USBamp::getUSBampFilterName(double n)
{
  map<string, unsigned int>::iterator it;
  int nr = boost::numeric_cast<int>(n);
  it = usbamp_filter_types_.begin();

  while(it != usbamp_filter_types_.end())
  {
    if(it->second == nr)
      return(it->first);
    it++;
  }
  string e = "USBamp filter number not found!";
  throw(std::invalid_argument(e));
}

//-----------------------------------------------------------------------------

string USBamp::getUSBampOpMode(const string& s)
{
  map<string, string>::iterator it;
  it = usbamp_opModes_.find(to_lower_copy(s));
  if(it == usbamp_opModes_.end())
  {
    string e = "USBamp operation mode \"" + s + "\" not found -- please also check spelling!";
    throw(std::invalid_argument(e));
  }
  return(it->second);
}

//-----------------------------------------------------------------------------

int USBamp::getUSBampBlockNr(const string& s)
{
  map<string, unsigned int>::iterator it;
  it = usbamp_block_names_.find(to_lower_copy(s));
  if(it == usbamp_block_names_.end())
  {
    string e = "USBamp channel block \"" + s + "\" wrong -- must be \"a, b, c or d\"!";
    throw(std::invalid_argument(e));
  }
  return(it->second);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss



// DEBUG code from constructor

//#ifdef DEBUG
//  map<uint16_t, pair<string, uint32_t> >::iterator it(channel_info_.begin());
//
//  cout << "Filter/Notch Settings:  (filter id size)" << filter_id_.size() <<endl;
//  for(unsigned int n = 0; n < filter_id_.size(); n++)
//  {
//    cout << "ch: " << it->first << " -- type: "<<  bp_filters_[ filter_id_[n] ].type;
//    cout << ", order: "  <<  bp_filters_[ filter_id_[n] ].order;
//    cout << ", f_low: "  <<  bp_filters_[ filter_id_[n] ].fu;
//    cout << ", f_high: " <<  bp_filters_[ filter_id_[n] ].fo;
//    cout << "; -- notch: type: " <<  notch_filters_[ notch_id_[n] ].type;
//    cout << ", order: "  <<  notch_filters_[ notch_id_[n] ].order;
//    cout << ", f_low: "  <<  notch_filters_[ notch_id_[n] ].fu;
//    cout << ", f_high: " <<  notch_filters_[ notch_id_[n] ].fo << endl;
//    it++;
//  }
//
//  GND ground;
//  usb_amp_.getGround(h_, &ground);
//  //    GT_GetGround(h_, &ground);
//
//  cout << "Ground settings: " << endl;
//  cout << ground.GND1 << ", " << ground.GND2 << ", ";
//  cout << ground.GND3 << ", " << ground.GND4 << endl;
//
//  REF reference;
//  usb_amp_.getReference(h_, &reference);
//  //    GT_GetReference(h_, &reference);
//
//  cout << "Reference settings: " << endl;
//  cout << reference.ref1 << ", " << reference.ref2 << ", ";
//  cout << reference.ref3 << ", " << reference.ref4 << endl;
//
//  cout << "Bipolar channel settings: " << endl;
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel1) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel2) << ", ";
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel3) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel4) << ", ";
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel5) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel6) << ", ";
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel7) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel8) << endl;
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel9) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel10) << ", ";
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel11) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel12) << ", ";
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel13) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel14) << ", ";
//  cout << static_cast<uint32_t>(bipolar_channels_.Channel15) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel16) << endl;
//
//  cout << "DRL channel settings: " << endl;
//  cout << static_cast<uint32_t>(drl_channels_.Channel1) << ", " << static_cast<uint32_t>(drl_channels_.Channel2) << ", ";
//  cout << static_cast<uint32_t>(drl_channels_.Channel3) << ", " << static_cast<uint32_t>(drl_channels_.Channel4) << ", ";
//  cout << static_cast<uint32_t>(drl_channels_.Channel5) << ", " << static_cast<uint32_t>(drl_channels_.Channel6) << ", ";
//  cout << static_cast<uint32_t>(drl_channels_.Channel7) << ", " << static_cast<uint32_t>(drl_channels_.Channel8) << endl;
//  cout << static_cast<uint32_t>(drl_channels_.Channel9) << ", " << static_cast<uint32_t>(drl_channels_.Channel10) << ", ";
//  cout << static_cast<uint32_t>(drl_channels_.Channel11) << ", " << static_cast<uint32_t>(drl_channels_.Channel12) << ", ";
//  cout << static_cast<uint32_t>(drl_channels_.Channel13) << ", " << static_cast<uint32_t>(drl_channels_.Channel14) << ", ";
//  cout << static_cast<uint32_t>(drl_channels_.Channel15) << ", " << static_cast<uint32_t>(drl_channels_.Channel16) << endl;
//#endif


