
#include "hardware/mouse.h"


namespace tobiss
{

set<boost::uint16_t> Mouse::used_ids_;

//-----------------------------------------------------------------------------

Mouse::Mouse(XMLParser& parser, ticpp::Iterator<ticpp::Element> hw)
  : HWThread(parser)
{
  #ifdef DEBUG
    cout << "Mouse: Constructor" << endl;
  #endif

  checkMandatoryHardwareTags(hw);
  if(mode_ != APERIODIC)
    throw(std::invalid_argument("Mouse has to be started as aperiodic device!"));
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

  initMouse();
  setDeviceSettings(0);

  data_.init(1, channel_types_.size() , channel_types_);

  vector<boost::uint32_t> v;
  empty_block_.init(0,0, v);
  cout << " * Mouse sucessfully initialized -- running as aperiodic: ";
  cout << (mode_ == APERIODIC) << ";  ";
  cout << "Mouse ID: " << id_ << ",  Name: " << name_ << endl;
}

//-----------------------------------------------------------------------------

Mouse::~Mouse()
{

}

//-----------------------------------------------------------------------------

void Mouse::setDeviceSettings(ticpp::Iterator<ticpp::Element>const&)
{
  #ifdef DEBUG
    cout << "Mouse: setDeviceSettings" << endl;
  #endif

//   ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_fs,true));
//   setSamplingRate(elem);

  //ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_channels,true));
//   elem = father->FirstChildElement(cst_.hw_channels,true);

  string naming;
  string type;

  if(buttons_)
    channel_types_.push_back(SIG_MBUTTON);
  for(boost::uint32_t n = 0; n < buttons_; n++)
    channel_types_.push_back(SIG_MBUTTON);

  if(axes_)
    channel_types_.push_back(SIG_MOUSE);
  for(boost::uint32_t n = 0; n < axes_; n++)
    channel_types_.push_back(SIG_MOUSE);
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
      channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(n, pair<string, boost::uint32_t>(naming, SIG_MOUSE)));


  homogenous_signal_type_ = 0;

}

//---------------------------------------------------------------------------------------

void Mouse::setChannelSettings(ticpp::Iterator<ticpp::Element>const& )
{
//  #ifdef DEBUG
    cout << "Mouse: setChannelSettings" << endl;
//  #endif

//   ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(cst_.hw_sel,false));
//   if (elem != elem.end())
//     setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Mouse::getAsyncData()
{
  #ifdef DEBUG
    cout << "Mouse: getAsyncData" << endl;
  #endif

  bool dirty = 0;
  int helpx,helpy;
  SDL_PumpEvents();
  int state = SDL_GetMouseState(&helpx, &helpy);

  // in Fenster einsperren:

//  if(helpx<100)
//    helpx = 101;
//  else if(helpx>wx_-100)
//  helpx = wx_-101;
//  if(helpy<100)
//    helpy = 101;
//  else if(helpy>wy_-100)
//    helpy = wy_-101;

   SDL_WarpMouse(helpx, helpy);
  
  //int test =  SDL_ShowCursor(SDL_DISABLE);
  //cout<<"  x,y: "<< helpx<<","<<helpy<<endl;


  for(uint n = 0; n < buttons_values_.size(); n++)
    if( (state&SDL_BUTTON(n+1)) != buttons_values_[n])
    {
      dirty = 1;
      buttons_values_[n] = state&SDL_BUTTON(n+1);
    }

  if(state&SDL_BUTTON(3))
  {
      cout<<"close window and exit program by pressing 3"<<endl;
      SDL_Quit();
      exit(0);
  }

    if(helpx != axes_values_[0] || helpy != axes_values_[1])
    {
      dirty = 1;
      axes_values_[0]=helpx;
      axes_values_[1]=helpy;
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

  data_.setSamples(v);
  return(data_);
}

//-----------------------------------------------------------------------------

void Mouse::run()  { }

//-----------------------------------------------------------------------------

void Mouse::stop() { }

//-----------------------------------------------------------------------------

void Mouse::initMouse()
{
  set<boost::uint16_t>::iterator it(used_ids_.begin());

  if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
  {
    string error_msg(SDL_GetError());
    throw(std::runtime_error("Mouse::Constructor -- Unable to init SDL: " + error_msg ));
  }

  axes_    = 2;
  buttons_ = 3;

  axes_values_.resize(axes_,0);
  buttons_values_.resize(buttons_,0);

  wx_ = 1000;
  wy_ = 800;
  SDL_Surface* screen = NULL;
  screen = SDL_SetVideoMode( wx_, wy_, 32, SDL_NOFRAME );
  SDL_ShowCursor(SDL_ENABLE);
  SDL_WarpMouse(100, 100);

}


//-----------------------------------------------------------------------------

} // Namespace tobiss
