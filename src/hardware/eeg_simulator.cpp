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

#include "hardware/eeg_simulator.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace tobiss
{

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

const HWThreadBuilderTemplateRegistrator<EEGSimulator> EEGSimulator::factory_registrator_ ("eegsim", "eegsimulator");

static const float EEG_DIST_MEAN = 0;
static const float EEG_DIST_STD  = 11;  // lt. Martin Billinger, BCI Comp4, Graz Dataset A, 4.11.2010

static const unsigned int MESSAGE_BUFFER_SIZE_BYTE = 4096;

//-----------------------------------------------------------------------------

EEGSimulator::EEGSimulator(boost::asio::io_service& io,
                           ticpp::Iterator<ticpp::Element> hw)
 : ArtificialSignalSource(io, hw), twister_(static_cast<unsigned int>(std::time(0))),
   eeg_dist_(EEG_DIST_MEAN,EEG_DIST_STD), eeg_gen_(twister_,eeg_dist_),
   acceptor_(io), socket_(io), port_(0), connected_(0)
{

  setType("EEG Simulator");
  message_buffer_.resize(MESSAGE_BUFFER_SIZE_BYTE);

  setHardware(hw);


  port_ = 12874;

  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port_);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
  acceptor_.bind(endpoint);

  acceptor_.listen();
  acceptor_.async_accept(socket_, boost::bind(&EEGSimulator::acceptHandler, this,
                                              boost::asio::placeholders::error));
  init();

  using std::make_pair;

  SineCfg sine_cfg;
  sine_cfg.freq_ = 1;
  sine_cfg.amplitude_ = 100;
  sine_cfg.phase_ = 0;

  SineCfg sine_cfg2;
  sine_cfg2.freq_ = 4;
  sine_cfg2.amplitude_ = 50;
  sine_cfg2.phase_ = 0;

  EEGCfg eeg_cfg;
  eeg_cfg.amplitude_ = 3;
  eeg_cfg.offset_ = 2;

  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // modify sine- or eeg config maps
  sine_configs_.insert(make_pair(0, sine_cfg));
  sine_configs_.insert(make_pair(0, sine_cfg2));
  eeg_config_.insert(  make_pair(1, eeg_cfg) );

  lock.unlock();

  cout << " * EEGSimulator sucessfully initialized" << endl;
  cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_  << ", blocksize: " << blocks_  << endl;
}

//-----------------------------------------------------------------------------

EEGSimulator::~EEGSimulator()
{

}

//-----------------------------------------------------------------------------

void EEGSimulator::generateSignal()
{
  #ifdef DEBUG
    cout << "EEGSimulator: generateSignal" << endl;
  #endif

  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // create EEG
  std::map<boost::uint16_t, EEGCfg>::iterator eeg_it;
  for(boost::uint16_t n = 0; n < nr_ch_ ; n++)
  {
    if( (eeg_it = eeg_config_.find(n)) != eeg_config_.end() )
      samples_[n] = eeg_gen_() * eeg_it->second.amplitude_ + eeg_it->second.offset_;
    else
      samples_[n] = eeg_gen_();
  }

  // add sine waves
  std::multimap<boost::uint16_t, SineCfg>::iterator sine_it;

  for(boost::uint16_t n = 0; n < nr_ch_ ; n++)
  {
    for (sine_it = sine_configs_.equal_range(n).first;
         sine_it != sine_configs_.equal_range(n).second;
         ++sine_it)
    {
      samples_[n] += sine_it->second.amplitude_ *
                     sin(step_ * 2 * M_PI * sine_it->second.freq_ + sine_it->second.phase_);
    }
  }

  (step_ < 1-cycle_dur_ ? step_ += cycle_dur_ : step_ = 0);
  t_->expires_at(t_->expires_at() + td_);

  if(blocks_  == 1)
  {

    boost::unique_lock<boost::mutex> syn(sync_mut_);
    samples_available_ = true;
    data_.setSamples(samples_);
    lock.unlock();
    cond_.notify_all();
    if(isMaster() && acquiring_)
    {
      cond_.wait(sync_mut_);
      // if( !cond_.timed_wait(sync_mut_, td_))
      //   cerr << "Warning: New data was not fetched fast enough!" << endl;
      //   throw std::runtime_error("SineGenerator::genSine() -- Timeout; New data was not fetched fast enough!");
    }
    syn.unlock();
  }
  else
  {
    buffer_.appendBlock(samples_);
    current_block_++;

    if(current_block_ == blocks_ )
    {
      boost::unique_lock<boost::mutex> syn(sync_mut_);
      samples_available_ = true;
      data_ = buffer_;
      lock.unlock();
      cond_.notify_all();
      buffer_.reset();
      current_block_ = 0;
      if(isMaster() && acquiring_)
      {
        cond_.wait(sync_mut_);
        // if( !cond_.timed_wait(sync_mut_, td_))
        //   cerr << "Warning: New data was not fetched fast enough!" << endl;
        //   throw std::runtime_error("SineGenerator::genSine() -- Timeout; New data was not fetched fast enough!");
      }
      syn.unlock();
    }
  }

  if(running_)
    t_->async_wait(boost::bind(&EEGSimulator::generateSignal, this));
}

//-----------------------------------------------------------------------------

void EEGSimulator::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "EEGSimulator: setHardware" << endl;
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
      string ex_str(type_ + " -- ");
        ex_str += "Multiple channel_settings found!";
        throw(std::invalid_argument(ex_str));
      }
      setChannelSettings(cs);
  }
}

//-----------------------------------------------------------------------------

void EEGSimulator::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "EEGSimulator: setDeviceSettings" << endl;
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

void EEGSimulator::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "EEGSimulator: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//-----------------------------------------------------------------------------

void EEGSimulator::handleAsyncRead(const boost::system::error_code& ec,
                                     std::size_t bytes_transferred )
{
  std::cout << ec.message() << std::endl;
  if(ec)
    throw(std::runtime_error("EEGSimulator::handleAsyncRead() -- \
                             Error handling async read -- bytes transferred: " + bytes_transferred));

  // check if message is complete

  using std::make_pair;

  SineCfg sine_cfg;
  sine_cfg.freq_ = 5;
  sine_cfg.amplitude_ = 2;
  sine_cfg.phase_ = 180;

  SineCfg sine_cfg2;
  sine_cfg.freq_ = 10;
  sine_cfg.amplitude_ = 4;
  sine_cfg.phase_ = 180;

  EEGCfg eeg_cfg;
  eeg_cfg.amplitude_ = 3;
  eeg_cfg.offset_ = 2;

  boost::unique_lock<boost::shared_mutex> lock(rw_);

  // modify sine- or eeg config maps
  sine_configs_.insert(make_pair(0, sine_cfg));
  sine_configs_.insert(make_pair(0, sine_cfg2));
  eeg_config_.insert(  make_pair(0, eeg_cfg) );

  lock.unlock();
}

//-----------------------------------------------------------------------------

void EEGSimulator::acceptHandler(const boost::system::error_code& error)
{

  if (!error)
  {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_buffer_),
                            boost::bind(&EEGSimulator::handleAsyncRead,
                                        this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred)
                            );
  }

  acceptor_.async_accept(socket_, boost::bind(&EEGSimulator::acceptHandler, this,
                                              boost::asio::placeholders::error));
}


//-----------------------------------------------------------------------------


}  // tobiss
