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
* @file event_listener.h
*
* @brief a very first udp listener for events provided as sample block
*
**/

#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/cstdint.hpp>

#include "hw_thread.h"

using namespace std;

namespace tobiss
{
//-----------------------------------------------------------------------------
class EventListener : public HWThread
{
  public:
  /**
    * @brief Constructor
    */
    EventListener(boost::asio::io_service& io);

    /**
    * @brief Destructor
    */
    virtual ~EventListener();

    virtual SampleBlock<double> getAsyncData();
    /**
    * @brief Method to start listening for events.
    */
    virtual void run();
    /**
    * @brief Method to stop listening for events.
    */
    virtual void stop();

//-----------------------------------------------
  private:
    virtual SampleBlock<double> getSyncData()
    { return(data_); }

    void listen4Events(const boost::system::error_code& error,
                        std::size_t bytes_transferred);

    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const& )
    {  }

    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const&)
    {  }

//-----------------------------------------------

  private:
    bool initialized_;   ///< to check, if condition variable has been set at least once (otherwise deadlock)

//     TCPEventListener    tcp_event_listener_;
    boost::asio::ip::udp::socket    event_socket_udp_;

    boost::mutex sync_mut_;  ///< mutex neede for synchronisation
    boost::condition_variable_any cond_;   ///< condition variable to wake up getSyncData()

    vector<char> buffer_;
    vector<double> events_; ///< temporary vector holding recent samples of the sine (1 element per channel)
};

} // Namespace tobiss

#endif // EVENTLISTENER_H

//-----------------------------------------------------------------------------
