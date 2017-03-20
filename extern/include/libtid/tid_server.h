/*
    This file is part of TOBI Interface D (TiD).

    TOBI Interface D (TiD) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface D (TiD) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface D (TiD).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2012 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file tid_server.h
*
* @brief \TODO.
*
**/

#ifndef TID_SERVER_H
#define TID_SERVER_H

// Boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>

#include "tcp_server.h"
#include "tid_shm_server.h"
#include "tid_connection.h"

//-----------------------------------------------------------------------------

/**
* @class TiDServer
*
* @brief TODO
*/

class IDMessage;

namespace TiD
{

class TiDMessageBuilder;

class TiDServer : public TCPServer, public TiDSHMServer
{
  friend class TimedTiDServer;
  friend class LPTTiDServer;

  public:

    typedef std::map<TiDConnection::ConnectionID, TiDConnection::pointer>     TiDConnHandlers;

    /**
     * @brief Constructor
     * @param
     * @throw
     */
    TiDServer();
    /**
   * @brief Destructor
   */
    virtual ~TiDServer();

    bool newMessagesAvailable();
    void getLastMessages(std::vector<IDMessage>& messages);
    void clearMessages();
    void reserveNrOfMsgs (size_t expected_nr_of_msgs);
    void start();
    void stop();
    void update(boost::uint64_t rel_timestamp, boost::uint64_t packet_nr);

    void assumeZeroNetworkDelay(bool val);
    void keepIncomingMessages(bool val);


  protected:
    /**
     * @brief Handles a new client connection
     * @param new_connection the connection to the client
     */
    virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error);

    void clientHasDisconnected(const TiDConnection::ConnectionID& id);

    void dispatchMsg(IDMessage& msg, const TiDConnection::ConnectionID& src_id);

  private:

    bool running_;

    TiDConnHandlers             connections_;  ///< list holding handlers for each connected client
    boost::mutex                dispatch_mutex_;
    boost::mutex                erase_mutex_;
    std::vector< IDMessage >    messages_;

    TiDMessageBuilder*          msg_builder_;
    std::string                 current_xml_string_;

    boost::uint64_t             current_rel_timestamp_;
    boost::uint64_t             current_packet_nr_;
    TCTimeval                   current_timeval_;

    bool                        assume_zero_network_delay_;
    bool                        keep_messages_;

};

} // Namespace TiD

#endif //TID_SERVER_H

// End Of File
