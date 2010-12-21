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

/**
* @file artificial_signal_source.cpp
**/

#include "hardware/artificial_signal_source.h"

#include <math.h>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

namespace tobiss
{
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

using boost::lexical_cast;
using boost::bad_lexical_cast;

using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------

ArtificialSignalSource::ArtificialSignalSource(boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw)
: acquiring_(0), io_(io), current_block_(0), td_(0)
{
  #ifdef DEBUG
    cout << "ArtificialSignalSource: Constructor" << endl;
  #endif
}

//-----------------------------------------------------------------------------

ArtificialSignalSource::~ArtificialSignalSource()
{
  delete t_;
}

//-----------------------------------------------------------------------------

void ArtificialSignalSource::init()
{

  step_ = 1/static_cast<float>(fs_);
  cycle_dur_ = 1/static_cast<float>(fs_);
  boost::posix_time::microseconds period(1000000/fs_);
  td_ += period;

  buffer_.init(blocks_ , nr_ch_ , channel_types_);
  data_.init(blocks_ , nr_ch_ , channel_types_);

  samples_.resize(nr_ch_ ,0);
  t_ = new boost::asio::deadline_timer(io_, td_);

//  cout << " * ArtificialSignalSource sucessfully initialized" << endl;
//  cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_  << ", blocksize: " << blocks_  << endl;

}

//-----------------------------------------------------------------------------

void ArtificialSignalSource::run()
{
  #ifdef DEBUG
    cout << "ArtificialSignalSource: run" << endl;
  #endif

  cout << " * " << type_ << " sucessfully started" << endl;
  running_ = 1;
  generateSignal();
}


//-----------------------------------------------------------------------------

void ArtificialSignalSource::stop()
{
  #ifdef DEBUG
    cout << "ArtificialSignalSource: stop" << endl;
  #endif

  cout << " * " << type_ << " sucessfully stopped" << endl;
  running_ = 0;
  cond_.notify_all();
}

//-----------------------------------------------------------------------------

SampleBlock<double> ArtificialSignalSource::getSyncData()
{
  #ifdef DEBUG
    cout << "ArtificialSignalSource: getSyncData" << endl;
  #endif

  if(!acquiring_)
    acquiring_ = 1;

  boost::unique_lock<boost::mutex> syn(sync_mut_);
  while(!samples_available_ && running_)
    cond_.wait(syn);
  boost::shared_lock<boost::shared_mutex> lock(rw_);
  samples_available_ = false;
  lock.unlock();
  cond_.notify_all();
  syn.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> ArtificialSignalSource::getAsyncData()
{
  #ifdef DEBUG
    cout << "ArtificialSignalSource: getAsyncData" << endl;
  #endif
  boost::shared_lock<boost::shared_mutex> lock(rw_);
  samples_available_ = false;
  lock.unlock();
  return(data_);
}

//---------------------------------------------------------------------------------------

} // Namespace tobiss
