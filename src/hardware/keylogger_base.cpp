#include "hardware/keylogger_base.h"
#include <vector>
#include <set>

#include <boost/bind.hpp>
#include <boost/thread.hpp>


#ifdef DEBUG
  #include <iostream>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::pair;

namespace tobiss
{

std::set<boost::uint16_t> KeyLoggerBase::used_ids_;

const std::string               KeyLoggerBase::str_detach_from_os_("detach_from_os");
boost::circular_buffer<char>    KeyLoggerBase::pressed_keycodes_(32);
boost::shared_mutex             KeyLoggerBase::static_rw_;
bool                            KeyLoggerBase::dirty_(1);

//-----------------------------------------------------------------------------

KeyLoggerBase::KeyLoggerBase(ticpp::Iterator<ticpp::Element> hw)
  : detach_from_os_(0), async_acqu_thread_(0), released_(1),
    id_(0)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  if(mode_ != APERIODIC)
    throw(std::invalid_argument("KeyLogger has to be started as aperiodic device!"));

  id_ = 256;
  while(used_ids_.find(id_) != used_ids_.end() )
    id_++;
  used_ids_.insert(id_);

  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));
  setDeviceSettings(ds);
  data_.init(1, channel_types_.size() , channel_types_);

  homogenous_signal_type_ = 0;

  std::vector<boost::uint32_t> v;
  empty_block_.init(0,0, v);

  setType("Keylogger");
}

//-----------------------------------------------------------------------------

KeyLoggerBase::~KeyLoggerBase()
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
}

//-----------------------------------------------------------------------------

void KeyLoggerBase::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(str_detach_from_os_,true));
  setDetachFromOS(elem);

  channel_types_.push_back(SIG_KEYCODE);
  channel_types_.push_back(SIG_KEYCODE);

  std::string naming("KeyCode ");
  for(unsigned int n = 0 ; n < 2; n++)
  {
    naming += boost::lexical_cast<std::string>(n);
    channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_KEYCODE)));
  }
}

//-----------------------------------------------------------------------------

void KeyLoggerBase::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif
}

//-----------------------------------------------------------------------------

//void KeyLoggerBase::init()
//{
//  #ifdef DEBUG
//    cout <<  BOOST_CURRENT_FUNCTION << endl;
//  #endif
//}

//-----------------------------------------------------------------------------

void KeyLoggerBase::setDetachFromOS(ticpp::Iterator<ticpp::Element>const &elem)
{
  #ifdef DEBUG
    cout <<  BOOST_CURRENT_FUNCTION << endl;
  #endif

  detach_from_os_ = equalsOnOrOff(elem->GetText(true));

  if(detach_from_os_)
    throw(std::invalid_argument("Detaching keyboard from OS not supported yet!"));
}

//-----------------------------------------------------------------------------

void KeyLoggerBase::run()
{
  if(mode_ == APERIODIC)
  {
    async_acqu_thread_ = new boost::thread( boost::bind(&KeyLoggerBase::acquireData, this) );
  }
  running_ = true;
}

//-----------------------------------------------------------------------------

void KeyLoggerBase::stop()
{
  running_ =false;
}

//-----------------------------------------------------------------------------

SampleBlock<double> KeyLoggerBase::getAsyncData()
{

  //cout <<  BOOST_CURRENT_FUNCTION << endl;

  if(!running_)
    return(empty_block_);

  boost::shared_lock<boost::shared_mutex> lock(static_rw_);

  if(!dirty_)
    return(empty_block_);

  std::vector<double> v;

  v.push_back(id_);
  while(!pressed_keycodes_.empty())
  {
    v.push_back(pressed_keycodes_.front());
    pressed_keycodes_.pop_front();
//    std::cout << v.back() << " -- ";
//    std::cout << static_cast<char>(v.back()) << std::flush;
  }
  //std::cout <<  std::endl;
  pressed_keycodes_.clear();
  lock.unlock();

  channel_types_.resize(v.size(),SIG_KEYCODE);
  data_.init(1, v.size() , channel_types_);
  data_.setSamples(v);

  dirty_ = false;
  return(data_);
}

//-----------------------------------------------------------------------------

}

