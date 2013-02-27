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
* @file imus.cpp
**/

#include "hardware/imus.h"

#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <iostream>

#import "D:\ABC\IMU-integration\SDK_IMU\SDK_imu_x86.tlb"

namespace tobiss
{

using boost::thread;
using boost::lexical_cast;

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

using namespace SDK_MCS_Studio2;

//-----------------------------------------------------------------------------

const string IMUs::hw_buffersize_("buffersize");

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<IMUs> IMUs::FACTORY_REGISTRATOR_( "imu" ); //, "blablaetc" );

//-----------------------------------------------------------------------------

struct IMUs::ImplDetails
{
	ImplDetails( )
	{
    HRESULT hr = CoInitialize(NULL);
		imu = IcMCSImuPtr(__uuidof(cMCSImu));

    // Detect Port
    FUNCTION_RESULT port_com = imu->AutoDetectPort();

	  if (port_com == SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK){
		  std::cout << ">> Port detected for IMU device.\n" << std::endl;
	  }else{
		  CoUninitialize();
      throw std::runtime_error( "Could not detect IMU Port." );
	  }

    // Connect IMU
    FUNCTION_RESULT result = imu->Connect();
	  if (result == SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK){
		  std::cout << ">> IMU connected.\n" << std::endl; 
	  }else {
		  CoUninitialize();
      throw std::runtime_error( "Could not connect IMU." );
	  }

    // HUB:
    //  sensors.resize( hub->getNumSensors() );
    sensors.resize( 1 );

    last_num_frames = 0;
	}

  ~ImplDetails( )
  {

    // Stop capture if it's still running.
    CAPTURE_STEP cs = imu->getCaptureStep( );
    if( cs != CAPTURE_STEP_Stoped )
    {
      FUNCTION_RESULT result = imu->StopCapture( );
      if( result != SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK )
      {
        std::cout << "Could not stop capturing from IMU." << endl;
      }
    }


    // Disconnect IMU
    if (imu->IsConnect())
    {
      imu->Disconnect();
	    if (imu->IsConnect()){
		    //it never happens, unless hub->Disconnect() fails
        std::cout << "Error while disconnecting IMU. Still connected." << endl;
	    }else{
		    std::cout << ">> IMU was Disconnected.\n" << std::endl;
	    }
    }

		CoUninitialize();
  }
  
  long last_num_frames;

	IcMCSImuPtr imu;

  std::vector<IcMCSSensorPtr> sensors;

  IORIENTATION_FRAMEPtr orientation_frame;
  IPHYSICAL_FRAMEPtr physical_frame;
  //IDIGITAL_FRAMEPtr digital_frame;
};

//-----------------------------------------------------------------------------

IMUs::IMUs(ticpp::Iterator<ticpp::Element> hw)
  : HWThread(), async_buffer_( 0 )
{
  #ifdef DEBUG
    cout << "IMUs: Constructor" << endl;
  #endif

  setHardware(hw);

  std::map<std::string, std::string>::const_iterator it;


  impl = 0;
  try {
    impl = new ImplDetails( );

    /*// Detect Port
    FUNCTION_RESULT port_com = impl->imu->AutoDetectPort();

	  if (port_com == SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK){
		  std::cout << ">> Port detected for IMU device.\n" << std::endl;
	  }else{
      throw std::runtime_error( "Could not detect IMU Port. (Is the device plugged in?)" );
	  }

    // Connect IMU
    FUNCTION_RESULT result = impl->imu->Connect();
	  if (result == SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK){
		  std::cout << ">> IMU connected.\n" << std::endl; 
	  }else {	
      throw std::runtime_error( "Could not connect IMU." );
	  }*/
  }
  catch(...)
  {
     throw;
  }

  #ifdef DEBUG
    cout << "IMUs: Device successfully initialized." << endl;
  #endif

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

  //IMU_INFORMATION info = IMU_INFORMATIONPtr(__uuidof(IMU_INFORMATION));
  //FUNCTION_RESULT r = impl->imu->getIMUInformation( info );

  devstr_ = "IMU";
  devinfo_ = "(Device information not yet available in the SDK)";

  cout << endl;
  cout << " IMU Device: ";
  cout << devstr_ << " - " << devinfo_ << endl;


  #ifdef DEBUG
    cout << "IMUs: Leaving Constructor." << endl;
  #endif
}

//-----------------------------------------------------------------------------

IMUs::~IMUs( )
{
  #ifdef DEBUG
    cout << "IMUs: Destructor" << endl;
  #endif

/*  if( !isMaster() )
    stopAsyncAquisition( true );*/

  if( impl )
    delete impl;
}

//-----------------------------------------------------------------------------

void IMUs::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "IMUs: setHardware" << endl;
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

void IMUs::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "IMUs: setDeviceSettings" << endl;
  #endif
  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_fs_,true));
  setSamplingRate(elem);

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);

  buffersize_ = 10;  // FIXME ... hard coded default
  elem = father->FirstChildElement(hw_buffersize_,false);
  if(elem != elem.end())
    buffersize_ = lexical_cast<DataBuffer<frametype>::size_type>(elem->GetText(true));
}

//---------------------------------------------------------------------------------------

void IMUs::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father )
{
  #ifdef DEBUG
    cout << "IMUs: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

void IMUs::waitForNewFrame( )
{
  long num_frames;

  do
  {
    boost::this_thread::sleep(boost::posix_time::microseconds(1000)); /// @todo don't hardcode sleep time
    num_frames = impl->sensors[0]->getCountPhysicalFrames();          // get new number of frames
  } while( num_frames <= impl->last_num_frames );                     // loop until number of frames changed

  impl->last_num_frames = num_frames;
}

void IMUs::getDataFrame( int bidx )
{
  //frametype frame;

  boost::posix_time::ptime now =  boost::posix_time::microsec_clock::local_time();

  long num_frames = impl->sensors[0]->getCountPhysicalFrames();

  static long last_num = 0; // just for debugging. remove later.

  if( num_frames == 0 || num_frames == last_num )
  {
    int scount = 0;
    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::const_iterator channel = channel_info_.begin( );
    for( int i=0; channel != channel_info_.end(); channel++, i++ )
    {
      samples_[scount++] = 0;
    }
  }
  else
  {
    const long ch_per_sensor = 19;
    
    int scount = 0;

    //HUB:
    // long num_sensors = impl->hub->getNumSensors();
    long num_sensors = 1;
    for( long sensor=0; sensor<num_sensors; sensor++ )
    {
      impl->orientation_frame = impl->sensors[sensor]->getOrientationFrame( num_frames - 1 );
      impl->physical_frame = impl->sensors[sensor]->getPhysicalFrame( num_frames - 1 );
  
      IORIENTATION_AXISPtr orientation_axis_x = impl->orientation_frame->GetsX( );
      IORIENTATION_AXISPtr orientation_axis_y = impl->orientation_frame->GetsY( );
      IORIENTATION_AXISPtr orientation_axis_z = impl->orientation_frame->GetsZ( );
  
      IPHYSICAL_AXISPtr acceleration_axis = impl->physical_frame->GetAcce( );
      IPHYSICAL_AXISPtr gyro_axis = impl->physical_frame->GetGyro( );
      IPHYSICAL_AXISPtr magnetic_axis = impl->physical_frame->GetMagn( );

      float temperature = impl->physical_frame->GetTemp( );

      //cout << oa->GetX() << ", " << oa->GetY() << ", " << oa->GetZ() << endl;
  

      std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::const_iterator channel = channel_info_.begin( );
      for( int i=0; channel != channel_info_.end(); channel++, i++ )
      {
        bool add_to_block = true;
        double value;
        switch( channel->first - sensor * ch_per_sensor )
        {
        case 1 : value = orientation_axis_x->GetX( ); break;
        case 2 : value = orientation_axis_x->GetY( ); break;
        case 3 : value = orientation_axis_x->GetZ( ); break;
        case 4 : value = orientation_axis_y->GetX( ); break;
        case 5 : value = orientation_axis_y->GetY( ); break;
        case 6 : value = orientation_axis_y->GetZ( ); break;
        case 7 : value = orientation_axis_z->GetX( ); break;
        case 8 : value = orientation_axis_z->GetY( ); break;
        case 9 : value = orientation_axis_z->GetZ( ); break;
      
        case 10 : value = acceleration_axis->GetX( ); break;
        case 11 : value = acceleration_axis->GetY( ); break;
        case 12 : value = acceleration_axis->GetZ( ); break;
      
        case 13 : value = gyro_axis->GetX( ); break;
        case 14 : value = gyro_axis->GetY( ); break;
        case 15 : value = gyro_axis->GetZ( ); break;
      
        case 16 : value = magnetic_axis->GetX( ); break;
        case 17 : value = magnetic_axis->GetY( ); break;
        case 18 : value = magnetic_axis->GetZ( ); break;

        case 19 : value = temperature; break;

        default: add_to_block = false;  break;
        }
        if( add_to_block)
        {
          samples_[scount++] = value;
        }
      }
    }
  }

  data_.appendBlock( samples_, 1 );

  last_num = num_frames;

  statistics_.rate_statistics_.update( (now - last_time_).total_microseconds() );
  last_time_ = now;

}

//---------------------------------------------------------------------------------------

SampleBlock<double> IMUs::getSyncData()
{
  #ifdef DEBUG
    cout << "IMUs: getSyncData" << endl;
  #endif
  
  data_.reset( );
  samples_.resize( data_.getNrOfChannels( ) );

  for( int i=0; i<blocks_; i++ )
  {
    waitForNewFrame( );
    getDataFrame( i );
  }
  
  /*num_frames_total_ += blocks_;
  
  printStatistics( );

  convertFrames2SampleBlock( );*/

  return data_;
}

//---------------------------------------------------------------------------------------
    
void IMUs::asyncAcquisitionThread( )
{
  /*#ifdef DEBUG
    cout << "IMUs: asyncAcquisitionThread" << endl;
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
    cout << "IMUs::asyncAcquisitionThread( ): caught exception:" << endl;
    cout << e.what( ) << endl;
    throw e;
  }*/
  cout << "IMUs::asyncAcquisitionThread() - not implemented!" << endl;
}

//---------------------------------------------------------------------------------------

SampleBlock<double> IMUs::getAsyncData()
{
  #ifdef DEBUG
    cout << "IMUs: getAsyncData" << endl;
  #endif
  
  data_.reset( );
  samples_.resize( data_.getNrOfChannels( ) );

  for( int i=0; i<blocks_; i++ )
  {
    getDataFrame( i );
  }

  return data_;
}

//-----------------------------------------------------------------------------

/*void IMUs::startAsyncAquisition( )
{
  async_buffer_.resize( buffersize_ );

  // Start the acquisition thread
  async_acquisition_thread_ = thread( &IMUs::asyncAcquisitionThread, this );
  #ifdef WIN32
   SetPriorityClass(async_acquisition_thread_.native_handle(),  HIGH_PRIORITY_CLASS);
   SetThreadPriority(async_acquisition_thread_.native_handle(), THREAD_PRIORITY_HIGHEST );
  #endif

  // Wait until the thread starts producing data
  async_buffer_.blockWhileEmpty( );
  cout << "In Buffer: " << async_buffer_.getNumAvail( ) << endl;
  
  cout << "IMUs::startAsyncAquisition() - not implemented!" << endl;
}*/

//-----------------------------------------------------------------------------

/*void IMUs::stopAsyncAquisition( bool blocking )
{
  async_acquisition_thread_.interrupt( );
  if( blocking )
    async_acquisition_thread_.join( );
  
  cout << "IMUs::stopAsyncAquisition() - not implemented!" << endl;
}*/

//-----------------------------------------------------------------------------

void IMUs::printStatistics( )
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

void IMUs::run()  
{
  #ifdef DEBUG
    cout << "IMUs: run" << endl;
  #endif

  // Number of bits per sample to record. 8 or 12
  // FIXME ... hard coded value!
  BYTE nbits = 12;

  // IMU requires integer sampling rates
  int fs = boost::numeric_cast<int>( fs_ );
  if( boost::numeric_cast<double>( fs ) != fs_ )
    throw std::invalid_argument( "IMUs::run() - Invalid Sampling Rate." );

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

  FUNCTION_RESULT result = impl->imu->setFrequency( fs );

  if (result != SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK)
  {
    throw std::runtime_error( "Could not set sampling rate for IMU." );
  }
  
  //impl->imu->setCaptureTime(0,0,2); // h, m, s

  // Start capturing
  result =  impl->imu->StartCapture( SDK_MCS_Studio2::CAPTURE_INFORMATION_OrientationPhysicalData, false, true, false );
  if( result != SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK )
  {
    throw std::runtime_error( "Could not start capturing from IMU." );
  }

  // Get sensor(s) for later access.
  // HUB:
  //for( int i=0; i<impl->hub->getNumSensors();  i++ )
  //{
  //  impl->sensors[i] = impl->hub->getSensor( i );
  //}
  impl->sensors[0] = impl->imu->getSensor();

  /*if( !isMaster() )
  {
    startAsyncAquisition( );
  }*/

  cout << " * " << devinfo_ << " (" << devstr_ << ") sucessfully started." << endl;
}

//-----------------------------------------------------------------------------

void IMUs::stop() 
{
  #ifdef DEBUG
    cout << "IMUs: stop" << endl;
  #endif

  cout << "Stopping " << devinfo_ << " (" << devstr_ << ") .";

/*  if( !isMaster() )
    stopAsyncAquisition( );*/

  FUNCTION_RESULT result = impl->imu->StopCapture( );

  if( result != SDK_MCS_Studio2::FUNCTION_RESULT_FunctionOK )
  {
    throw std::runtime_error( "Could not stop capturing from IMU." );
  }

  cout << " OK" << endl;
}

//-----------------------------------------------------------------------------

void IMUs::convertFrames2SampleBlock( )
{

  cout << "IMUs::convertFrames2SampleBlock() - not implemented!" << endl;

  /*static boost::posix_time::ptime starttime = boost::posix_time::microsec_clock::local_time();

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
        default: throw std::invalid_argument( "Unsopported channel number." );
      }

    data_.appendBlock( samples_, 1 );
  }*/
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

IMUs::SequenceNumber::SequenceNumber( int s )
{
  if( s < 0 )
    s = -1;
  else while( s >= 128 )
    s -= 128;
  seq = s;
}

//-----------------------------------------------------------------------------


bool IMUs::SequenceNumber::valid( )
{
  return seq >= 0;
}

//-----------------------------------------------------------------------------

void IMUs::SequenceNumber::operator++( int )
{
  if( seq == 127 )
    seq = 0;
  else
    seq++;
}

//-----------------------------------------------------------------------------

void IMUs::SequenceNumber::operator--( int )
{
  if( seq == 0 )
    seq = 127;
  else
    seq--;
}

//-----------------------------------------------------------------------------

bool IMUs::SequenceNumber::operator==( const seqtype &s ) const
{
  return seq == s;
}

//-----------------------------------------------------------------------------

bool IMUs::SequenceNumber::operator<( const seqtype &s ) const
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

bool IMUs::SequenceNumber::operator>( const seqtype &s ) const
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


IMUs::frametype::frametype( const frametype &a, const frametype &b, const SequenceNumber &seq )
{        
/*  double factor = (seq.cast<double>()-a.frame.seq)/(b.frame.seq-a.frame.seq);

  time = a.time + boost::posix_time::microseconds( (b.time - a.time).total_microseconds() * factor );

  for( int i=0; i<8; i++ )
    //frame.an_in[i] = a.frame.an_in[i] + boost::numeric_cast<int>( (b.frame.an_in[i] - a.frame.an_in[i]) * factor );
    frame.an_in[i] = a.frame.an_in[i];

  if( factor < 0.5 )
    frame.dig_in = a.frame.dig_in;
  else
    frame.dig_in = b.frame.dig_in;

  frame.seq = seq.cast<BYTE>();*/

  throw std::runtime_error( "frame interpolation not implemented!" );
}

} // Namespace tobiss
