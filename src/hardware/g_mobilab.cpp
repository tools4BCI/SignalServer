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

#include "hardware/g_mobilab.h"
#include "hardware/hw_thread_builder.h"

#include <vector>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace tobiss
{

using std::vector;
using std::string;
using std::map;

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

using boost::uint16_t;
using boost::uint32_t;

static const unsigned int MOBILAB_MAX_NR_OF_CHANNELS    = 8;
static const double MOBILAB_DAQ_RESOLUTION_BIT    = 16;
static const unsigned int MOBILAB_DAQ_RESOLUTION_BYTE   = 2;
static const unsigned int MOBILAB_SAMPLING_RATE  = 256;  // Hz

static const float MOBILAB_EEG_SENSITIVITY =       500;  //micro volts
static const float MOBILAB_EOG_SENSITIVITY =      2000;  //micro volts
static const float MOBILAB_EMG_SENSITIVITY =      5000;  //micro volts
static const float MOBILAB_ANALOG_SENSITIVITY = 250000;  //micro volts

static const double MOBILAB_COMMON_LOWPASS  =  100;  // Hz
static const double MOBILAB_EEG_HIGHPASS    =  0.5;  // Hz
static const double MOBILAB_EOG_HIGHPASS    =  0.01;  // Hz
static const double MOBILAB_EMG_HIGHPASS    =  0.5;  // Hz

static const unsigned int MOBILAB_BAUD_RATE    = 57600;
static const unsigned int MOBILAB_CHAR_SIZE    = 8;
static const std::string  MOBILAB_FLOW_CONTROL = "none";
static const std::string  MOBILAB_STOP_BITS    = "1";
static const std::string  MOBILAB_PARITY       = "none";


const string GMobilab::hw_mobilab_serial_port_("serial_port");
const string GMobilab::hw_mobilab_type_("mobilab_type");
const string GMobilab::hw_mobilab_eeg_("eeg");
const string GMobilab::hw_mobilab_multi_("multi");

//-----------------------------------------------------------------------------

#ifndef WIN32

  #include <termios.h>

  // very very bad hack ... taken from ttylog
  // don't know if something similar has to be done for windows
  // if this code is not executed, the mobilab does not respond
  // don't know why
  // only starting ttylog before the sigserver was enough, otherwise
  // the mobilab did not respond
  // maybe some pins have to be set manually
  void DO_BAD_SERIAL_PORT_HACK(std::string dev)
  {
    unsigned int count = 2;
    while(count)
    {
      FILE *logfile;
      int fd;
      fd_set rfds;
      struct termios oldtio, newtio;

      logfile = fopen (dev.c_str(), "rb");

      fd = fileno (logfile);

      tcgetattr (fd, &oldtio);	/* save current serial port settings */
      bzero (&newtio, sizeof (newtio));	/* clear struct for new port settings */

      newtio.c_cflag = B57600 | CRTSCTS | CS8 | CLOCAL | CREAD;
      newtio.c_iflag = IGNPAR | IGNCR;
      newtio.c_oflag = 0;
      newtio.c_lflag = ICANON;

      tcflush (fd, TCIFLUSH);
      tcsetattr (fd, TCSANOW, &newtio);

      /* Clear the device */
      FD_ZERO (&rfds);
      FD_SET (fd, &rfds);

      fclose (logfile);
      tcsetattr (fd, TCSANOW, &oldtio);

      count--;
      usleep(20000);
    }
  }

#endif

const HWThreadBuilderTemplateRegistrator<GMobilab> GMobilab::factory_registrator_ ("mobilab", "mobilab+", "g.mobilab", "g.mobilab+");

//-----------------------------------------------------------------------------
GMobilab::GMobilab(boost::asio::io_service& io, XMLParser& parser,
         ticpp::Iterator<ticpp::Element> hw)
  : SerialPortBase(io), HWThread(parser), type_(EEG), async_acqu_thread_(0)
{
  setHardware(hw);

  checkNrOfChannels();
  setScalingValues();

#ifndef WIN32
  DO_BAD_SERIAL_PORT_HACK(getSerialPortName());
#endif

  open();
  setBaudRate(MOBILAB_BAUD_RATE);
  setFlowControl(MOBILAB_FLOW_CONTROL);
  setParity(MOBILAB_PARITY);
  setStopBits(MOBILAB_STOP_BITS);
  setCharacterSize(MOBILAB_CHAR_SIZE);

  raw_data_.resize(nr_ch_, 0);

  samples_.reserve(4 * nr_ch_);
  samples_.resize(nr_ch_, 0);

  if(blocks_ != 1)
    throw(std::invalid_argument("Blocksize > 1 not supported yet!") );

  std::vector<unsigned char> command;
  std::vector<unsigned char> reply(1,0);

  unsigned char channel_code = getChannelCode();

  // command to set channels ... sniffed from serial communication
  command.push_back(0x63);
  command.push_back(channel_code);
  command.push_back(0x20);

  sync_write(command);
  sync_read(reply);

  // response from mobilab if channel setting was ok -- don't know other messages
  if(reply[0] != 0x63)
    throw(std::runtime_error("GMobilab::Constructor -- Wrong hardware response from mobilab!") );

  data_.init(blocks_ , nr_ch_ , channel_types_);

  std::cout << " * g.Mobilab sucessfully initialized" << std::endl;
  std::cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_  << ", blocksize: " << blocks_  << std::endl;
}

//-----------------------------------------------------------------------------

void GMobilab::run()
{

  // command to start data transmission
  std::vector<unsigned char> command(1, 0x61);
  sync_write(command);

  if(mode_ == SLAVE)
    async_acqu_thread_ = new boost::thread( boost::bind(&GMobilab::acquireData, this) );

  running_ = true;
  std::cout << " * g.Mobilab sucessfully started" << std::endl;
}

//-----------------------------------------------------------------------------

void GMobilab::stop()
{
  running_ = false;
  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // command to stop data transmission
  std::vector<unsigned char> command(1,0x62);
  sync_write(command);

  std::cout << " * g.Mobilab sucessfully stopped!" << std::endl;
}

//-----------------------------------------------------------------------------

SampleBlock<double> GMobilab::getSyncData()
{
  if(running_)
  {
    boost::unique_lock<boost::shared_mutex> lock(rw_);

    sync_read(raw_data_);

    for(unsigned int n = 0; n < raw_data_.size(); n++)
      samples_[n] = raw_data_[n]* scaling_factors_[n];

    data_.setSamples(samples_);
  }
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> GMobilab::getAsyncData()
{
  if(running_)
  {
    boost::unique_lock<boost::shared_mutex> lock(rw_);

    data_.setSamples(samples_);

  }

  return(data_);
}

//-----------------------------------------------------------------------------

void GMobilab::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    std::cout << "GMobilab: setHardware" << std::endl;
  #endif

  checkMandatoryHardwareTags(hw);
  ticpp::Iterator<ticpp::Element> ds(hw->FirstChildElement(hw_devset_, true));

  setDeviceSettings(ds);

  ticpp::Iterator<ticpp::Element> cs(hw->FirstChildElement(hw_chset_, false));
  if (cs != cs.end())
  {
    for(ticpp::Iterator<ticpp::Element> it(cs); ++it != it.end(); )
      if(it->Value() == hw_chset_)
      {
        string ex_str;
        ex_str = "Error in "+ hardware_name_ +" - " + m_.find(hardware_name_)->second + " -- ";
        ex_str += "Multiple channel_settings found!";
        throw(ticpp::Exception(ex_str));
      }
      setChannelSettings(cs);
  }
}

//-----------------------------------------------------------------------------

void GMobilab::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "GMobilab: setDeviceSettings" << std::endl;
  #endif

  fs_ = MOBILAB_SAMPLING_RATE;

  ticpp::Iterator<ticpp::Element>elem = father->FirstChildElement(hw_mobilab_serial_port_,true);
  setPortName(elem->GetText(true));

  elem = father->FirstChildElement(hw_mobilab_type_,true);
  if(elem->GetText(true) == hw_mobilab_eeg_)
    type_ = EEG;
  else if(elem->GetText(true) == hw_mobilab_multi_)
    type_ = MULTI;
  else
    throw(std::invalid_argument(
        "GMobilab::setDeviceSettings() -- type not supported or wrong:") );

  elem = father->FirstChildElement(hw_channels_,false);
  if(elem != elem.end())
    setDeviceChannels(elem);

  elem = father->FirstChildElement(hw_blocksize_,false);
  if(elem != elem.end())
    setBlocks(elem);
}

//---------------------------------------------------------------------------------------

void GMobilab::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    std::cout << "GMobilab: setChannelSettings" << std::endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

void GMobilab::setScalingValues()
{
  // use only 50% of the sensitivity to get a correct scaling

  if(type_ == EEG)
  {
    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::iterator it;
    for(it = channel_info_.begin(); it != channel_info_.end(); it++)
    {
      scaling_factors_.push_back(
          MOBILAB_EEG_SENSITIVITY/(2.0 * pow(2.0 ,MOBILAB_DAQ_RESOLUTION_BIT) ) );
    }
  }
  if(type_ == MULTI)
  {
    double scaling = 0;
    double sensitivity = 0;
    boost::uint16_t channel = 0;

    std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::iterator it;
    for(it = channel_info_.begin(); it != channel_info_.end(); it++)
    {
      channel = (*it).first;

      std::cout << "GMobilab::setScalingValues() -- channel numbers: ";
      std::cout <<  channel << std::flush << std::endl;

      if(channel == 1 || channel == 2)
        sensitivity = MOBILAB_EEG_SENSITIVITY;
      if(channel == 3 || channel == 4)
        sensitivity = MOBILAB_EOG_SENSITIVITY;
      if(channel == 5 || channel == 6)
        sensitivity = MOBILAB_EMG_SENSITIVITY;
      if(channel == 7 || channel == 8)
        sensitivity = MOBILAB_ANALOG_SENSITIVITY;

      scaling = sensitivity/pow(2.0 ,MOBILAB_DAQ_RESOLUTION_BIT);
      scaling_factors_.push_back(scaling/2.0);

    }
  }
}

//---------------------------------------------------------------------------------------

unsigned char GMobilab::getChannelCode()
{
  // binary channel coding for the mobilab
  channel_coding_[1] = 0x80;
  channel_coding_[2] = 0x40;
  channel_coding_[3] = 0x20;
  channel_coding_[4] = 0x10;
  channel_coding_[5] = 0x08;
  channel_coding_[6] = 0x04;
  channel_coding_[7] = 0x02;
  channel_coding_[8] = 0x01;

  unsigned char code = 0;
  std::map<boost::uint16_t, std::pair<std::string, boost::uint32_t> >::iterator it;
  for(it = channel_info_.begin(); it != channel_info_.end(); it++)
    code |= channel_coding_[ (*it).first ];

  return(code);
}

//---------------------------------------------------------------------------------------

void GMobilab::checkNrOfChannels()
{
  #ifdef DEBUG
    cout << "USBamp: checkNrOfChannels" << endl;
  #endif

  if(nr_ch_ > MOBILAB_MAX_NR_OF_CHANNELS )
    throw(std::invalid_argument("Too many channels defined -- maximum nr of channels: "\
                                +boost::lexical_cast<string>(MOBILAB_MAX_NR_OF_CHANNELS)) );

  map<uint16_t, std::pair<string, uint32_t> >::iterator it(channel_info_.begin());

  for(  ; it != channel_info_.end(); it++ )
    if(it->first > MOBILAB_MAX_NR_OF_CHANNELS )
      throw(std::invalid_argument("Channel number too high -- maximum nr of channels: "\
      +boost::lexical_cast<string>(MOBILAB_MAX_NR_OF_CHANNELS)) );
}

//---------------------------------------------------------------------------------------

void GMobilab::acquireData()
{
  while(running_)
  {
    sync_read(raw_data_);

    boost::unique_lock<boost::shared_mutex> lock(rw_);
    for(unsigned int n = 0; n < raw_data_.size(); n++)
      samples_[n] = raw_data_[n]* scaling_factors_[n];
    lock.unlock();
  }
}

//---------------------------------------------------------------------------------------

} // Namespace tobiss

