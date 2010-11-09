/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @tcp_data_server.h
*
* @brief \TODO.
*
**/

#ifndef TCPDATASERVER_H
#define TCPDATASERVER_H

// Standard
#include <set>
#include <vector>

// Boost
#include <boost/thread/condition.hpp>

// local
#include "tcp_server.h"

namespace tobiss
{
// forward declarations
class DataPacket;

//-----------------------------------------------------------------------------

/**
* @class TCPConnection
*
* @brief Represents a TCP data connection to a client
*
* @todo
*/
class TCPDataConnection : public TCPServer, public boost::enable_shared_from_this<TCPDataConnection>
{
public:
  /**
   * @brief TCPDataConnection Handle
   */
  typedef boost::shared_ptr<TCPDataConnection> pointer;

  /**
   * @brief Creates a new TCPDataConnection object
   * @return Handle pointing to the new object
   */
  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new TCPDataConnection(io_service));
  }

public:
  /**
   * @brief Destructor
   */
  virtual ~TCPDataConnection(){}

  /**
   * @brief Tells if the client is still connected
   * @return \c true if connected \c false otherwise
   */
  bool connected() const {return connection_; }

  /**
   * @brief The local endpoint
   */
  boost::asio::ip::tcp::endpoint localEndpoint() const
  {
    return acceptor_.local_endpoint();
  }

  /**
   * @brief Sends a DataPacket to the clients
   * @param packet the DataPacket to send
   */
  void sendDataPacket(DataPacket& packet);

protected:

  /**
   * @brief Handles a new client connection
   */
  virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error);

private:
  /**
   * @brief Construct a connection with the given io_service.
   */
  TCPDataConnection(boost::asio::io_service& io_service);

  /**
   * @brief Handle completion of a write operation.
   */
  void handleWrite(const boost::system::error_code& e,
      std::size_t bytes_transferred);

private:
  TCPConnection::pointer         connection_;  ///<

  boost::asio::ip::tcp::endpoint remote_endpoint_;  ///<
};

//-----------------------------------------------------------------------------

/**
* @class TCPDataServer
*
* @brief This class manages all TCP data connections to clients
*
*/
class TCPDataServer
{
public:
  /**
   * @brief Constructor
   */
  TCPDataServer(boost::asio::io_service& io_service);

  /**
   * @brief Tells if the server is connected with given endpoint (client)
   * @param endpoint the address and port of the client
   */
  bool connected(const boost::asio::ip::tcp::endpoint& endpoint) const;

  /**
   * @brief Adds a new client connection
   * @return The local endpoint the server listens on
   */
  boost::asio::ip::tcp::endpoint addConnection();

  /**
   * @brief Removes a client specified by its endpoint
   * @param endpoint the address and port of the client
   */
  void removeConnection(const boost::asio::ip::tcp::endpoint& endpoint);

  /**
   * @brief Enables/Disables the transmission of data packets to the specified client
   * @param enable If \c true the client will get data packets from the server.
   */
  void enableTransmission(const boost::asio::ip::tcp::endpoint& endpoint, bool enable);

  /**
   * @brief Sends a data packet to all active clients
   * @param packet the DataPacket to send
   * FIXME: const correctness
   * \sa enableTransmission()
   */
  void sendDataPacket(DataPacket& packet);

private:
  typedef std::map<boost::asio::ip::tcp::endpoint, TCPDataConnection::pointer> ClientConnectionMap;

  boost::asio::io_service&  io_service_; ///<
  ClientConnectionMap       connections_; ///<
  ClientConnectionMap       connections_transmission_enabled_; ///<
  mutable boost::mutex      mutex_; ///<
};

} // Namespace tobiss

#endif //TCPDATASERVER_H

// End Of File
