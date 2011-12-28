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
* @file plux.cpp
**/

#include "hardware/plux.h"

#include <iostream>

namespace tobiss
{

using boost::thread;

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<Plux> Plux::FACTORY_REGISTRATOR_ ("plux", "bioplux");

//-----------------------------------------------------------------------------

Plux::Plux(ticpp::Iterator<ticpp::Element> hw)
  : HWThread(), device_(NULL), last_frame_seq_(-1)
{
  #ifdef DEBUG
    cout << "Plux: Constructor" << endl;
  #endif

  setHardware(hw);

  PLUX_TRY {

    std::map<std::string, std::string>::const_iterator it = m_.find( "mac" );

    if( it == m_.end() )
      devstr_ = findDevice( );
    else
      devstr_ = it->second;

    device_ = BP::Device::Create( devstr_ );

    device_->GetDescription( devinfo_ );
    if( devinfo_[ devinfo_.length()-1 ] == 13 )
      devinfo_ = devinfo_.substr( 0, devinfo_.length() - 1 );
    setType( devinfo_ );
  } PLUX_THROW

  if( !device_ )
    throw std::runtime_error( "Error during BioPlux Device Initialization." );

  data_.init( blocks_, nr_ch_, channel_types_ );
  frames_.resize( blocks_ );

  cout << endl;
  cout << " BioPlux Device: ";
  cout << devstr_ << " - " << devinfo_ << endl;

  if(!homogenous_signal_type_)
  {
    cout << "   ... NOTICE: Device is acquiring different signal types" << endl;
    //cout << "     --  ensure that reference and ground settings are correctly set!" << endl;
  }
}

//-----------------------------------------------------------------------------

Plux::~Plux( )
{
  #ifdef DEBUG
    cout << "Plux: Destructor" << endl;
  #endif

  if( device_ )
    delete device_;
}

//-----------------------------------------------------------------------------

void Plux::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "Plux: setHardware" << endl;
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

void Plux::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "Plux: setDeviceSettings" << endl;
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

void Plux::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father )
{
  #ifdef DEBUG
    cout << "Plux: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Plux::getSyncData()
{
  #ifdef DEBUG
    cout << "Plux: getSyncData" << endl;
  #endif

  PLUX_TRY {
    device_->GetFrames( blocks_, &frames_[0] );
  } PLUX_THROW

  convertFrames2SampleBlock( frames_, &data_ );

  return data_;
}

//---------------------------------------------------------------------------------------

void _test( const BP::Device::Frame &frame )
{
  cout << (int)frame.seq << " ";
}
    
void Plux::asyncAcquisitionThread( )
{
  #ifdef DEBUG
    cout << "Plux: asyncAcquisitionThread" << endl;
  #endif

  cout << " **** GO **** " << endl;
  while( run_async_ )
  {
    //boost::this_thread::sleep( boost::posix_time::milliseconds(100) );

    BP::Device::Frame frame;
    device_->GetFrames( 1, &frame );

    boost::mutex::scoped_lock lock( async_buffer_mutex_ );

    if( async_unread_ >= async_buffer_.capacity() )
    {
      cout << "Plux::asyncAcquisitionThread( ) - async_buffer overrun!" << endl;
      //throw std::runtime_error( "Plux::asyncAcquisitionThread( ) - async_buffer overrun!" );
    }
    else
      async_unread_++;

    async_buffer_.push_front( frame );

    lock.unlock( );
  }
  cout << " **** STOP **** " << endl;
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Plux::getAsyncData()
{
  #ifdef DEBUG
    cout << "Plux: getAsyncData" << endl;
  #endif

  for( int i=0; i<blocks_; i++ )
  {
    boost::mutex::scoped_lock lock( async_buffer_mutex_ );

    if( async_unread_ == 0 )
    {
      lock.unlock( );
      cout << "Plux::getAsyncData( ) - async_buffer underrun!" << endl;
      frames_[i] = BP::Device::Frame( );
    }
    else
    {
      frames_[i] = async_buffer_[--async_unread_];
      lock.unlock( );
    }
  }

  convertFrames2SampleBlock( frames_, &data_ );

  return data_;
}

//-----------------------------------------------------------------------------

void Plux::startAsyncAquisition( size_t buffer_size )
{
  async_buffer_.resize( buffer_size );
  async_unread_ = 0;
  run_async_ = true;
  async_acquisition_thread_ = thread( &Plux::asyncAcquisitionThread, this );
}

//-----------------------------------------------------------------------------

void Plux::stopAsyncAquisition( bool blocking )
{
  run_async_ = false;
  if( blocking )
    async_acquisition_thread_.join( );
}

//-----------------------------------------------------------------------------

void Plux::run()  
{
  #ifdef DEBUG
    cout << "Plux: run" << endl;
  #endif

  last_frame_seq_ = -1;

  BYTE nbits = 12;

  int fs = boost::numeric_cast<int>( fs_ );

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

  PLUX_TRY {
    device_->BeginAcq( fs, chmask, nbits );
  } PLUX_THROW

  if( !isMaster() )
    startAsyncAquisition( 100 );

  cout << " * " << devinfo_ << " (" << devstr_ << ") sucessfully started." << endl;
}

//-----------------------------------------------------------------------------

void Plux::stop() 
{
  #ifdef DEBUG
    cout << "Plux: stop" << endl;
  #endif

  if( !isMaster() )
    stopAsyncAquisition( );

  PLUX_TRY {
    device_->EndAcq();
  } PLUX_THROW
  

  cout << " * " << devinfo_ << " (" << devstr_ << ") sucessfully stopped." << endl;
}

//-----------------------------------------------------------------------------

void Plux::convertFrames2SampleBlock( const vector<BP::Device::Frame> &frames, SampleBlock<double> *data )
{
  data->reset( );

  samples_.resize( data->getNrOfChannels( ) );

  vector<BP::Device::Frame>::const_iterator frame = frames.begin( );
  for( ; frame!=frames.end(); frame++ )
  {
    int scount = 0;

    if( !checkSequenceNumber( frame->seq ) )
    {
      cout << "PLUX: Frame-Loss detected."  << endl;

      for( int i=0; i<data->getNrOfChannels( ); i++ )
        samples_[scount++] = samples_[scount];
    }
    else
    {    
      std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::const_iterator channel = channel_info_.begin( );
      for( int i=0; channel != channel_info_.end(); channel++, i++ )
        if( channel->first == 9 )
          samples_[scount++] = frame->dig_in;
        else
          samples_[scount++] = frame->an_in[i];
    }

    data->appendBlock( samples_, 1 );
  }
}

//-----------------------------------------------------------------------------

bool Plux::checkSequenceNumber( const BYTE id )
{
  if( last_frame_seq_ == 127 )
    last_frame_seq_ = -1;

  last_frame_seq_++;

  if( last_frame_seq_ == id )
    return true;

  last_frame_seq_ = id;

  return false;
}

//-----------------------------------------------------------------------------
    
std::string Plux::findDevice( )
{
  #ifdef DEBUG
    cout << "Plux: acquireDevice" << endl;
  #endif
  vector<string> devs;
  BP::Device::FindDevices(devs);

  if( devs.size() == 0 )
    throw std::runtime_error( "No BioPlux Devices found." );

  if( devs.size() > 1 )
  {
    string message( "Multiple BioPlux Devices found:\n" );
    for(int i=0; i < devs.size(); i++)
      message += "  " + devs[i] + "\n";
    message += "Don't know which one I should use.";
    throw std::runtime_error( message );
  }

  return devs.front( );
}

//-----------------------------------------------------------------------------

void Plux::rethrowPluxException(  BP::Err &err, bool do_throw )
{
    string message;
    switch( err.GetType() )
    {
    case BP::Err::TYP_NOTIFICATION:
      message = "BioPlux Notification: "; break;
    case BP::Err::TYP_ERROR:
      message = "BioPlux Error: "; break;
    default:
      message = "BioPlux Unknown Exception: "; break;
    }

    const char *tmp = err.GetDescription( );
    message += tmp;

    if( do_throw )
    {
      cout << " **** PLUX Exception:" << endl << "==============================================" << endl << message << endl << endl;
      throw(std::runtime_error( message ));
    }
    else
      cout << " **** PLUX Exception:" << endl << "==============================================" << endl << message << endl << endl;
}

//-----------------------------------------------------------------------------

} // Namespace tobiss
