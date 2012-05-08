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
* @file tcp_server.h
* @brief This file includes a class for a simple TCP server.
**/

#ifndef TCPSERVER_H
#define TCPSERVER_H

// Standard
#include <string>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread/thread.hpp>

#include "tcp_connection.h"

namespace TiD
{
//-----------------------------------------------------------------------------

/**
* @class TCPServer
*
* @brief A TCP server base class used to establish TCP connections.
*/
class TCPServer
{
  public:
    /**
     * @brief Constructor
     * @param io_service
     */
    TCPServer();
    virtual ~TCPServer();

    /**
     * @brief Binds the server to the given port
     */
    void bind(boost::uint16_t port);

    /**
     * @brief Binds the server to the given address and port
     */
    void bind(const std::string& address, boost::uint16_t port);

  protected:
    /**
     * @brief Starts listening
     */
    void listen();
    /**
     * @brief Accept a new connection
     */
    virtual void startAccept();

    /**
     * @brief Handles a new client connection (abstract method).
     * @param[in] new_connection A reference to a new TCPConnection object used from this TCPServer object.
     */
    virtual void handleAccept(const TCPConnection::pointer& new_connection,
          const boost::system::error_code& error) = 0;

  protected:
    boost::asio::io_service         io_service_; ///<
    boost::asio::ip::tcp::acceptor  acceptor_;   ///<
    boost::thread*                  io_service_thread_;
};

} // Namespace TiD

#endif //TCPSERVER_H

// End Of File
