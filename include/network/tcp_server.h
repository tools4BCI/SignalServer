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
  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new TCPConnection(io_service));
  }

  static std::string endpointToString(const boost::asio::ip::tcp::endpoint& endpoint);

public:
  /**
   * @brief The socket associated with this connection
   */
  boost::asio::ip::tcp::socket& socket() { return socket_; }

  virtual ~TCPConnection()
  {
    socket_.close();
  }

private:
  /**
   * @brief Constructor
   */
  TCPConnection(boost::asio::io_service& io_service) :
      socket_(io_service)
    {    }

private:
  boost::asio::ip::tcp::socket socket_;
};

//-----------------------------------------------------------------------------

/**
* @class TCPServer
*
* @brief
*/
class TCPServer
{
public:
  /**
   * @brief Constructor
   * @param io_service
   */
  TCPServer(boost::asio::io_service& io_service);

  /**
   * @brief Binds the server to the given port
   */
  void bind(boost::uint16_t port);

  /**
   * @brief Binds the server to the given address and port
   */
  void bind(const std::string& address, boost::uint16_t port);

  /**
   * @brief Starts listening
   */
  void listen();

protected:
  /**
   * @brief Accept a new connection
   */
  void startAccept();

  /**
   * @brief Handles a new client connection
   * @param[in]  packet
   * @throws
   */
  virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error) = 0;

protected:
  boost::asio::io_service&       io_service_; ///<
  boost::asio::ip::tcp::acceptor acceptor_;   ///<
};

} // Namespace tobiss

#endif //TCPSERVER_H

// End Of File
