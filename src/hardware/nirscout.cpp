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

#include <iostream>

#include "hardware/nirscout.h"

namespace tobiss
{

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::make_pair;

using std::cout;
using std::cerr;
using std::endl;

const HWThreadBuilderTemplateRegistratorWithoutIOService<NIRScout> NIRScout::factory_registrator_ ("NIRScout");

static const int NIRSCOUT_TIMEOUT = 1000;
static const int STR_BUFFER_SIZE = 1024;
static const int CHUNK_SIZE = 1;

//-----------------------------------------------------------------------------

NIRScout::NIRScout(ticpp::Iterator<ticpp::Element> hw)
{
  versionStruct version;

	tsdk_util_getAPIVersion(&version);
	cout << "NIRScout Version: " << version.Major << "."<< version.Minor << ".";
  cout << version.Fix << "."<< version.Build << endl;

  string_buffer_ = new char[STR_BUFFER_SIZE];
  for(unsigned int n = 0; n < STR_BUFFER_SIZE; n++)
    string_buffer_[n] = 0;

	timestamps_  = new double[CHUNK_SIZE];
  for(unsigned int n = 0; n < CHUNK_SIZE; n++)
    timestamps_[n] = 0;

	timing_bytes_ = new char[CHUNK_SIZE];
  for(unsigned int n = 0; n < CHUNK_SIZE; n++)
    timing_bytes_[n] = 0;

  error_code_ = tsdk_initialize();
  //error_code_ = tsdk_connect(target_ip_.c_str(), port_, NIRSCOUT_TIMEOUT);
  error_code_ = tsdk_getChannels(&sources_, &detectors_, &wavelengths_);

  unsigned int status_flags;
  double sample_rate;
  error_code_ = tsdk_getStatus( &status_flags, &sample_rate);

  if(sample_rate)
    fs_ = sample_rate;

  // TODO:   blocks_   ==  frame_size ???

  for(unsigned int i=0; i < detectors; i++)
  {
    error_code_ = tsdk_getName(sources, i, string_buffer_, STR_BUFFER_SIZE);
    // DO SOMETHING WITH NAMES
  }

  for(unsigned int i=0; i < detectors; i++)
  {
    error_code_ = tsdk_getName(detectors, i, string_buffer_, STR_BUFFER_SIZE);
    // DO SOMETHING WITH NAMES
  }

  for(unsigned int i=0; i < detectors; i++)
  {
    error_code_ = tsdk_getName(wavelengths, i, string_buffer_, STR_BUFFER_SIZE);
    // DO SOMETHING WITH NAMES
  }
}

//-----------------------------------------------------------------------------

NIRScout::~NIRScout()
{
  if(string_buffer_)
    delete[] string_buffer_;

  if(raw_data_)
    delete[] raw_data_;

  if(timestamps_)
    delete[] timestamps_;

  if(timing_bytes_)
    delete[] timing_bytes_;
}

//-----------------------------------------------------------------------------

SampleBlock<double> NIRScout::getSyncData()
{
  error_code_ = tsdk_getNFrames(CHUNK_SIZE, NIRSCOUT_TIMEOUT, &frame_count_,
                                timestamps_, timing_bytes_, raw_data_, &buffer_size_);

  if(frame_count_)
  {
    tsdk_util_getTimeString(timestamps_[0], string_buffer_, STR_BUFFER_SIZE);

    int frames_avail = 0;
    error_code_ = tsdk_getFramesAvail(&frames_avail);
  }
  else
  {
    unsigned int status_flags;
    double sample_rate;
    error_code_ = tsdk_getStatus( &status_flags, &sample_rate);
    if(!(status_flags & FLAGS_CONNECTED))
    {
      cerr << "Connection lost!" << endl;
    }
  }

  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> NIRScout::getAsyncData()
{
  return(data_);
}

//-----------------------------------------------------------------------------

void NIRScout::run()
{
  int framesize = 0;
  error_code_ = tsdk_start(NULL, NULL, NULL, 0, 0, 0, 10, &framesize);
  
	raw_data_ = new float[framesize];
  for(int n = 0; n < framesize; n++)
    raw_data_[n] = 0;

	buffer_size_ = framesize;


}

//-----------------------------------------------------------------------------
void NIRScout::stop()
{
  error_code_ = tsdk_stop();
  error_code_ = tsdk_disconnect();
  error_code_ = tsdk_close();
}

//-----------------------------------------------------------------------------

void NIRScout::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{

}

//-----------------------------------------------------------------------------

void NIRScout::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{

}

//-----------------------------------------------------------------------------

void NIRScout::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{

}

//-----------------------------------------------------------------------------


} // tobiss

