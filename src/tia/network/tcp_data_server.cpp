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
* @file tcp_data_server.cpp
*
* @brief \TODO.
*
**/

// Standard
#include <assert.h>
#include <iostream>

// boost
#include <boost/bind.hpp>

// local
#include "tia-private/network/tcp_data_server.h"
#include "tia/data_packet.h"

namespace tobiss
{
using std::cout;
using std::cerr;
using std::endl;
using std::make_pair;

using boost::uint32_t;

//-----------------------------------------------------------------------------

TCPDataConnection::TCPDataConnection(boost::asio::io_service& io_service) :
    TCPServer(io_service)
{}


//-----------------------------------------------------------------------------

void TCPDataConnection::handleWrite(const boost::system::error_code& ec,
    std::size_t /*bytes_transferred*/)
{
  if (ec)
  {
    cerr << "TCPDataConnection::handleWrite: sending data packet to client @"
              << remote_endpoint_.address().to_string() << ":" << remote_endpoint_.port()
              << " failed - closing connection -- Error:" << endl
              << "--> " << ec.message() << endl;

    boost::unique_lock<boost::mutex> lock(mutex_);

    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    connection_->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    connection_->socket().close();
  }
}

//-----------------------------------------------------------------------------

void TCPDataConnection::sendDataPacket(DataPacket& packet)
{
  boost::unique_lock<boost::mutex> lock(mutex_);

  if (!connection_ || !connection_->socket().is_open()) return;

  void* data = packet.getRaw();
  uint32_t size = packet.getRawMemorySize();

  assert(data != 0);
  assert(size != 0);

#ifdef DEBUG
  std::cout << "TCPDataConnection::sendDataPacket: sending data packet to "
                << remote_endpoint_.address().to_string() << ":" << remote_endpoint_.port()
                << std::endl;
#endif

  connection_->socket().async_send(boost::asio::buffer(data, size),
      boost::bind(&TCPDataConnection::handleWrite,
          shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

//-----------------------------------------------------------------------------

void TCPDataConnection::startAccept()
{
  TCPConnection::pointer new_connection = TCPConnection::create(io_service_);

  acceptor_.async_accept(new_connection->socket(), boost::bind(
      &TCPDataConnection::handleAccept, shared_from_this(), new_connection,
      boost::asio::placeholders::error));
}

//-----------------------------------------------------------------------------

void TCPDataConnection::handleAccept(const TCPConnection::pointer& new_connection,
      const boost::system::error_code& error)
{
  if (error)
  {
    cerr << "TCPDataConnection::handleAccept: cannot accept connection on "
         << localEndpoint().address().to_string() << ":" << localEndpoint().port()
         << "-- Error:" << endl
         << error.message() << endl;

    startAccept();
    return;
  }

  boost::unique_lock<boost::mutex> lock(mutex_);

  connection_ = new_connection;
  remote_endpoint_ = connection_->socket().remote_endpoint();
  boost::asio::socket_base::send_buffer_size buffer_option(BUFFER_SIZE);
  connection_->socket().set_option(buffer_option);

  boost::asio::ip::tcp::no_delay delay_option(true);
  connection_->socket().set_option(delay_option);

  boost::asio::socket_base::linger linger_option(false, 0);
  connection_->socket().set_option(linger_option);

#ifdef DEBUG
  cout << "TCPDataConnection::handleAccept: client @"
       << remote_endpoint_.address().to_string() << ":" << remote_endpoint_.port()
       << " connected on  "
       << connection_->socket().local_endpoint().address().to_string()
       << connection_->socket().local_endpoint().port();
#endif
}
//-----------------------------------------------------------------------------

TCPDataServer::TCPDataServer(boost::asio::io_service& io_service) :
  io_service_(io_service)
{}

//-----------------------------------------------------------------------------

bool TCPDataServer::connected(const boost::asio::ip::tcp::endpoint& endpoint) const
{
  boost::unique_lock<boost::mutex> lock(mutex_);
  ClientConnectionMap::const_iterator it = connections_.find(endpoint);
  if (it != connections_.end())
  {
    throw(std::invalid_argument("No connection with that local endpoint"));
  }

  TCPDataConnection::pointer connection = (*it).second;
  return connection->connected();
}

//-----------------------------------------------------------------------------

boost::asio::ip::tcp::endpoint TCPDataServer::addConnection()
{
  boost::unique_lock<boost::mutex> lock(mutex_);
  TCPDataConnection::pointer connection = TCPDataConnection::create(io_service_);

  boost::asio::ip::tcp::endpoint local_endpoint;

  try
  {
    connection->bind(0);
    local_endpoint = connection->localEndpoint();
  }
  catch (boost::exception& e)
  {
    throw (std::runtime_error("Could not bind socket - adding new connection failed"));
  }

  connection->listen();

  connections_.insert(make_pair(local_endpoint, connection));

  return local_endpoint;
}

//-----------------------------------------------------------------------------

void TCPDataServer::removeConnection(const boost::asio::ip::tcp::endpoint& endpoint)
{
  boost::unique_lock<boost::mutex> lock(mutex_);
  ClientConnectionMap::iterator it = connections_.find(endpoint);
  if (it == connections_.end())
  {
    throw(std::invalid_argument("No connection with that local endpoint"));
  }

  connections_.erase(it);

  connections_transmission_enabled_.erase(endpoint);
}

//-----------------------------------------------------------------------------

void TCPDataServer::enableTransmission(const boost::asio::ip::tcp::endpoint& endpoint,
                                       bool enable)
{
  boost::unique_lock<boost::mutex> lock(mutex_);

  ClientConnectionMap::iterator it = connections_.find(endpoint);
  if (it == connections_.end())
  {
    throw(std::invalid_argument("No connection with that local endpoint"));
  }

  if (enable)
  {
    connections_transmission_enabled_[(*it).first] = (*it).second;
  }
  else
  {
    connections_transmission_enabled_.erase((*it).first);
  }
}

//-----------------------------------------------------------------------------

void TCPDataServer::sendDataPacket(DataPacket& packet)
{
  // lock the connection list
  boost::unique_lock<boost::mutex> lock(mutex_);

  ClientConnectionMap::iterator it  = connections_transmission_enabled_.begin();
  ClientConnectionMap::iterator end = connections_transmission_enabled_.end();

  for (; it != end; ++it)
  {
    TCPDataConnection::pointer connection = (*it).second;
    connection->sendDataPacket(packet);
  }
}

//-----------------------------------------------------------------------------

} // Namespace tobiss
