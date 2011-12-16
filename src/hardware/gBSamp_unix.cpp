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

#include "hardware/gBSamp_unix.h"

#include "hardware/hw_thread_builder.h"
#include "tia/constants.h"

#include <comedilib.h>
#include <boost/cstdint.hpp>

// STL
#include <iostream>

namespace tobiss
{
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::pair;
using std::cerr;

//-----------------------------------------------------------------------------

const HWThreadBuilderTemplateRegistratorWithoutIOService<gBSamp> gBSamp::factory_registrator_ ("gbsamp", "g.bsamp");

#define SUBDEVICE_AI 0 //subdevice for analog input
#define PREF_RANGE  0 //range -10V - +10V
static const int buf_size = 1000;

//-----------------------------------------------------------------------------


gBSamp::gBSamp(ticpp::Iterator<ticpp::Element> hw)
: first_run_(1)
{
  #ifdef DEBUG
    cout << "gBSamp: Constructor" << endl;
  #endif

  setHardware(hw);

  expected_values_ = nr_ch_ * blocks_;

  data_.init(blocks_, nr_ch_, channel_types_);
  samples_.resize(expected_values_, 0);

  if( initCard() != 0)
    throw(std::invalid_argument("gBSamp: not initialized, please restart") );
}

//-----------------------------------------------------------------------------

gBSamp::~gBSamp()
{
  #ifdef DEBUG
    cout << "gBSamp: Destructor" << endl;
  #endif

  if(device_)
  {
    //    comedi_cancel(device_, 0);
    comedi_close(device_);
  }

  if( channel_list_ )
    delete [] channel_list_;
  channel_list_ = 0;
}

//-----------------------------------------------------------------------------

void gBSamp::run()
{
  #ifdef DEBUG
    cout << "gBSamp: run" << endl;
  #endif

  running_ = 1;

}

//-----------------------------------------------------------------------------

void gBSamp::stop()
{
  #ifdef DEBUG
    cout << "gBSamp: stop" << endl;
  #endif

  running_ = 0;

  if(device_)
  {
    //    comedi_cancel(device_, 0);
    comedi_close(device_);
  }
}

//-----------------------------------------------------------------------------

SampleBlock<double> gBSamp::getSyncData()
{
  #ifdef DEBUG
    cout << "gBSamp: getSyncData" << endl;
  #endif

  if(!running_)
  {
    cout << "Not running!" << endl;
    return(data_);
  }

  boost::shared_lock<boost::shared_mutex> lock(rw_);
  sampl_t buf[buf_size];

  comedi_get_buffer_contents(device_,0);
  int temp = expected_values_;
  int bytes_read = 0;
  int samples_read = 0;


  while(temp>0)     // isn't this a little bit dangerous!?
  {
    bytes_read = read(comedi_fileno(device_), buf+samples_read, temp*sizeof(sampl_t));
    temp -= bytes_read/sizeof(sampl_t);
    samples_read += bytes_read/sizeof(sampl_t);
    //    if((bytes_read < 0) || (bytes_read % 2 != 0)) cout << "Error: Bytes read are " << bytes_read << endl;
  }

  // what happens now, if we got more bytes than we expected?

  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  int k=0;
  for(int i=0; i < nr_ch_; it++)
  {
    range_info_ = comedi_get_range(device_, SUBDEVICE_AI, (it->first - 1), 0);
    maxdata_ = comedi_get_maxdata(device_, SUBDEVICE_AI, (it->first - 1));
    for(int j = 0; j <  blocks_; j++)
    {
     samples_[k++] = comedi_to_phys(buf[j*nr_ch_+i], range_info_, maxdata_);
    }
    i++;
  }

  for(unsigned int m = 0; m < channel_info_.size() ; m++)
    for(unsigned int n = 0; n < blocks_; n++)
    {
      samples_[ (blocks_*m) +n] *= scaling_factors_[m];
    }

  data_.setSamples(samples_);
  samples_available_ = false;
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

  return(data_);
}

//-----------------------------------------------------------------------------

int gBSamp::initCard()
{
  #ifdef DEBUG
    cout << "gBSamp: getAsyncData" << endl;
  #endif

  device_ = comedi_open(device_id_.c_str());
  if(device_==0)
  {
      cerr << "Error: " << comedi_strerror( comedi_errno() ) << endl;
      return -1;
  }

  #ifdef DEBUG
    cout << "gBSamp: device opened!" << endl;
  #endif

  comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

  map<uint16_t, pair<string, uint32_t> >::iterator it = channel_info_.begin();
  channel_list_ = new unsigned int[nr_ch_];

  for(int i=0; i < nr_ch_; it++)
  {
    int temp_channel = (it->first - 1);
    channel_list_[i] = CR_PACK(temp_channel, PREF_RANGE, AREF_GROUND);
    i++;
  }

  comedi_cmd_.subdev = 0;
  comedi_cmd_.flags = TRIG_ROUND_NEAREST | TRIG_WAKE_EOS;

  comedi_cmd_.start_src = TRIG_NOW;
  comedi_cmd_.start_arg = 0;

  comedi_cmd_.scan_begin_src = TRIG_TIMER;
  comedi_cmd_.scan_begin_arg = (1e9/fs_);

  comedi_cmd_.convert_src = TRIG_TIMER;
  comedi_cmd_.convert_arg = (1e9/(fs_*nr_ch_));

  comedi_cmd_.scan_end_src = TRIG_COUNT;
  comedi_cmd_.scan_end_arg = nr_ch_;

  comedi_cmd_.stop_src = TRIG_NONE;
  comedi_cmd_.stop_arg = 0;

  comedi_cmd_.chanlist = channel_list_;
  comedi_cmd_.chanlist_len = nr_ch_;

  int i=10;
  while(first_run_ && i>0)
  {
    error_ = comedi_command_test(device_, &comedi_cmd_);
    if (error_ >= 0)
    {
      cerr << "gBSamp: Error for 1st comedi_command_test: " << error_ << endl;
    }
    error_ = comedi_command_test(device_, &comedi_cmd_);
    if (error_ >= 0)
    {
      cerr << "gBSamp: Error for 2nd comedi_command_test: " << error_ << endl;
    }
    error_ = comedi_command_test(device_, &comedi_cmd_);
    if (error_ >= 0)
    {
      cerr << "gBSamp: Error for 3nd comedi_command_test: " << error_ << endl;
    }
    error_ = comedi_command(device_, &comedi_cmd_);
    first_run_ = 0;
    if (error_ < 0)
    {
      cerr << "gBSamp: Error for comedi_command: " << error_ << endl;
      first_run_=1;
      i--;
    }
    if(!first_run_)
      cout << "gBSamp: comedi_command successful" << endl;
  }

  return error_;
}

//-----------------------------------------------------------------------------

} // Namespace tobiss
