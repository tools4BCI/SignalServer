/**
* @tcp_server.h
*
* @brief \TODO.
*
**/

#ifndef TCPSERVER_H
#define TCPSERVER_H

// Standard
#include <string>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

//local
#include "definitions/defines.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
public:
  typedef boost::shared_ptr<TCPConnection> pointer;

  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new TCPConnection(io_service));
  }

public:
  // @
  boost::asio::ip::tcp::socket& socket() { return socket_; }

private:
  TCPConnection(boost::asio::io_service& io_service) :
    socket_(io_service)
  {  }

private:
  boost::asio::ip::tcp::socket socket_;
};

//-----------------------------------------------------------------------------

class TCPServer
{
public:
  // @
  TCPServer(boost::asio::io_service& io_service);

  // @
  void bind(boost::uint16_t port);

  // @
  void bind(const std::string& address, boost::uint16_t port);

  // @
  void listen();

protected:
  // @
  void startAccept();

  // @
  virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error) = 0;

protected:
  boost::asio::io_service&       io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

} // Namespace tobiss

#endif //TCPSERVER_H

// End Of File
