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
* @file TMSi.cpp
**/

#include "hardware/tmsi.h"

#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <iostream>

namespace tobiss
{

using boost::thread;
using boost::lexical_cast;

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

//-----------------------------------------------------------------------------

// Place XML declarations here

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<TMSi> TMSi::FACTORY_REGISTRATOR_( "TMSi", "tmsi" );

//-----------------------------------------------------------------------------

// TODO: remove the TODO exception :)

class TODOexception : public std::exception
{
  std::string msg;
public:
  TODOexception( const std::string &arg, const char *file, int line) : std::exception()
  {
    std::ostringstream o;
    o << "TODO: " << file << ":" << line << ": " << arg;
    msg = o.str();
  }
  ~TODOexception() throw() {}
  const char *what() const throw()
  {
    return msg.c_str();
  }
};

#define TODO(arg) throw TODOexception(arg, __FILE__, __LINE__);

//-----------------------------------------------------------------------------

TMSi::TMSi(ticpp::Iterator<ticpp::Element> hw)
  : HWThread()
{
  #ifdef DEBUG
    cout << "TMSi: Constructor" << endl;
  #endif

  setHardware(hw);

  std::map<std::string, std::string>::const_iterator it;

    // Initialize Device instance.
#ifdef DISABLE_TMSi_COMPILE
    throw std::runtime_error( std::string("TMSi support is disabled in this build of SignalServer: ") + DISABLE_TMSi_REASON );
#else
  #ifdef DEBUG
    cout << "TMSi: Initialization..." << endl;
  #endif
  // TODO: init TMSi here
  TODO("initialize TMSi")
#endif

    // TODO: get device name or something from the TMSi API
    setType( "TMSi Device" );

  // Initialize and pre-allocate members.
  data_.init( blocks_, nr_ch_, channel_types_ );

  // Print some information.
  //cout << endl;
  //cout << " TMSi Device: ";
  //cout << devstr_ << " - " << devinfo_ << endl;

  #ifdef DEBUG
    cout << "TMSi: Leaving Constructor." << endl;
  #endif
}

//-----------------------------------------------------------------------------

TMSi::~TMSi( )
{
  #ifdef DEBUG
    cout << "TMSi: Destructor" << endl;
  #endif

  if( !isMaster() )
    stopAsyncAquisition( true );

  TODO("Destroy Device")
}

//-----------------------------------------------------------------------------

void TMSi::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "TMSi: setHardware" << endl;
  #endif

  checkMandatoryHardwareTags(hw);

  // parse hardware attributes
  ticpp::Iterator<ticpp::Element> elem(hw);
  ticpp::Iterator< ticpp::Attribute > attribute;
  for(attribute = attribute.begin(elem.Get()); attribute != attribute.end();
      attribute++)
      m_.insert(pair<string, string>(attribute->Name(), attribute->Value()));

  // parse device settings
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));
  setDeviceSettings(ds);

  // parse channel selection
  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_chset_, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_chset_)
      {
      string ex_str(type_ + " -- ");
        ex_str += "Multiple channel_settings found!";
        throw(std::invalid_argument(ex_str));
      }
      setChannelSettings(cs);
  }
}

//-----------------------------------------------------------------------------

void TMSi::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "TMSi: setDeviceSettings" << endl;
  #endif
  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs_,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);
}

//---------------------------------------------------------------------------------------

void TMSi::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father )
{
  #ifdef DEBUG
    cout << "TMSi: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

SampleBlock<double> TMSi::getSyncData()
{
  #ifdef DEBUG
    cout << "TMSi: getSyncData" << endl;
  #endif

  data_.reset( );

  samples_.resize( data_.getNrOfChannels( ) );

  TODO("read data from TMSi API")

  for( int i=0; i<blocks_; i++ ) // sample i
  {
    int scount = 0;
    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::const_iterator channel = channel_info_.begin( );
    for( int j=0; channel != channel_info_.end(); channel++, j++ )
    {
      // tia channel: scount
      // dev channel: j or get from channel_info_?
      samples_[scount++] = 0;
    }

    data_.appendBlock( samples_, 1 );
  }

  return data_;
}

//---------------------------------------------------------------------------------------
    
void TMSi::asyncAcquisitionThread( )
{
  #ifdef DEBUG
    cout << "TMSi: asyncAcquisitionThread" << endl;
  #endif

  TODO("implement asynchronous TMSi acquisition")

  /*try {
    while( !async_acquisition_thread_.interruption_requested() )
    {
      //async_buffer_.insert_overwriting( getNextFrame( ) );
      //async_buffer_.insert_throwing( getNextFrame( ) );
      //async_buffer_.insert_blocking( getNextFrame( ) );
    }
  }
  catch( std::exception &e )
  {
    cout << "TMSi::asyncAcquisitionThread( ): caught exception:" << endl;
    cout << e.what( ) << endl;
    throw e;
  }*/
}

//---------------------------------------------------------------------------------------

SampleBlock<double> TMSi::getAsyncData()
{
  #ifdef DEBUG
    cout << "TMSi: getAsyncData" << endl;
  #endif

  TODO("implement asynchronous TMSi acquisition")

  data_.reset();

  return data_;
}

//-----------------------------------------------------------------------------

void TMSi::startAsyncAquisition( )
{
  TODO("implement asynchronous TMSi acquisition")
}

//-----------------------------------------------------------------------------

void TMSi::stopAsyncAquisition( bool blocking )
{
  async_acquisition_thread_.interrupt( );
  if( blocking )
    async_acquisition_thread_.join( );
}

//-----------------------------------------------------------------------------

void TMSi::run()  
{
  #ifdef DEBUG
    cout << "TMSi: run" << endl;
  #endif

  // Number of bits per sample to record. 8 or 12
  // FIXME ... hard coded value!
  BYTE nbits = 12;

  // TMSi requires integer sampling rates
  int fs = boost::numeric_cast<int>( fs_ );
  if( boost::numeric_cast<double>( fs ) != fs_ )
    throw std::invalid_argument( "TMSi::run() - Invalid Sampling Rate." );

  // Create channel mask.
  // That is a bit mask which tells the Plux Device what channels to record.
  TODO("configure TMSi to use channels acoording to channel_info_")
  BYTE chmask = 0;
  std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::const_iterator channel = channel_info_.begin( );
  for( ; channel != channel_info_.end(); channel++ )
  {
    if( channel->first >= 9 )
      continue;

    // m = 2^(cannel->first - 1) 
    BYTE m = 1;
    for( int p=0; p<channel->first-1; p++ )
      m *= 2;

    chmask += m;
  }

  TODO("start TMSi acquisition")

  if( !isMaster() )
  {
    startAsyncAquisition( );
  }

  cout << " * TMSi sucessfully started." << endl;
}

//-----------------------------------------------------------------------------

void TMSi::stop() 
{
  #ifdef DEBUG
    cout << "TMSi: stop" << endl;
  #endif

  TODO("stop TMSi acqusition")

  cout << "Stopping TMSi ...";

  if( !isMaster() )
    stopAsyncAquisition( );

  cout << " OK" << endl;
}

} // Namespace tobiss
