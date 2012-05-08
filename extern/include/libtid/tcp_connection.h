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
* @file tcp_connection.h
* @brief This file includes a class handling TCP connections.
**/

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <string>


#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/enable_shared_from_this.hpp>
namespace TiD
{

static const int SOCKET_BUFFER_SIZE = 65536;
//4194304;
static const int SOCKET_LINGER_TIMEOUT = 0;

//-----------------------------------------------------------------------------

/**
* @class TCPConnection
*
* @brief A TCP Connection to a client
*/

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
  public:

    /**
    * @brief TCPConnection Handle
    */
    typedef boost::shared_ptr<TCPConnection> pointer;

    /**
    * @brief Creates a new TCPConnection object
    * @return Return a handle pointing to the new object.
    */
    static pointer create(boost::asio::io_service& io_service);

    static std::string endpointToString(const boost::asio::ip::tcp::endpoint& endpoint);

    /**
     * @brief The socket associated with this connection
     */
    boost::asio::ip::tcp::socket& socket() { return socket_; }

    virtual ~TCPConnection();

  private:
    /**
     * @brief Constructor
     */
    TCPConnection(boost::asio::io_service& io_service);

  private:
    boost::asio::ip::tcp::socket socket_;
};

} // TiD

#endif
