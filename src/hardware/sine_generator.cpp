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

#include "hardware/sine_generator.h"

#include <cmath>

namespace tobiss
{
using boost::uint16_t;

using std::string;
using std::cout;
using std::endl;

const HWThreadBuilderTemplateRegistrator<SineGenerator> SineGenerator::factory_registrator_ ("sinegen", "sinegenerator");

//-----------------------------------------------------------------------------

SineGenerator::SineGenerator(boost::asio::io_service& io, ticpp::Iterator<ticpp::Element> hw)
 : ArtificialSignalSource(io, hw)
{
  #ifdef DEBUG
    cout << "SineGenerator: Constructor" << endl;
  #endif

  setType("Sine Generator");
  setHardware(hw);
  init();

  cout << " * SineGenerator sucessfully initialized" << endl;
  cout << "    fs: " << fs_ << "Hz, nr of channels: " << nr_ch_  << ", blocksize: " << blocks_  << endl;
}

//-----------------------------------------------------------------------------

SineGenerator::~SineGenerator()
{

}

//-----------------------------------------------------------------------------

void SineGenerator::genSine()
{
  #ifdef DEBUG
    cout << "SineGenerator: genSine" << endl;
  #endif

  for(uint16_t n = 0; n < nr_ch_ ; n++)
    samples_[n] = sin(step_ * 2 * M_PI + n/4);

  (step_ < 1-cycle_dur_ ? step_ += cycle_dur_ : step_ = 0);
  t_->expires_at(t_->expires_at() + td_);

  if(blocks_  == 1)
  {
    boost::unique_lock<boost::shared_mutex> lock(rw_);
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
      boost::unique_lock<boost::shared_mutex> lock(rw_);
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
    t_->async_wait(boost::bind(&SineGenerator::genSine, this));
}

//-----------------------------------------------------------------------------

void SineGenerator::setHardware(ticpp::Iterator<ticpp::Element>const &hw)
{
  #ifdef DEBUG
    cout << "SineGenerator: setHardware" << endl;
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

void SineGenerator::setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "SineGenerator: setDeviceSettings" << endl;
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

void SineGenerator::setChannelSettings(ticpp::Iterator<ticpp::Element>const &father)
{
  #ifdef DEBUG
    cout << "SineGenerator: setChannelSettings" << endl;
  #endif

  ticpp::Iterator<ticpp::Element> elem(father->FirstChildElement(hw_chset_sel_,false));
  if (elem != elem.end())
    setChannelSelection(elem);
}

//---------------------------------------------------------------------------------------

} // Namespace tobiss
