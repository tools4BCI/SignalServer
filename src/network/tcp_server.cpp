/**
* @tcp_server.cpp
*
* @brief \TODO.
*
**/

// STL
#include <sstream>

// Boost
#include <boost/bind.hpp>

// local
#include "network/tcp_server.h"

using boost::uint16_t;
using boost::uint32_t;

//-----------------------------------------------------------------------------

TCPServer::TCPServer(boost::asio::io_service& io_service)
  : io_service_(io_service),
    acceptor_(io_service)
{}

//-----------------------------------------------------------------------------

void TCPServer::bind(uint16_t port)
{
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
}

//-----------------------------------------------------------------------------

void TCPServer::bind(const std::string& address, uint16_t port)
{
  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  boost::asio::ip::tcp::resolver resolver(io_service_);
  std::ostringstream ss;
  ss << port;
  boost::asio::ip::tcp::resolver::query query(address, ss.str());
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
}

//-----------------------------------------------------------------------------

void TCPServer::listen()
{
  acceptor_.listen();
  startAccept();
}

//-----------------------------------------------------------------------------

void TCPServer::startAccept()
{
  TCPConnection::pointer new_connection = TCPConnection::create(io_service_);

  acceptor_.async_accept(new_connection->socket(), boost::bind(
      &TCPServer::handleAccept, this, new_connection,
      boost::asio::placeholders::error));
}

//-----------------------------------------------------------------------------

// End Of File
