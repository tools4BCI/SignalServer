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

enum id_check
{
  CHK_ID_REPEATED,
  CHK_ID_OK,
  CHK_ID_MISSED,
};

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<Plux> Plux::FACTORY_REGISTRATOR_( "plux", "bioplux" );

//-----------------------------------------------------------------------------

Plux::Plux(ticpp::Iterator<ticpp::Element> hw)
  : HWThread(), device_(NULL), async_buffer_( 0 )
{
  #ifdef DEBUG
    cout << "Plux: Constructor" << endl;
  #endif

  setHardware(hw);

  std::map<std::string, std::string>::const_iterator it;

  try {

    it = m_.find( "mac" );
    if( it == m_.end() )
      devstr_ = findDevice( );
    else
      devstr_ = it->second;

    device_ = BP::Device::Create( devstr_ );

    device_->GetDescription( devinfo_ );
    if( devinfo_[ devinfo_.length()-1 ] == 13 )
      devinfo_ = devinfo_.substr( 0, devinfo_.length() - 1 );
    setType( devinfo_ );
  } catch( BP::Err &err ) { rethrowPluxException( err, true ); }

  if( !device_ )
    throw std::runtime_error( "Error during BioPlux Device Initialization." );

  data_.init( blocks_, nr_ch_, channel_types_ );
  frames_.resize( blocks_ );

  cout << endl;
  cout << " BioPlux Device: ";
  cout << devstr_ << " - " << devinfo_ << endl;

  it = m_.find( "statupdate" );
  if( it == m_.end() )
    slave_statistics_.statistics_interval_ = 0;
  else
    slave_statistics_.statistics_interval_ = lexical_cast<unsigned int>( it->second );

  if(!homogenous_signal_type_)
  {
    cout << "   ... NOTICE: Device is acquiring different signal types" << endl;
  }
}

//-----------------------------------------------------------------------------

Plux::~Plux( )
{
  #ifdef DEBUG
    cout << "Plux: Destructor" << endl;
  #endif

  //async_acquisition_thread_.interrupt( );

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

  frametype frame;

  for( int i=0; i<blocks_; i++ )
  {
    try {
      device_->GetFrames( 1, &frame.frame );
    } catch( BP::Err &err ) { rethrowPluxException( err, true ); }

    if( seq_expected.valid() )
      while( seq_expected < frame.frame.seq )
      {
        // Frame is newer than expected. That can only mean packets were lost.
        // interpolate.
        frametype iframe = frametype( last_frame_, frame, seq_expected );
        last_frame_ = iframe;
        seq_expected++;
        frames_[i] = iframe.frame;
      }

    if( !seq_expected.valid() || seq_expected == frame.frame.seq )
    {
      // Exactly what we expected. Just use that frame.
      last_frame_ = frame;
      seq_expected = frame.frame.seq + 1;
      frames_[i] = frame.frame;
    }
    else
    {
      // Frame is older than expected. That cannot happen in synch mode.
      // -> critiical failure
      std::cout << "Plux::getSyncData(): Unexpected Sample Number." << endl;
      throw std::runtime_error( "Plux::getSyncData(): Unexpected Sample Number." );
    }

  }

  //boost::this_thread::sleep( boost::posix_time::milliseconds(1) );

  convertFrames2SampleBlock( );

  return data_;
}

//---------------------------------------------------------------------------------------
    
void Plux::asyncAcquisitionThread( )
{
  #ifdef DEBUG
    cout << "Plux: asyncAcquisitionThread" << endl;
  #endif
    
  BP::Device::Frame frame;

  cout << " **** GO **** " << endl;
  while( !async_acquisition_thread_.interruption_requested() )
  {
    //boost::this_thread::sleep( boost::posix_time::milliseconds(100) );

    device_->GetFrames( 1, &frame );

    try {
      //async_buffer_.insert_overwriting( frametype( frame, boost::posix_time::microsec_clock::local_time() ) );
      async_buffer_.insert_throwing( frametype( frame, boost::posix_time::microsec_clock::local_time() ) );
      //async_buffer_.insert_blocking( frametype( frame, boost::posix_time::microsec_clock::local_time() ) );
    }
    catch( DataBuffer<frametype>::buffer_overrun &e )
    {
      cout << e.what( ) << endl;
      throw e;
    }
  }
  cout << " **** STOP **** " << endl;
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Plux::getAsyncData()
{
  #ifdef DEBUG
    cout << "Plux: getAsyncData" << endl;
  #endif

  frametype frame;

  for( int i=0; i<blocks_; i++ )
  {
    try {
      async_buffer_.peekNext_throwing( &frame );

      if( seq_expected.valid() )
        while( seq_expected > frame.frame.seq )
        {
          // Frame is older than expected. Likely it was substituted previously.
          // Drop it.
          // But still use it as last_frame. (TODO: is that a good idea?)
          last_frame_ = frame;
          async_buffer_.dropOldest( );
          async_buffer_.peekNext_throwing( &frame );
          slave_statistics_.frames_dropped_++;
        }

      if( !seq_expected.valid() || seq_expected == frame.frame.seq )
      {
        // Exactly what we expected. Just use that frame.
        last_frame_ = frame;
        seq_expected = frame.frame.seq + 1;
        async_buffer_.dropOldest( );
      }
      else
      {
        // Frame is newer than expected. That can only mean packets were lost.
        // Leave in buffer, interpolate.
        frame = frametype( last_frame_, frame, seq_expected );
        last_frame_ = frame;
        seq_expected++;
        slave_statistics_.frames_lost_++;
      }

      boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - last_frame_.time;
      slave_statistics_.time_statistics_.update( diff.total_milliseconds() );
    }
    catch( DataBuffer<frametype>::buffer_underrun &e )
    {
      // No sample available.
      // Substitute some data... (re-use last frame)
      frame = last_frame_;
      seq_expected++;
      last_frame_.frame.seq = seq_expected.cast<BYTE>();
      slave_statistics_.frames_repeated_++;
    }
    frames_[i] = frame.frame;
  }

  /*if( async_buffer_.getNumAvail() < 100 )
      seq_expected--;

  if( async_buffer_.getNumAvail() > 200 )
      seq_expected++;*/

  if( slave_statistics_.statistics_interval_ > 0 )
  {    
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    if( (now - slave_statistics_.last_printed_).total_milliseconds() > 1000*slave_statistics_.statistics_interval_ )
    {
      slave_statistics_.last_printed_ = now;
      printAsyncStatistics( );
    }
  }

  convertFrames2SampleBlock( );

  return data_;
}

//-----------------------------------------------------------------------------

void Plux::startAsyncAquisition( size_t buffer_size )
{
  slave_statistics_.reset( );

  async_buffer_.resize( buffer_size );
  async_acquisition_thread_ = thread( &Plux::asyncAcquisitionThread, this );
    
  // buffer a few samples...
  while( async_buffer_.getNumAvail( ) <= 2 * blocks_ );
}

//-----------------------------------------------------------------------------

void Plux::stopAsyncAquisition( bool blocking )
{
  async_acquisition_thread_.interrupt( );
  if( blocking )
    async_acquisition_thread_.join( );
}

//-----------------------------------------------------------------------------

void Plux::printAsyncStatistics( )
{ 
  cout << endl;
  cout << boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ) << endl;
  cout << " Slave device: " << devinfo_ << " (" << devstr_ << ")" << endl;
  cout << "  Lost: " << slave_statistics_.frames_lost_ << endl;
  cout << "  repeated: " << slave_statistics_.frames_repeated_ << endl;
  cout << "  dropped: " << slave_statistics_.frames_dropped_ << endl;
  cout << "  Bilanz: " << slave_statistics_.frames_repeated_ - slave_statistics_.frames_dropped_ << endl;
  cout << "  Unread Frames: " << async_buffer_.getNumAvail( ) << endl;
  cout << "       Frame delay (milliseconds) " << endl;
  cout << "           mean: " << slave_statistics_.time_statistics_.get_mean( ) << endl;
  cout << "  adaptive mean: " << slave_statistics_.time_statistics_.get_adaptive_mean() << endl;
  cout << "  adaptive  std: " << sqrt(slave_statistics_.time_statistics_.get_adaptive_var()) << endl;
  cout << "=============================" << endl;
}

//-----------------------------------------------------------------------------

void Plux::run()  
{
  #ifdef DEBUG
    cout << "Plux: run" << endl;
  #endif

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

  seq_expected.setInvalid( );

  try {
    device_->BeginAcq( fs, chmask, nbits );
  } catch( BP::Err &err ) { rethrowPluxException( err, true ); }

  if( !isMaster() )
  {
    startAsyncAquisition( 1000 );

  }

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

  try {
    device_->EndAcq();
  } catch( BP::Err &err ) { rethrowPluxException( err, true ); }
  

  cout << " * " << devinfo_ << " (" << devstr_ << ") sucessfully stopped." << endl;
}

//-----------------------------------------------------------------------------

void Plux::convertFrames2SampleBlock( )
{
  data_.reset( );

  samples_.resize( data_.getNrOfChannels( ) );

  vector<BP::Device::Frame>::const_iterator frame = frames_.begin( );
  for( ; frame!=frames_.end(); frame++ )
  {

    int scount = 0;
    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::const_iterator channel = channel_info_.begin( );
    for( int i=0; channel != channel_info_.end(); channel++, i++ )
      if( channel->first == 9 )
        samples_[scount++] = frame->dig_in;
      else
        samples_[scount++] = frame->an_in[i];

    data_.appendBlock( samples_, 1 );
  }
}

//-----------------------------------------------------------------------------

/*int Plux::checkSequenceNumber( const BYTE id )
{
  if( last_frame_seq_ == 127 )
    last_frame_seq_ = -1;

  if( last_frame_seq_ == id )
    return CHK_ID_REPEATED;

  last_frame_seq_++;

  if( last_frame_seq_ == id )
    return CHK_ID_OK;

  last_frame_seq_ = id;

  return CHK_ID_MISSED;
}*/

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
//-----------------------------------------------------------------------------

Plux::SequenceNumber::SequenceNumber( int s )
{
  if( s < 0 )
    s = -1;
  else while( s >= 128 )
    s -= 128;
  seq = s;
}

//-----------------------------------------------------------------------------


bool Plux::SequenceNumber::valid( )
{
  return seq >= 0;
}

//-----------------------------------------------------------------------------

void Plux::SequenceNumber::operator++( int )
{
  if( seq == 127 )
    seq = 0;
  else
    seq++;
}

//-----------------------------------------------------------------------------

void Plux::SequenceNumber::operator--( int )
{
  if( seq == 0 )
    seq = 127;
  else
    seq--;
}

//-----------------------------------------------------------------------------

bool Plux::SequenceNumber::operator==( const seqtype &s ) const
{
  return seq == s;
}

//-----------------------------------------------------------------------------

bool Plux::SequenceNumber::operator<( const seqtype &s ) const
{
  if( seq == s )
    return false;

  if( seq < s )
    if( s-seq >= 64 )
      return false;
    else
      return true;
  else
    if( seq-s > 64 )
      return true;
    else
      return false;
}

//-----------------------------------------------------------------------------

bool Plux::SequenceNumber::operator>( const seqtype &s ) const
{
  if( seq == s )
    return false;

  if( seq < s )
    if( s-seq >= 64 )
      return true;
    else
      return false;
  else
    if( seq-s > 64 )
      return false;
    else
      return true;
}

//-----------------------------------------------------------------------------


Plux::frametype::frametype( const frametype &a, const frametype &b, const SequenceNumber &seq )
{        
  double factor = (seq.cast<double>()-a.frame.seq)/(b.frame.seq-a.frame.seq);

  time = a.time + boost::posix_time::microseconds( (b.time - a.time).total_microseconds() * factor );

  for( int i=0; i<8; i++ )
    frame.an_in[i] = a.frame.an_in[i] + boost::numeric_cast<int>( (b.frame.an_in[i] - a.frame.an_in[i]) * factor );

  if( factor < 0.5 )
    frame.dig_in = a.frame.dig_in;
  else
    frame.dig_in = b.frame.dig_in;

  frame.seq = seq.cast<BYTE>();
}

} // Namespace tobiss
