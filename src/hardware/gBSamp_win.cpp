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

#include <Windows.h>
#include <boost/thread/locks.hpp>

#include "hardware/gBSamp_win.h"
#include "hardware/nidaqmx_wrapper.h"

namespace tobiss
{

using namespace nidaqmx;

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;


using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;

static const double MAX_DAQ_RANGE_VOLTS = 10.0;
static const double MIN_DAQ_RANGE_VOLTS = -10.0;

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<gBSamp> gBSamp::factory_registrator_ ("gbsamp", "g.bsamp");

//-----------------------------------------------------------------------------

gBSamp::gBSamp(ticpp::Iterator<ticpp::Element> hw)
  : current_block_(0), expected_values_(0),
    task_handle_(0), read_samples_(0), received_samples_(0), max_timeout_(0)
{
  #ifdef DEBUG
    cout << "gBSamp: Constructor" << endl;
  #endif

  nidaqmx_modes_map_.insert(std::make_pair(RSE,  DAQmx_Val_RSE) );
  nidaqmx_modes_map_.insert(std::make_pair(NRSE, DAQmx_Val_NRSE) );
  nidaqmx_modes_map_.insert(std::make_pair(diff, DAQmx_Val_Diff) );

  setHardware(hw);

  expected_values_ = nr_ch_ * blocks_;

  data_.init(blocks_, nr_ch_, channel_types_);
  samples_.resize(expected_values_, 0);
  initCard();

  max_timeout_ = 4*blocks_/fs_;
}

//-----------------------------------------------------------------------------

gBSamp::~gBSamp()
{
  #ifdef DEBUG
    cout << "gBSamp: Destructor" << endl;
  #endif

  stop();

}

//-----------------------------------------------------------------------------

void gBSamp::run()
{
  #ifdef DEBUG
    cout << "gBSamp: run" << endl;
  #endif

  running_ = 1;

  readFromDAQCard();
}

//-----------------------------------------------------------------------------

void gBSamp::stop()
{
  #ifdef DEBUG
    cout << "gBSamp: stop" << endl;
  #endif

  if(!running_)
    return;

  running_ = 0;

  if( task_handle_!=0 )
  {
    checkNIDAQmxError( nidaqmx_.stopTask(task_handle_), false);
    checkNIDAQmxError( nidaqmx_.clearTask(task_handle_), false);
  }
  task_handle_ = 0;
}

//-----------------------------------------------------------------------------

void gBSamp::readFromDAQCard()
{
  checkNIDAQmxError( nidaqmx_.startTask(task_handle_) );
}

//-----------------------------------------------------------------------------

SampleBlock<double> gBSamp::getSyncData()
{
  #ifdef DEBUG
    cout << "gBSamp: getSyncData" << endl;
  #endif

  if(!running_)
  {
    std::cerr << "gBSamp: getSyncData -- Not running!" << endl;
    return(data_);
  }

  boost::shared_lock<boost::shared_mutex> lock(rw_);
  received_samples_ = 0;
  while(received_samples_ < blocks_)
  {
    checkNIDAQmxError( nidaqmx_.readAnalogF64(task_handle_, blocks_ , max_timeout_,
                                            DAQmx_Val_GroupByChannel, &samples_[received_samples_],
                                            samples_.size()-received_samples_, &read_samples_, 0) );
    received_samples_ += read_samples_;
  }

  for(unsigned int m = 0; m < channel_info_.size() ; m++)
    for(unsigned int n = 0; n < blocks_; n++)
    {
      samples_[ (blocks_*m) +n] *= scaling_factors_[m];
    }

  data_.setSamples(samples_);
  lock.unlock();

  return(data_);
}

//-----------------------------------------------------------------------------

SampleBlock<double> gBSamp::getAsyncData()
{
  #ifdef DEBUG
    cout << "gBSamp: getAsyncData" << endl;
  #endif

  throw(std::runtime_error("gBSamp::getAsyncData -- Async data acquisition not available for g.BSamp yet!"));
}

//-----------------------------------------------------------------------------

void gBSamp::initCard()
{
  std::string channel_list;

  for(map<uint16_t, std::pair<string, uint32_t> >::iterator it = channel_info_.begin();
      it != channel_info_.end(); it++)
  {
    channel_list += device_id_ + "/ai";
    channel_list += boost::lexical_cast<std::string>( it->first - 1 );
    channel_list += ",";
  }
  channel_list.erase(channel_list.size()-1);

  checkNIDAQmxError( nidaqmx_.createTask("",&task_handle_) );

  nidaqmx::int32 mode_id = getDAQmxModeID(daq_mode_);

  checkNIDAQmxError( nidaqmx_.createAIVoltageChan(task_handle_, channel_list.c_str(), "",
                                              mode_id, MIN_DAQ_RANGE_VOLTS, MAX_DAQ_RANGE_VOLTS,
                                                  DAQmx_Val_Volts, 0));


  checkNIDAQmxError( nidaqmx_.cfgSampClkTiming(task_handle_, 0, fs_, DAQmx_Val_Rising,
                                               DAQmx_Val_HWTimedSinglePoint, blocks_));

  checkNIDAQmxError( nidaqmx_.cfgInputBuffer(task_handle_, 0) );
}

//-----------------------------------------------------------------------------


void gBSamp::checkNIDAQmxError(nidaqmx::int32 error_code, bool throw_on_failure)
{
  if(error_code)
  {
    char error_string[1024];
    nidaqmx_.getErrorString(error_code, error_string, 1024);
    std::string error_msg ("  --  An error occured during NIDAQmx operations -- Code: ");
    error_msg += boost::lexical_cast<std::string>(error_code);
    error_msg += " ... ";
    error_msg += error_string;
    std::cerr << error_msg << std::endl;
    if(throw_on_failure)
      throw std::runtime_error(error_msg);
  }
}

//-----------------------------------------------------------------------------

nidaqmx::int32 gBSamp::getDAQmxModeID(daq_mode_type val)
{
  std::map<daq_mode_type, nidaqmx::int32>::iterator it;
  it = nidaqmx_modes_map_.find(val);

  if(it == nidaqmx_modes_map_.end())
  {
    string e = "g.BSamp NIDAQmx mode \"" + boost::lexical_cast<string>(val) + "\" not found!";
    throw(std::invalid_argument(e));
  }

  return(it->second);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

