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
* @file kinect.cpp
**/

#include <iostream>

#include "hardware/kinect.h"

using boost::interprocess::shared_memory_object;
using boost::interprocess::open_only;
using boost::interprocess::read_write;

using boost::posix_time::millisec;
using boost::posix_time::microsec_clock;

using std::vector;
using std::pair;
using std::string;
using std::cout;
using std::endl;

namespace tobiss
{

const HWThreadBuilderTemplateRegistratorWithoutIOService<KinectShmReader> KinectShmReader::FACTORY_REGISTRATOR_ ("kinect");
const char *KinectShmReader::SHARED_MEMORY_NAME = "MotionSensingSharedMemory";

//-----------------------------------------------------------------------------
KinectShmReader::KinectShmReader(ticpp::Iterator<ticpp::Element> hw)
  : HWThread(), old_frame_id_(0), connected_(false), event_reset_needed_(false), KinectShmReader_data_(6, 0)
{
#ifdef DEBUG
  cout << "KinectShmReader: Constructor" << endl;
#endif

  // setup shared memory
  try{
    shared_memory_obj_ = new shared_memory_object(open_only, SHARED_MEMORY_NAME, read_write);
    region_ = new boost::interprocess::mapped_region(*shared_memory_obj_, read_write);
    void *addr = region_->get_address();
    shared_memory_ = static_cast<shared_memory_struct *>(addr);
  } catch(boost::interprocess::interprocess_exception &ex)
  {
    throw(std::runtime_error("KinectShmReader::KinectShmReader -- unable to open shared memory. Is the motion sensing software running?"));
  }

  // increase the client counter in order to prevent that the motion sensing software destroys/* the shared memory when the client is still active
  shared_memory_->mutex.lock();
  shared_memory_->clients++;
  shared_memory_->mutex.unlock();

  setType("KinectShmReader motion sensing device");

  checkMandatoryHardwareTags(hw);

  if(mode_ != APERIODIC)
    throw(std::invalid_argument("KinectShmReader motion sensing device has to be started as aperiodic device!"));

  setChannelSettings(0);

  data_.init(1, channel_types_.size() , channel_types_);
  vector<boost::uint32_t> v;
  empty_block_.init(0, 0, v);

  last_change_ = microsec_clock::local_time();
}

//---------------------------------------------------------------------------------------

KinectShmReader::~KinectShmReader()
{
  shared_memory_->mutex.lock();
  shared_memory_->clients--;
  shared_memory_->mutex.unlock();

  delete region_;
  delete shared_memory_obj_;
}

//---------------------------------------------------------------------------------------

void KinectShmReader::setDeviceSettings(ticpp::Iterator<ticpp::Element>const&)
{
#ifdef DEBUG
  cout << "KinectShmReader: setDeviceSettings" << endl;
#endif
}

//---------------------------------------------------------------------------------------

void KinectShmReader::setChannelSettings(ticpp::Iterator<ticpp::Element>const&)
{
#ifdef DEBUG
  cout << "KinectShmReader: setChannelSettings" << endl;
#endif

  channel_types_.push_back(SIG_USER_2);
  channel_types_.push_back(SIG_USER_2);
  channel_types_.push_back(SIG_USER_2);
  channel_types_.push_back(SIG_USER_3);
  channel_types_.push_back(SIG_USER_3);
  channel_types_.push_back(SIG_USER_3);

  channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(1, pair<string, boost::uint32_t>("x", SIG_USER_2)));
  channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(2, pair<string, boost::uint32_t>("y", SIG_USER_2)));
  channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(3, pair<string, boost::uint32_t>("z", SIG_USER_2)));
  channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(4, pair<string, boost::uint32_t>("confidence", SIG_USER_3)));
  channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(5, pair<string, boost::uint32_t>("event", SIG_USER_3)));
  channel_info_.insert(pair<boost::uint16_t, pair<string, boost::uint32_t> >(6, pair<string, boost::uint32_t>("state", SIG_USER_3)));

  homogenous_signal_type_ = 0;
  nr_ch_ = 6;
}

//---------------------------------------------------------------------------------------

SampleBlock<double> KinectShmReader::getAsyncData()
{
  #ifdef DEBUG
    cout << "KinectShmReader: getAsyncData" << endl;
  #endif

  bool dirty = false;

  shared_memory_->mutex.lock();

  boost::uint32_t frame_id = shared_memory_->frame_id;

  if (frame_id - old_frame_id_ > 1 && old_frame_id_ != 0)
  {
    std::cout << std::endl << "* warning: missed " << frame_id - old_frame_id_ + 1 << " motion frames, ensure that master's sample rate is sufficiently high" << std::endl;
  }

  if (frame_id != old_frame_id_) // new frame was written to shared memory
  {
    dirty = true;

    KinectShmReader_data_[0] = shared_memory_->x;
    KinectShmReader_data_[1] = shared_memory_->y;
    KinectShmReader_data_[2] = shared_memory_->z;
    KinectShmReader_data_[3] = shared_memory_->confidence;
    KinectShmReader_data_[4] = shared_memory_->event;
    KinectShmReader_data_[5] = shared_memory_->state;

    old_frame_id_ = frame_id;
    last_change_ = microsec_clock::local_time();

    if (!connected_)
    {
      std::cout << std::endl << " * connected to motion sensing software" << std::endl;
      connected_ = true;
    }
  }
  else if (last_change_ + millisec(TIMEOUT) < microsec_clock::local_time() && connected_) // timeout, no new data frame was written to the shared memory for some time
  {
    dirty = true;

    KinectShmReader_data_[0] = 0;
    KinectShmReader_data_[1] = 0;
    KinectShmReader_data_[2] = 0;
    KinectShmReader_data_[3] = 0;
    KinectShmReader_data_[4] = 0;
    KinectShmReader_data_[5] = 0;

    std::cout << std::endl << " * connection to motion sensing software timed out" << std::endl;
    connected_ = false;
  }

  shared_memory_->mutex.unlock();

  if (dirty || event_reset_needed_)
  {
    data_.setSamples(KinectShmReader_data_);
    if (KinectShmReader_data_[4] != 0) // a hack to ensure that *discrete* events will still be discrete events on the client side
    {
      KinectShmReader_data_[4] = 0;
      event_reset_needed_ = true;
    }
    else
    {
      event_reset_needed_ = false;
    }
    return(data_);
  }
  else
  {
    return(empty_block_);
  }
}

//-----------------------------------------------------------------------------

void KinectShmReader::run() { }

//-----------------------------------------------------------------------------

void KinectShmReader::stop() { }

//-----------------------------------------------------------------------------

} // Namespace tobiss
