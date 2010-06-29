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

class TCPDataConnection : public TCPServer, public boost::enable_shared_from_this<TCPDataConnection>
{
public:
  virtual ~TCPDataConnection(){}

  typedef boost::shared_ptr<TCPDataConnection> pointer;

  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new TCPDataConnection(io_service));
  }

  bool connected() const {return connection_; }

  boost::asio::ip::tcp::endpoint localEndpoint() const
  {
    return acceptor_.local_endpoint();
  }

  void sendDataPacket(DataPacket& packet);

protected:
  // @
  virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error);

private:
  /// Construct a connection with the given io_service.
  TCPDataConnection(boost::asio::io_service& io_service);

  /// Handle completion of a write operation.
  void handleWrite(const boost::system::error_code& e,
      std::size_t bytes_transferred);
private:
  ///
  TCPConnection::pointer         connection_;

  boost::asio::ip::tcp::endpoint remote_endpoint_;
};

//-----------------------------------------------------------------------------

class TCPDataServer
{
public:
  // @
  TCPDataServer(boost::asio::io_service& io_service);

  bool connected(const boost::asio::ip::tcp::endpoint& endpoint) const;

  boost::asio::ip::tcp::endpoint addConnection();

  void removeConnection(const boost::asio::ip::tcp::endpoint& endpoint);

  void enableTransmission(const boost::asio::ip::tcp::endpoint& endpoint, bool enable);

  // @
  // FIXME: const correctness
  void sendDataPacket(DataPacket& packet);

private:
  typedef std::map<boost::asio::ip::tcp::endpoint, TCPDataConnection::pointer> ClientConnectionMap;

  boost::asio::io_service&  io_service_;
  ClientConnectionMap       connections_;
  ClientConnectionMap       connections_transmission_enabled_;
  mutable boost::mutex      mutex_;
};

} // Namespace tobiss

#endif //TCPDATASERVER_H

// End Of File
