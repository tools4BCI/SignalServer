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
* @file event_listener.cpp
**/

#include "hardware/event_listener.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace tobiss
{
using boost::lexical_cast;
using boost::bad_lexical_cast;

using boost::uint32_t;
using std::vector;
using std::cout;
using std::endl;

//-----------------------------------------------------------------------------

EventListener::EventListener(boost::asio::io_service& io) :
//  HWThread(parser),
  initialized_(0),
  event_socket_udp_(io),
  buffer_(EVENT_BUFFER_SIZE)
{

  setType("EventListener");

  nr_ch_ = 0;
  blocks_ = 0;
  homogenous_signal_type_ = 1;
//   for(unsigned int n = 0; n< nr_ch_; n++)
//   {
//     channel_info_[n] = make_pair("marker",SIG_EVENT);
//     channel_types_.push_back(SIG_USER_2);
//   }

  boost::system::error_code ec;
  boost::asio::ip::udp::endpoint udp_endpoint(boost::asio::ip::udp::v4(), 12344);
  event_socket_udp_.open(boost::asio::ip::udp::v4(), ec);
  if (!ec)
  {
    event_socket_udp_.bind(udp_endpoint, ec);
    boost::asio::socket_base::broadcast bcast(true);
    event_socket_udp_.set_option(bcast);
  }
  initialized_ = 1;
  cout << " * EventListener sucessfully initialized" << endl;
}
//-----------------------------------------------------------------------------
EventListener::~EventListener()
{
  boost::system::error_code ec;
  event_socket_udp_.close(ec);
}

//-----------------------------------------------------------------------------

SampleBlock<double> EventListener::getAsyncData()
{
  boost::shared_lock<boost::shared_mutex> lock(rw_);
//   for(unsigned int n = 0; n < 16-events_.size(); n++)
//     events_.push_back(0);
  data_.init(1, events_.size() , vector<uint32_t>(events_.size(), SIG_EVENT) );
  data_.setSamples(events_);

  events_.clear();
  samples_available_ = false;
  lock.unlock();
  return(data_);
}

//-----------------------------------------------------------------------------

void EventListener::run()
{
  event_socket_udp_.async_receive(boost::asio::buffer(buffer_),
    boost::bind(&EventListener::listen4Events, this,
    boost::asio::placeholders::error,
    boost::asio::placeholders::bytes_transferred));
}

//-----------------------------------------------------------------------------

void EventListener::stop()
{

}

//-----------------------------------------------------------------------------

void EventListener::listen4Events(const boost::system::error_code& error,
  std::size_t bytes_transferred)
{
  if(error)
    throw std::runtime_error(error.message());
  std::string event;

//   boost::unique_lock<boost::shared_mutex> lock(rw_);
//   boost::unique_lock<boost::mutex> syn(sync_mut_);
//   samples_available_ = true;
  for(uint32_t n = 0; n < bytes_transferred; n++)
  {
    if(buffer_[n] == '\n')
    {
      try
      {
      events_.push_back(lexical_cast<double>(event));
      }
      catch(bad_lexical_cast &)
      {
      }
      event = "";
    }
    else
    {
      event += buffer_[n];
    }
  }
//   events_.push_back(lexical_cast<double>(event));
//   lock.unlock();
//   cond_.notify_all();
//   syn.unlock();

  cout << " Event Codes: " << endl;
  for(uint32_t n = 0; n < events_.size(); n++)
    cout << "   " << events_[n] << endl;

  cout << endl;
  run();
}

//-----------------------------------------------------------------------------

} //Namespace tobiss
