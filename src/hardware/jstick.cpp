/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#include "hardware/jstick.h"

#include "SDL/SDL.h"

namespace tobiss
{

using std::vector;
using std::string;
using std::pair;
using std::cout;
using std::endl;
using std::make_pair;
using std::set;

set<boost::uint16_t> JStick::used_ids_;


const HWThreadBuilderTemplateRegistratorWithoutIOService<JStick> JStick::FACTORY_REGISTRATOR_ ("jstick", "joystick", "joycable");

//-----------------------------------------------------------------------------
JStick::JStick(ticpp::Iterator<ticpp::Element> hw)
  : HWThread()
{
  #ifdef DEBUG
    cout << "JStick: Constructor" << endl;
  #endif

  setType("Joystick");
  checkMandatoryHardwareTags(hw);
  if(mode_ != APERIODIC)
    throw(std::invalid_argument("Joystick has to be started as aperiodic device!"));

  initJoystick();
  setDeviceSettings(0);

  data_.init(1, channel_types_.size() , channel_types_);

  vector<boost::uint32_t> v;
  empty_block_.init(0,0, v);
  cout << " * Joystick sucessfully initialized -- running as aperiodic: ";
  cout << (mode_ == APERIODIC) << ";  ";
  cout << "Jostick ID: " << id_ << ",  Name: " << name_ << endl;
  cout << " ... buttons: " << buttons_;
  cout << ", axes: " << axes_;
  cout << ", balls: " << balls_ << endl;
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

}

//---------------------------------------------------------------------------------------

SampleBlock<double> JStick::getAsyncData()
{
  #ifdef DEBUG
    cout << "JStick: getAsyncData" << endl;
  #endif

  bool dirty = 0;

  if(!SDL_JoystickOpened(id_) )
    throw(std::runtime_error("JStick::getAsyncData -- Joystick not opened any more -- ID: " + id_) );


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
        throw(std::runtime_error("JStick::initJoystick -- Unable to open joystick at position: " + n));
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

} // Namespace tobiss
