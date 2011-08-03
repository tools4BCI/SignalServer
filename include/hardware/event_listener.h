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
* @file event_listener.h
*
* @brief a very first udp listener for events provided as sample block
*
**/

#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <vector>

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/cstdint.hpp>

#include "hw_thread.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

/**
* @class EventListener
* @brief A temporary class used to inject events into the signal server.
* @warning This class is planned to be replaced by TOBI Interface D (TiD) and will be removed.
*/
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

    std::vector<char> buffer_;
    std::vector<double> events_; ///< temporary vector holding recent samples of the sine (1 element per channel)
};

} // Namespace tobiss

#endif // EVENTLISTENER_H

//-----------------------------------------------------------------------------
