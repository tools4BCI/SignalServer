
#ifdef WIN32

#include "hardware/usbamp.h"

#include <boost/bind.hpp>


namespace tobiss
{
set<string> USBamp::serials_;

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

using boost::lexical_cast;
using boost::bad_lexical_cast;

//-----------------------------------------------------------------------------

USBamp::USBamp(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
  : HWThread(parser), enable_sc_(0), external_sync_(0), trigger_line_(0), sample_count_(0), error_count_(0) ,error_code_(0)
{
  #ifdef DEBUG
    cout << "USBamp: Constructor" << endl;
  #endif

  #pragma comment(lib,"gUSBamp.lib")

  checkMandatoryHardwareTags(hw);
  getHandles();
  initFilterPtrs();
  setHardware(hw);

  driver_buffer_size_ = nr_ch_ * blocks_ * sizeof(float) + HEADER_SIZE;
  driver_buffer_ = new BYTE[driver_buffer_size_];

  error_msg_ = new CHAR[USBMAP_ERROR_MSG_SIZE];

  data_.init(blocks_, nr_ch_, channel_types_);
  samples_.resize(nr_ch_ * blocks_, 0);

  initUSBamp();

  cout << " * g.USBamp sucessfully initialized" << endl;
  cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_ << ", blocksize: " << blocks_ << endl;
  if(!homogenous_signal_type_)
  {
    cout << "   ... NOTICE: Device is acquiring different signal types  ";
    cout << "--  ensure that reference and ground settings are correctly set!" << endl;
  }

  #ifdef DEBUG
    map<uint16_t, pair<string, uint32_t> >::iterator it(channel_info_.begin());

    cout << "Filter/Notch Settings:  (filter id size)" << filter_id_.size() <<endl;
    for(unsigned int n = 0; n < filter_id_.size(); n++)
    {
      cout << "ch: " << it->first << " -- type: "<<  bp_filters_[ filter_id_[n] ].type;
      cout << ", order: "  <<  bp_filters_[ filter_id_[n] ].order;
      cout << ", f_low: "  <<  bp_filters_[ filter_id_[n] ].fu;
      cout << ", f_high: " <<  bp_filters_[ filter_id_[n] ].fo;
      cout << "; -- notch: type: " <<  notch_filters_[ notch_id_[n] ].type;
      cout << ", order: "  <<  notch_filters_[ notch_id_[n] ].order;
      cout << ", f_low: "  <<  notch_filters_[ notch_id_[n] ].fu;
      cout << ", f_high: " <<  notch_filters_[ notch_id_[n] ].fo << endl;
      it++;
    }

    GND ground;
    GT_GetGround(h_, &ground);

    cout << "Ground settings: " << endl;
    cout << ground.GND1 << ", " << ground.GND2 << ", ";
    cout << ground.GND3 << ", " << ground.GND4 << endl;

    REF reference;
    GT_GetReference(h_, &reference);

    cout << "Reference settings: " << endl;
    cout << reference.ref1 << ", " << reference.ref2 << ", ";
    cout << reference.ref3 << ", " << reference.ref4 << endl;

    cout << "Bipolar channel settings: " << endl;
    cout << static_cast<uint32_t>(bipolar_channels_.Channel1) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel2) << ", ";
    cout << static_cast<uint32_t>(bipolar_channels_.Channel3) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel4) << ", ";
    cout << static_cast<uint32_t>(bipolar_channels_.Channel5) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel6) << ", ";
    cout << static_cast<uint32_t>(bipolar_channels_.Channel7) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel8) << endl;
    cout << static_cast<uint32_t>(bipolar_channels_.Channel9) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel10) << ", ";
    cout << static_cast<uint32_t>(bipolar_channels_.Channel11) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel12) << ", ";
    cout << static_cast<uint32_t>(bipolar_channels_.Channel13) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel14) << ", ";
    cout << static_cast<uint32_t>(bipolar_channels_.Channel15) << ", " << static_cast<uint32_t>(bipolar_channels_.Channel16) << endl;

    cout << "DRL channel settings: " << endl;
    cout << static_cast<uint32_t>(drl_channels_.Channel1) << ", " << static_cast<uint32_t>(drl_channels_.Channel2) << ", ";
    cout << static_cast<uint32_t>(drl_channels_.Channel3) << ", " << static_cast<uint32_t>(drl_channels_.Channel4) << ", ";
    cout << static_cast<uint32_t>(drl_channels_.Channel5) << ", " << static_cast<uint32_t>(drl_channels_.Channel6) << ", ";
    cout << static_cast<uint32_t>(drl_channels_.Channel7) << ", " << static_cast<uint32_t>(drl_channels_.Channel8) << endl;
    cout << static_cast<uint32_t>(drl_channels_.Channel9) << ", " << static_cast<uint32_t>(drl_channels_.Channel10) << ", ";
    cout << static_cast<uint32_t>(drl_channels_.Channel11) << ", " << static_cast<uint32_t>(drl_channels_.Channel12) << ", ";
    cout << static_cast<uint32_t>(drl_channels_.Channel13) << ", " << static_cast<uint32_t>(drl_channels_.Channel14) << ", ";
    cout << static_cast<uint32_t>(drl_channels_.Channel15) << ", " << static_cast<uint32_t>(drl_channels_.Channel16) << endl;
  #endif
}

//-----------------------------------------------------------------------------

USBamp::~USBamp()
{
  #ifdef DEBUG
    cout << "USBamp: Destructor" << endl;
  #endif

  delete[] bp_filters_;
  delete[] notch_filters_;
  delete[] driver_buffer_;
  delete[] error_msg_;
}

//-----------------------------------------------------------------------------

void USBamp::setHardware(ticpp::Iterator<ticpp::Element>const& hw)
{
  #ifdef DEBUG
    cout << "USBamp: setHardware" << endl;
  #endif

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
  bytes_received_ = 0;

  if( !GT_GetData(h_, driver_buffer_, driver_buffer_size_, &ov_))
    throw(std::runtime_error("USBamp::getSyncData -- Error getting data!"));
  samples_available_ = true;

  check4USBampError();

  timeout_ = WaitForSingleObject(data_Ev_,1000);
  if(timeout_ == WAIT_TIMEOUT)
  {
    cout << "Timeout!" << endl;
    if( !GT_ResetTransfer(h_))
      throw(std::runtime_error("USBamp::getSyncData -- Error resetting transfer!"));
  }

  GetOverlappedResult(h_, &ov_, &bytes_received_, false);
  sample_count_++;

  bytes_received_ -= HEADER_SIZE;
  if(bytes_received_ != sizeof(float)* nr_ch_ * blocks_)
  {
    cout << "WARNING  -- Buffer-Size only: " << bytes_received_ << ";  -> should be: " << sizeof(float)* nr_ch_ * blocks_;
    cout << "  (at Sample: " << sample_count_ << ")"<< endl;
    cout << "  ** WARNING: Inserted zeros instead of samples for testing!! **"<< endl;
    error_count_++;
    for(unsigned int n = 0; n < samples_.size(); n++)
      samples_[n] = 0;

    data_.setSamples(samples_);
    return(data_);
  }

  unsigned int values = bytes_received_/sizeof(float);

  for(unsigned int k = 0; k < values/blocks_ ; k++)
    for(unsigned int j = 0; j < blocks_; j++)
      samples_[ (k*blocks_) + j ] = *(reinterpret_cast<float*>(driver_buffer_ + HEADER_SIZE + (k +(j* values/blocks_) )*sizeof(float) ));

  data_.setSamples(samples_);
  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> USBamp::getAsyncData()
{
  #ifdef DEBUG
    cout << "USBamp: getAsyncData" << endl;
  #endif

  throw(std::runtime_error("USBamp::getAsyncData -- Async data acquisition not available for g.USBamp yet!"));
  
//   boost::shared_lock<boost::shared_mutex> lock(rw_);
//   vector<float> tmp(buffer);
//   samples_available_ = false;
//   lock.unlock();
//   return(tmp);
  return(data_);
}

//-----------------------------------------------------------------------------

void USBamp::check4USBampError()
{
  #ifdef DEBUG
    cout << "USBamp: check4USBampError" << endl;
  #endif

  error_code_ = 0;
  CHAR* error_ptr = error_msg_;

  if( !GT_GetLastError(&error_code_, error_ptr))
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

  if(serials_.find(m_.find(cst_.hardware_serial)->second) != serials_.end())
    throw(std::invalid_argument("USBamp::getHandles -- g.USBamp with serial "\
          +m_.find(cst_.hardware_serial)->second+" already in use!"));

  if(m_.find(cst_.hardware_serial)->second == "")
    throw(std::invalid_argument("USBamp::getHandles -- No serial number for g.USBamp given!"));

  serials_.insert(m_.find(cst_.hardware_serial)->second);

  HANDLE h_tmp;
  h_tmp = GT_OpenDeviceEx( const_cast<LPSTR>(  m_.find(cst_.hardware_serial)->second.c_str() ));
  if(h_tmp != 0)
  {
    h_ = (h_tmp);
    data_Ev_ = CreateEvent(0, false, false, 0);

    ov_.hEvent = data_Ev_;
    ov_.Offset = 0;
    ov_.OffsetHigh = 0;
  }
  else
    throw(std::runtime_error("USBamp::getHandles -- g.USBamp with serial "\
          +m_.find(cst_.hardware_serial)->second+" not connected!"));
}

//-----------------------------------------------------------------------------

void USBamp::initFilterPtrs()
{
  #ifdef DEBUG
    cout << "USBamp: initFilterPtrs" << endl;
  #endif

  if( !GT_GetNumberOfFilter(&nr_of_bp_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting number of filter!"));
  bp_filters_ = new FILT[nr_of_bp_filters_];
  if( !GT_GetFilterSpec(bp_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting filter specifications!"));

  if( !GT_GetNumberOfNotch(&nr_of_notch_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting number of notch!"));
  notch_filters_ = new FILT[nr_of_notch_filters_];
  if( !GT_GetNotchSpec(notch_filters_))
    throw(std::runtime_error("USBamp::initFilterPtrs -- Error getting notch specifications!"));
}

//-----------------------------------------------------------------------------

void USBamp::run()
{
  #ifdef DEBUG
    cout << "USBamp: run" << endl;
  #endif

  if( !GT_ResetTransfer(h_))
    throw(std::runtime_error("USBamp::run -- Error resetting transfer!"));

  ResetEvent(data_Ev_);

  if( !GT_Start(h_))
    throw(std::runtime_error("USBamp::run -- Error starting g.USBamp!"));

  running_ = 1;
}

//-----------------------------------------------------------------------------

void USBamp::stop()
{
  #ifdef DEBUG
    cout << "USBamp: stop" << endl;
  #endif

  boost::unique_lock<boost::shared_mutex> lock(rw_);
  running_ = 0;

  if( !GT_Stop(h_))
    throw(std::runtime_error("USBamp::stop -- Error stopping the device!"));
  if( !GT_CloseDevice(&h_))
    throw(std::runtime_error("USBamp::stop -- Error closing the device!"));

  h_ = 0;
  serials_.erase(m_.find(cst_.hardware_serial)->second);

  CloseHandle(data_Ev_);
  lock.unlock();

  cout << " * USBamp sucessfully stopped" << endl;
}

//-----------------------------------------------------------------------------

void USBamp::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setDeviceSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_fs,true));
    setSamplingRate(elem);

  elem = father->FirstChildElement(cst_.hw_channels,false);
  if(elem != elem.end())
    setDeviceChannels(elem);
  checkNrOfChannels();

  elem = father->FirstChildElement(cst_.hw_buffer,false);
  if(elem != elem.end())
    setBlocks(elem);

  setDefaultSettings();
  //---- optional ---

  elem = father->FirstChildElement(cst_.hw_fil,false);
  if(elem != elem.end())
    setDeviceFilterSettings(elem);

  elem = father->FirstChildElement(cst_.hw_notch,false);
  if(elem != elem.end())
    setDeviceNotchSettings(elem);

  elem = father->FirstChildElement(cst_.hw_opmode,false);
  if(elem != elem.end())
    setOperationMode(elem);

  elem = father->FirstChildElement(cst_.hw_sc,false);
  if(elem != elem.end())
    setShortCut(elem);

  elem = father->FirstChildElement(cst_.hw_trigger_line,false);
  if(elem != elem.end())
    setTriggerLine(elem);

  elem = father->FirstChildElement(cst_.hw_usbampmaster,false);
  if(elem != elem.end())
    setUSBampMasterOrSlave(elem);

  elem = father->FirstChildElement(cst_.hw_comgnd,false);
  if(elem != elem.end())
    setCommonGround(elem);

  elem = father->FirstChildElement(cst_.hw_comref,false);
  if(elem != elem.end())
    setCommonReference(elem);

  elem = father->FirstChildElement(cst_.hw_drl,false);
  if(elem != elem.end())
    setDrivenRightLeg(elem);
}

//---------------------------------------------------------------------------------------

void USBamp::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_sel,false));
  if (elem != elem.end())
    setChannelSelection(elem);
  checkNrOfChannels();

  elem = father->FirstChildElement(cst_.hw_fil,false);
  if(elem != elem.end())
    setChannelFilterSettings(elem);

  elem = father->FirstChildElement(cst_.hw_notch,false);
  if(elem != elem.end())
    setChannelNotchSettings(elem);

  elem = father->FirstChildElement(cst_.hw_bip,false);
  if(elem != elem.end())
    setBipolar(elem);

  elem = father->FirstChildElement(cst_.hw_drl,false);
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

  external_sync_ = 0;
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
  float f_low = 0;
  float f_high = 0;

  getFilterParams(elem, type, order, f_low, f_high);

  int id = search4FilterID(type, order, f_low, f_high);
  for(unsigned int n = 0; n < channel_info_.size(); n++)
    filter_id_[n] = id;

//   setUSBampFilter();
}

//---------------------------------------------------------------------------------------

void USBamp::setChannelFilterSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "USBamp: setChannelFilterSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem;
  elem = father->FirstChildElement(cst_.hw_cs_ch,false);
  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == cst_.hw_cs_ch)
    {
      if(!elem.Get()->HasAttribute(cst_.hw_ch_nr))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_fil+"> given, but channel number ("+cst_.hw_ch_nr+") not given!";
        throw(ticpp::Exception(ex_str));
      }
      checkFilterAttributes(elem);

      uint16_t ch = 0;
      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(cst_.hw_ch_nr) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_fil + "> : Channel is not a number!";
        throw(ticpp::Exception(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str;
        ex_str = "Error in"+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_fil + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(ticpp::Exception(ex_str));
      }

      unsigned int type = 0;
      unsigned int order = 0;
      float f_low = 0;
      float f_high = 0;

      getFilterParams(elem, type, order, f_low, f_high);

      uint16_t ch_pos = 0;
      map<uint16_t, pair<string, uint32_t> >::iterator it;
      for(it = channel_info_.begin(); it !=channel_info_.find(ch); it++)
        ch_pos++;
      filter_id_.at(ch_pos) = search4FilterID(type, order, f_low, f_high);
    }
    else
      throw(std::invalid_argument("USBamp::setChannelFilterSettings -- Tag not equal to \""+cst_.hw_cs_ch+"\"!"));
}

//---------------------------------------------------------------------------------------

void USBamp::getFilterParams(ticpp::Iterator<ticpp::Element>const &elem,\
  unsigned int &type, unsigned int &order, float &f_low, float &f_high)
{
  #ifdef DEBUG
    cout << "USBamp: getFilterParams" << endl;
  #endif

  type = cst_.getUSBampFilterType( elem.Get()->GetAttribute(cst_.hw_fil_type) );
  try
  {
    order = lexical_cast<unsigned int>( elem.Get()->GetAttribute(cst_.hw_fil_order) );

    f_low  = lexical_cast<float>( elem.Get()->GetAttribute(cst_.hw_fil_low) );
    f_high = lexical_cast<float>( elem.Get()->GetAttribute(cst_.hw_fil_high ));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, but order, lower or upper cutoff frequency is not a number!";
    throw(ticpp::Exception(ex_str));
  }
}

//---------------------------------------------------------------------------------------

void USBamp::checkFilterAttributes(ticpp::Iterator<ticpp::Element>const &elem)
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
  if(!elem.Get()->HasAttribute(cst_.hw_fil_order))
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_fil+"> given, filter order ("+cst_.hw_fil_order+") not given!";
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
}

//---------------------------------------------------------------------------------------

int USBamp::search4FilterID(unsigned int type, unsigned int order, float f_low, float f_high)
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
    string ex_str = "USBamp::search4FilterID -- Filter settings not possible -- ";
    ex_str = ex_str + "Fs: "   + lexical_cast<string>(fs_)  +  ", ";
    ex_str = ex_str + "Type: "   + lexical_cast<string>(type)  +  ", ";
    ex_str = ex_str + "Order: "  + lexical_cast<string>(order) +  ", ";
    ex_str = ex_str + "f_low: "  + lexical_cast<string>(f_low) +  ", ";
    ex_str = ex_str + "f_high: " + lexical_cast<string>(f_high);
    throw(std::invalid_argument(ex_str));
  }

  return(id);
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
  elem = father->FirstChildElement(cst_.hw_cs_ch,false);
  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == cst_.hw_cs_ch)
    {
      if(!elem.Get()->HasAttribute(cst_.hw_ch_nr))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_notch+"> given, but channel number ("+cst_.hw_ch_nr+") not given!";
        throw(ticpp::Exception(ex_str));
      }
      checkNotchAttributes(elem);

      uint16_t ch = 0;
      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(cst_.hw_ch_nr) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_notch + "> : Channel is not a number!";
        throw(ticpp::Exception(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str;
        ex_str = "Error in"+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_notch + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(ticpp::Exception(ex_str));
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
      throw(std::invalid_argument("USBamp::setChannelNotchSettings -- Tag not equal to \""+cst_.hw_cs_ch+"\"!"));
}

//---------------------------------------------------------------------------------------

void USBamp::checkNotchAttributes(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: checkNotchAttributes" << endl;
  #endif

  if(!elem.Get()->HasAttribute(cst_.hw_notch_center))
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_notch+"> given, notch center frequency ("+cst_.hw_notch_center+") not given!";
    throw(ticpp::Exception(ex_str));
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
    f_center = lexical_cast<float>( elem.Get()->GetAttribute(cst_.hw_notch_center));
  }
  catch(bad_lexical_cast &)
  {
    string ex_str;
    ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
    ex_str += "Tag <"+cst_.hw_notch+"> given, but center frequency is not a number!";
    throw(ticpp::Exception(ex_str));
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
    string ex_str = "USBamp::search4NotchID -- Notch settings not possible -- ";
    ex_str = ex_str + "f_center: " + lexical_cast<string>(f_center);
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

  //string op_mode( cst.getUSBampOpMode( elem->GetText(true) ) );

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

  enable_sc_ = cst_.equalsOnOrOff(elem->GetText(true));
}

//---------------------------------------------------------------------------------------

void USBamp::setTriggerLine(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setTriggerLine" << endl;
  #endif

  trigger_line_ = cst_.equalsOnOrOff(elem->GetText(true));
}

//---------------------------------------------------------------------------------------

void USBamp::setUSBampMasterOrSlave(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout << "USBamp: setUSBampMasterOrSlave" << endl;
  #endif

  external_sync_ = !(cst_.equalsYesOrNo(elem->GetText(true)));
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
    if(elem->Value() == cst_.hw_gnd)
    {
      if(!elem.Get()->HasAttribute(cst_.hw_gnd_block))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_comgnd+"> given, but block identifier ("+cst_.hw_gnd_block+") not given!";
        throw(ticpp::Exception(ex_str));
      }
      if(!elem.Get()->HasAttribute(cst_.hw_gnd_value))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_comgnd+"> given, but block value ("+cst_.hw_gnd_value+") not given!";
        throw(ticpp::Exception(ex_str));
      }

      int gnd = cst_.getUSBampBlockNr( elem.Get()->GetAttribute(cst_.hw_gnd_block) );
      bool b = cst_.equalsYesOrNo( elem.Get()->GetAttribute(cst_.hw_gnd_value) );

      v.at(gnd) = b;
    }
    else
      throw(std::invalid_argument("USBamp::setCommonGround -- Tag not equal to \""+cst_.hw_gnd+"\"!"));

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
    if(elem->Value() == cst_.hw_cr)
    {
      if(!elem.Get()->HasAttribute(cst_.hw_cr_block))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_comref+"> given, but block identifier ("+cst_.hw_cr_block+") not given!";
        throw(ticpp::Exception(ex_str));
      }
      if(!elem.Get()->HasAttribute(cst_.hw_cr_value))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_comref+"> given, but block value ("+cst_.hw_cr_value+") not given!";
        throw(ticpp::Exception(ex_str));
      }

      int ref = cst_.getUSBampBlockNr( elem.Get()->GetAttribute(cst_.hw_cr_block) );
      bool b = cst_.equalsYesOrNo( elem.Get()->GetAttribute(cst_.hw_cr_value) );

      v.at(ref) = b;
    }
    else
      throw(std::invalid_argument("USBamp::setCommonReference -- Tag not equal to \""+cst_.hw_cr+"\"!"));

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
  elem = father->FirstChildElement(cst_.hw_cs_ch,false);

  for(  ; elem != elem.end(); elem++)
    if(elem->Value() == cst_.hw_cs_ch)
    {
      if(!elem.Get()->HasAttribute(cst_.hw_ch_nr))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_bip+"> given, but channel number ("+cst_.hw_ch_nr+") not given!";
        throw(ticpp::Exception(ex_str));
      }
      if(!elem.Get()->HasAttribute(cst_.hw_bip_with))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_bip+"> given, but bipolar combination ("+cst_.hw_drl_value+") not given!";
        throw(ticpp::Exception(ex_str));
      }

      uint16_t with = 0;
      uint16_t ch   = 0;

      try{
        with = lexical_cast<uint16_t>( elem.Get()->GetAttribute(cst_.hw_bip_with) );
        ch   = lexical_cast<uint16_t>( elem.Get()->GetAttribute(cst_.hw_ch_nr) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_drl + "> : Channel is not a number!";
        throw(ticpp::Exception(ex_str));
      }
      if( (channel_info_.find(ch) ==  channel_info_.end()) || (channel_info_.find(with) ==  channel_info_.end()) )
      {
        string ex_str;
        ex_str = "Error in"+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_bip + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(ticpp::Exception(ex_str));
      }

      // FIXME  --  check channels, if in range ( USBAMP_MAX_NR... )

      v.at(ch) = with;
    }
    else
      throw(std::invalid_argument("USBamp::setBipolar -- Tag not equal to \""+cst_.hw_cs_ch+"\"!"));

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
  elem = father->FirstChildElement(cst_.hw_cs_ch,false);
  if(elem != elem.end())
  {
    setIndividualDrivenRightLeg(elem);
    return;
  }

  if(cst_.equalsYesOrNo(father->GetText(true)))
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
    if(elem->Value() == cst_.hw_cs_ch)
    {
      if(!elem.Get()->HasAttribute(cst_.hw_ch_nr))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_drl+"> given, but channel number ("+cst_.hw_ch_nr+") not given!";
        throw(ticpp::Exception(ex_str));
      }
      if(!elem.Get()->HasAttribute(cst_.hw_drl_value))
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+cst_.hw_drl+"> given, but DRL value ("+cst_.hw_drl_value+") not given!";
        throw(ticpp::Exception(ex_str));
      }

      bool b = cst_.equalsOnOrOff( elem.Get()->GetAttribute(cst_.hw_drl_value) );
      uint16_t ch   = 0;

      try{
        ch = lexical_cast<uint16_t>( elem.Get()->GetAttribute(cst_.hw_ch_nr) );
      }
      catch(bad_lexical_cast &)
      {
        string ex_str;
        ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_drl + "> : Channel is not a number!";
        throw(ticpp::Exception(ex_str));
      }
      if( channel_info_.find(ch) ==  channel_info_.end() )
      {
        string ex_str;
        ex_str = "Error in"+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
        ex_str += "Tag <"+ cst_.hw_drl + "> - Channel "+ lexical_cast<string>(ch) +" not set for recording!";
        throw(ticpp::Exception(ex_str));
      }

      v.at(ch-1) = b;
    }
    else
      throw(std::invalid_argument("USBamp::setIndividualDrivenRightLeg -- Tag not equal to \""+cst_.hw_cs_ch+"\"!"));

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
    if(it->first >= USBAMP_MAX_NR_OF_CHANNELS )
      throw(std::invalid_argument("Channel number too high -- maximum nr of channels: "\
      +lexical_cast<string>(USBAMP_MAX_NR_OF_CHANNELS)+ " -- (2 digital channels not supported yet!)" ) );
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

//   if(it->first <= USBAMP_MAX_NR_OF_ANALOG_CHANNELS )

  for(  ; it != stop; it++)
      channels.push_back(it->first);

  UCHAR* uc_channels = new UCHAR[channels.size()];
  for(unsigned int n = 0; n < channels.size(); n++)
    uc_channels[n] = channels[n];

  if( !GT_SetChannels(h_, uc_channels, channels.size()))
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
    if(filter_id_[count] > 0)
      check = GT_SetBandPass(h_, (*it).first, filter_id_[count]);
    count++;
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
    if(notch_id_[count] > 0)
      check = GT_SetNotch(h_, (*it).first, notch_id_[count]);
    count++;
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

//   if(channel_info_.find(USBAMP_TRIGGER_LINE_CHANNEL) != channel_info_.end())

  if(trigger_line_)
  {
    channel_info_[channel_info_.size() + 1] = make_pair(cst_.hw_trigger_line, SIG_USER_1);
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

  if( !GT_SetMode(h_, M_NORMAL) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting mode!"));

  if( !GT_SetSampleRate(h_, fs_))
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting sampling rate!"));

  if( !GT_SetBufferSize(h_, blocks_))
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting buffer size!"));

  setUSBampChannels();

  if( !GT_SetSlave(h_, external_sync_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting synchronization mode (master or slave)!"));

  if( !GT_EnableTriggerLine(h_, trigger_line_))
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting trigger line!"));

  if( !GT_EnableSC(h_, enable_sc_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting shortcut!"));

  if( !GT_SetBipolar(h_, bipolar_channels_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting bipolar channels!"));

  if( !GT_SetReference(h_, reference_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting reference!"));

  if( !GT_SetGround(h_, ground_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting ground!"));

  if( !GT_SetDRLChannel(h_, drl_channels_) )
    throw(std::runtime_error("USBamp::initUSBamp -- Error setting driven right leg channels!"));

  setUSBampFilter();
  setUSBampNotch();
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

#endif // WIN32
