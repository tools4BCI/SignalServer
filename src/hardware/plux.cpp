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

const string Plux::hw_buffersize_("buffersize");

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

    // Try to discover the BioPlux device name, if no mac address was provided.
    it = m_.find( "mac" );
    if( it == m_.end() )
      devstr_ = findDevice( );
    else
      devstr_ = it->second;

    // Initialize BioPlux Device instance.
#ifdef DISABLE_PLUX_COMPILE
    throw std::runtime_error( std::string("BioPlux support is disabled in this build of SignalServer: ") + DISABLE_PLUX_REASON );
#else
    device_ = BP::Device::Create( devstr_ );
#endif

    // Put device description into HWThread type info.
    device_->GetDescription( devinfo_ );
    if( devinfo_[ devinfo_.length()-1 ] == 13 )
      devinfo_ = devinfo_.substr( 0, devinfo_.length() - 1 );
    setType( devinfo_ );

  } catch( BP::Err &err ) { rethrowPluxException( "Plux::Plux()", err, true ); }

  if( !device_ )
    throw std::runtime_error( "Error during BioPlux Device Initialization." );

  // Set statistics output interval, or disable the output.
  it = m_.find( "statoutput" );
  if( it == m_.end() )
    statistics_.statistics_interval_ = 0;
  else
    statistics_.statistics_interval_ = lexical_cast<unsigned int>( it->second );

  // Initialize and pre-allocate members.
  data_.init( blocks_, nr_ch_, channel_types_ );
  frames_.resize( blocks_ );
  statistics_.reset( );

  // Print some information.
  cout << endl;
  cout << " BioPlux Device: ";
  cout << devstr_ << " - " << devinfo_ << endl;
}

//-----------------------------------------------------------------------------

Plux::~Plux( )
{
  #ifdef DEBUG
    cout << "Plux: Destructor" << endl;
  #endif

  if( !isMaster() )
    stopAsyncAquisition( true );

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

  buffersize_ = 50;  // FIXME ... hard coded default
  elem = father->FirstChildElement(hw_buffersize_,false);
  if(elem != elem.end())
    buffersize_ = lexical_cast<DataBuffer<frametype>::size_type>(elem->GetText(true));
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

Plux::frametype Plux::getNextFrame( )
{
  //frametype frame;

  boost::posix_time::ptime now =  boost::posix_time::microsec_clock::local_time();

  if( !frame_pending_ )
  {
    try {
      device_->GetFrames( 1, &pending_frame_.frame );
      pending_frame_.time = now;
      frame_pending_ = true;
    } catch( BP::Err &err ) { rethrowPluxException( "Plux::getNextFrame()", err, true ); }
  }

  if( !seq_expected.valid() || seq_expected == pending_frame_.frame.seq )
  {
    last_frame_ = pending_frame_;
    seq_expected++;
    frame_pending_ = false;
  }
  else
  //if( seq_expected.valid() && seq_expected < frame.frame.seq )
  {
    // Frame is newer than expected. That can only mean packets were lost.
    // interpolate.
    frametype iframe = frametype( last_frame_, pending_frame_, seq_expected );
    last_frame_ = iframe;
    seq_expected++;
    statistics_.frames_lost_++;
  }
  /*else
  {
    // Frame is older than expected. That cannot happen in synch mode.
    // -> critical failure
    std::cout << "Plux::getNextFrame(): Unexpected Sample Number." << endl;
    throw std::runtime_error( "Plux::getNextFrame(): Unexpected Sample Number." );
  }*/

  statistics_.rate_statistics_.update( (now - last_time_).total_microseconds() );
  last_time_ = now;

  return last_frame_;

}

//---------------------------------------------------------------------------------------

SampleBlock<double> Plux::getSyncData()
{
  #ifdef DEBUG
    cout << "Plux: getSyncData" << endl;
  #endif


  for( int i=0; i<blocks_; i++ )
  {
    frames_[i] = getNextFrame( ).frame;
  }
  
  num_frames_total_ += blocks_;
  
  printStatistics( );

  convertFrames2SampleBlock( );

  return data_;
}

//---------------------------------------------------------------------------------------
    
void Plux::asyncAcquisitionThread( )
{
  #ifdef DEBUG
    cout << "Plux: asyncAcquisitionThread" << endl;
  #endif

  try {
    while( !async_acquisition_thread_.interruption_requested() )
    {
      async_buffer_.insert_overwriting( getNextFrame( ) );
      //async_buffer_.insert_throwing( getNextFrame( ) );
      //async_buffer_.insert_blocking( getNextFrame( ) );
    }
  }
  catch( std::exception &e )
  {
    cout << "Plux::asyncAcquisitionThread( ): caught exception:" << endl;
    cout << e.what( ) << endl;
    throw e;
  }
}

//---------------------------------------------------------------------------------------

SampleBlock<double> Plux::getAsyncData()
{
  #ifdef DEBUG
    cout << "Plux: getAsyncData" << endl;
  #endif

  for( int i=0; i<blocks_; i++ )
  {
    frametype frame;
    if( async_buffer_.getNext_substituting( &frame ) )
    {
      statistics_.frames_repeated_++;
    }

    // record statistics of how long samples remained in the buffer
    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - last_frame_.time;
    statistics_.time_statistics_.update( diff.total_milliseconds() );

    frames_[i] = frame.frame;
  }

  num_frames_total_ += blocks_;
  
  printStatistics( );

  convertFrames2SampleBlock( );

  return data_;
}

//-----------------------------------------------------------------------------

void Plux::startAsyncAquisition( )
{
  async_buffer_.resize( buffersize_ );

  // Start the acquisition thread
  async_acquisition_thread_ = thread( &Plux::asyncAcquisitionThread, this );
  #ifdef WIN32
   SetPriorityClass(async_acquisition_thread_.native_handle(),  HIGH_PRIORITY_CLASS);
   SetThreadPriority(async_acquisition_thread_.native_handle(), THREAD_PRIORITY_HIGHEST );
  #endif

  // Wait until the thread starts producing data
  async_buffer_.blockWhileEmpty( );
  cout << "In Buffer: " << async_buffer_.getNumAvail( ) << endl;
}

//-----------------------------------------------------------------------------

void Plux::stopAsyncAquisition( bool blocking )
{
  async_acquisition_thread_.interrupt( );
  if( blocking )
    async_acquisition_thread_.join( );
}

//-----------------------------------------------------------------------------

void Plux::printStatistics( )
{ 
  if( statistics_.statistics_interval_ > 0 )
  {
    double runtime = num_frames_total_ / fs_;
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    if( (now - statistics_.last_printed_).total_milliseconds() > 1000*statistics_.statistics_interval_ )
    {
      statistics_.last_printed_ = now;
      if( isMaster() )
      {
        cout << endl;
        cout << boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ) << endl;
        cout << " Master device: " << devinfo_ << " (" << devstr_ << ")" << endl;
        cout << "  Frames Lost : " << statistics_.frames_lost_ << "  (" << statistics_.frames_lost_/runtime << " per sec)" << endl;
        cout << "=============================" << endl;
      }
      else
      {
        cout << endl;
        cout << boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ) << endl;
        cout << " Slave device: " << devinfo_ << " (" << devstr_ << ")" << endl;
        cout << "  Frames Lost    : " << statistics_.frames_lost_ << "  (" << statistics_.frames_lost_/runtime << " per sec)" << endl;
        cout << "  Frames inserted: " << statistics_.frames_repeated_ << "  (" << statistics_.frames_repeated_/runtime << " per sec)" << endl;
        cout << "  Frames dropped : " << statistics_.frames_dropped_ << "  (" << statistics_.frames_dropped_/runtime << " per sec)" << endl;
        cout << "  Unread Frames  : " << async_buffer_.getNumAvail( ) << endl;
        cout << "       Frame delay (milliseconds) " << endl;
        cout << "           mean: " << statistics_.time_statistics_.get_mean( ) << endl;
        cout << "  adaptive mean: " << statistics_.time_statistics_.get_adaptive_mean() << endl;
        cout << "  adaptive  std: " << sqrt(statistics_.time_statistics_.get_adaptive_var()) << endl;
        cout << "       Sampling rate (milliseconds) " << endl;
        cout << "           mean: " << 1e6 / statistics_.rate_statistics_.get_mean( ) << endl;
        cout << "  adaptive mean: " << 1e6 / statistics_.rate_statistics_.get_adaptive_mean() << endl;
        cout << "  adaptive  std: " << 1e6 / sqrt(statistics_.rate_statistics_.get_adaptive_var()) << endl;
        cout << "=============================" << endl;
      }
    }
  }
}

//-----------------------------------------------------------------------------

void Plux::run()  
{
  #ifdef DEBUG
    cout << "Plux: run" << endl;
  #endif

  // Number of bits per sample to record. 8 or 12
  // FIXME ... hard coded value!
  BYTE nbits = 12;

  // Plux requires integer sampling rates
  int fs = boost::numeric_cast<int>( fs_ );
  if( boost::numeric_cast<double>( fs ) != fs_ )
    throw std::invalid_argument( "Plux::run() - Invalid Sampling Rate." );

  // Create channel mask.
  // That is a bit mask which tells the Plux Device what channels to record.
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

  // set initial conditions
  seq_expected.setInvalid( );
  statistics_.reset( );
  num_frames_total_ = 0;
  frame_pending_ = false;

  try {
    device_->BeginAcq( fs, chmask, nbits );
  } catch( BP::Err &err ) { rethrowPluxException( "Plux::run()", err, true ); }

  if( !isMaster() )
  {
    startAsyncAquisition( );
  }

  cout << " * " << devinfo_ << " (" << devstr_ << ") sucessfully started." << endl;
}

//-----------------------------------------------------------------------------

void Plux::stop() 
{
  #ifdef DEBUG
    cout << "Plux: stop" << endl;
  #endif

  cout << "Stopping " << devinfo_ << " (" << devstr_ << ") .";

  if( !isMaster() )
    stopAsyncAquisition( );

  cout << ".";

  try {
    device_->EndAcq();
  } catch( BP::Err &err ) { rethrowPluxException( "Plux::stop()", err, true ); }

  cout << ".";  

  cout << " OK" << endl;
}

//-----------------------------------------------------------------------------

void Plux::convertFrames2SampleBlock( )
{

  static boost::posix_time::ptime starttime = boost::posix_time::microsec_clock::local_time();

  data_.reset( );

  samples_.resize( data_.getNrOfChannels( ) );

  vector<BP::Device::Frame>::const_iterator frame = frames_.begin( );
  for( ; frame!=frames_.end(); frame++ )
  {
    int scount = 0;
    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::const_iterator channel = channel_info_.begin( );
    for( int i=0; channel != channel_info_.end(); channel++, i++ )
      if( channel->first < 9 )
        samples_[scount++] = frame->an_in[i];
      else switch( channel->first )
      {
        //treat special case channels
        case 9: samples_[scount++] = frame->dig_in; break;
        case 12: samples_[scount++] = async_buffer_.getNumAvail( ); break;
        case 11: samples_[scount++] = statistics_.time_statistics_.get_adaptive_mean( ); break;
        case 10: samples_[scount++] = (boost::posix_time::microsec_clock::local_time() - starttime).total_microseconds() / (1e6); break;
        default: throw std::exception( "Unsopported channel number." );
      }

    data_.appendBlock( samples_, 1 );
  }
}

//-----------------------------------------------------------------------------
    
std::string Plux::findDevice( )
{
  #ifdef DEBUG
    cout << "Plux: findDevice" << endl;
  #endif

#ifdef DISABLE_PLUX_COMPILE
    return "";
#else
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
#endif
}

//-----------------------------------------------------------------------------

void Plux::rethrowPluxException( std::string where, BP::Err &err, bool do_throw )
{
#ifndef DISABLE_PLUX_COMPILE
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
    
    cout << " **** (" << devstr_ << ") PLUX Exception in " << where << endl << "==============================================" << endl << message << endl << endl;
    if( do_throw )
    {
      throw(std::runtime_error( message ));
    }
#endif
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
    //frame.an_in[i] = a.frame.an_in[i] + boost::numeric_cast<int>( (b.frame.an_in[i] - a.frame.an_in[i]) * factor );
    frame.an_in[i] = a.frame.an_in[i];

  if( factor < 0.5 )
    frame.dig_in = a.frame.dig_in;
  else
    frame.dig_in = b.frame.dig_in;

  frame.seq = seq.cast<BYTE>();
}

} // Namespace tobiss
