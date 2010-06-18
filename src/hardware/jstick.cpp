
#include "hardware/jstick.h"

#include <SDL.h>

 set<boost::uint16_t> JStick::used_ids_;

//-----------------------------------------------------------------------------

JStick::JStick(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
  : HWThread(parser)
{
  #ifdef DEBUG
    cout << "JStick: Constructor" << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  if(mode_ != APERIODIC)
    throw(std::invalid_argument("Joystick has to be started as aperiodic device!"));
  //ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(cst_.hw_ds, true));



//   ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(cst_.hw_cs, false));
//   if (cs != cs.end())
//   {
//     for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
//       if(it->Value() == cst_.hw_cs)
//       {
//         string ex_str;
//         ex_str = "Error in "+ cst_.hardware_name +" - " + m_.find(cst_.hardware_name)->second + " -- ";
//         ex_str += "Multiple channel_settings found!";
//         throw(ticpp::Exception(ex_str));
//       }
//       setChannelSettings(cs);
//   }

  initJoystick();
  setDeviceSettings(0);


  data_.init(1, channel_types_.size() , channel_types_);

  vector<boost::uint32_t> v;
  empty_block_.init(0,0, v);
  cout << " * Joystick sucessfully initialized -- running as aperiodic: ";
  cout << (mode_ == APERIODIC) << ";  ";
  cout << "Jostick ID: " << id_ << ",  Name: " << name_ << endl;
}

//-----------------------------------------------------------------------------

JStick::~JStick()
{
  if(SDL_JoystickOpened(id_))
    SDL_JoystickClose(static_cast<SDL_Joystick*>(joy_));
}

//-----------------------------------------------------------------------------

void JStick::setDeviceSettings(ticpp::Iterator<ticpp::Element>const&)
{
  #ifdef DEBUG
    cout << "JStick: setDeviceSettings" << endl;
  #endif

//   ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_fs,true));
//   setSamplingRate(elem);

  //ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_channels,true));
//   elem = father->FirstChildElement(cst_.hw_channels,true);

  string naming;
  string type;

  if(buttons_)
    channel_types_.push_back(SIG_BUTTON);
  for(boost::uint32_t n = 0; n < buttons_; n++)
    channel_types_.push_back(SIG_BUTTON);

  if(axes_)
    channel_types_.push_back(SIG_JOYSTICK);
  for(boost::uint32_t n = 0; n < axes_; n++)
    channel_types_.push_back(SIG_JOYSTICK);

  if(balls_)
    channel_types_.push_back(SIG_JOYSTICK);
  for(boost::int32_t n = 0; n < 2*balls_; n++)
    channel_types_.push_back(SIG_JOYSTICK);

  nr_ch_= channel_types_.size();

  //try
  //{
  //  parser_.parseDeviceChannels(elem, nr_ch_, naming, type);
  //}
  //catch(ticpp::Exception& e)
  //{
  //  string ex_str;
  //  ex_str = "Error in "+ cst_.hardware +" - " + m_.find(cst_.hardware_name)->second + " -- ";
  //  throw(ticpp::Exception(ex_str + e.what()));
  //}

  boost::uint16_t n = 1;
  if(buttons_)
    for( ; n <= buttons_ +1; n++)
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_BUTTON)));

  if(axes_)
    for( ; n <= axes_ + buttons_ +2; n++)
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_JOYSTICK)));

  if(balls_)
    for( ; n <= (2*balls_)+ axes_ + buttons_ +3; n++)
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_JOYSTICK)));

  homogenous_signal_type_ = 0;

}

//---------------------------------------------------------------------------------------

void JStick::setChannelSettings(ticpp::Iterator<ticpp::Element>const& )
{
  #ifdef DEBUG
    cout << "JStick: setChannelSettings" << endl;
  #endif

//   ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_sel,false));
//   if (elem != elem.end())
//     setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

SampleBlock<double> JStick::getAsyncData()
{
  #ifdef DEBUG
    cout << "JStick: getAsyncData" << endl;
  #endif

  bool dirty = 0;

  SDL_JoystickUpdate();

  for(boost::uint8_t n = 0; n < buttons_values_.size(); n++)
    if(SDL_JoystickGetButton(static_cast<SDL_Joystick*>(joy_), n) != buttons_values_[n])
    {
      dirty = 1;
      buttons_values_[n] = SDL_JoystickGetButton(static_cast<SDL_Joystick*>(joy_), n);
    }

  for(boost::uint8_t n = 0; n < axes_values_.size(); n++)
    if(SDL_JoystickGetAxis(static_cast<SDL_Joystick*>(joy_), n) != axes_values_[n])
    {
      dirty = 1;
      axes_values_[n] = SDL_JoystickGetAxis(static_cast<SDL_Joystick*>(joy_), n);
    }

  for(boost::uint8_t n = 0; n < balls_values_.size(); n++)
  {
    int dx = 0;
    int dy = 0;
    SDL_JoystickGetBall(static_cast<SDL_Joystick*>(joy_), n, &dx, &dy);
    pair<int,int> dxy = make_pair(dx,dy);
    if(dxy != balls_values_[n])
    {
      dirty = 1;
      balls_values_[n] = dxy;
    }
  }

  if(!dirty)
    return(empty_block_);

  vector<double> v;

  if(buttons_)
    v.push_back(id_);
  for(boost::uint8_t n = 0; n < buttons_values_.size(); n++)
      v.push_back(buttons_values_[n]);

  if(axes_)
    v.push_back(id_);
  for(boost::uint8_t n = 0; n < axes_values_.size(); n++)
    v.push_back(axes_values_[n]);

  if(balls_)
    v.push_back(id_);
  for(boost::uint8_t n = 0; n < balls_values_.size(); n++)
  {
    v.push_back(balls_values_[n].first);
    v.push_back(balls_values_[n].second);
  }

  data_.setSamples(v);
  return(data_);
}

//-----------------------------------------------------------------------------

void JStick::run()  { }

//-----------------------------------------------------------------------------

void JStick::stop() { }

//-----------------------------------------------------------------------------

void JStick::initJoystick()
{
  set<boost::uint16_t>::iterator it(used_ids_.begin());

  boost::int16_t nr_of_jsticks = 0;
  if ( SDL_Init(SDL_INIT_JOYSTICK) < 0 )
  {
    string error_msg(SDL_GetError());
    throw(std::runtime_error("JStick::Constructor -- Unable to init SDL: " + error_msg ));
  }
  if( (nr_of_jsticks = SDL_NumJoysticks()) <= 0 )
    throw(std::runtime_error("JStick::initJoystick -- No Joysticks found!"));

  for(boost::uint8_t n = 0; n < nr_of_jsticks;  n++)
    if(used_ids_.find(n) == used_ids_.end())
    {
      joy_ = SDL_JoystickOpen(n);
      if(!static_cast<SDL_Joystick*>(joy_))
        throw(std::runtime_error("JStick::initJoystick -- Unable to opne joystick at position: " + n));
      name_= SDL_JoystickName(n);
      id_ = n;
      used_ids_.insert(n);
      break;
    }

  if(!static_cast<SDL_Joystick*>(joy_))
    throw(std::runtime_error("JStick::initJoystick -- Joystick already opened!"));

  axes_    = SDL_JoystickNumAxes(static_cast<SDL_Joystick*>(joy_));
  buttons_ = SDL_JoystickNumButtons(static_cast<SDL_Joystick*>(joy_));
  balls_   = SDL_JoystickNumBalls(static_cast<SDL_Joystick*>(joy_));

  axes_values_.resize(axes_,0);
  balls_values_.resize(balls_, make_pair(0,0) );
  buttons_values_.resize(buttons_,0);

  SDL_JoystickEventState(SDL_ENABLE);
  SDL_JoystickUpdate();
}

//-----------------------------------------------------------------------------

