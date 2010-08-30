/**
* @control_connection_server.h
*
* @brief \TODO.
*
**/

#ifndef CONTROLCONNECTION_H
#define CONTROLCONNECTION_H

// Standard
#include <assert.h>
#include <iostream>

// Boost
#include <boost/thread/condition.hpp>
#include <boost/cstdint.hpp>

// local
#include "config/control_messages.h"
#include "definitions/constants.h"
#include "tcp_server.h"

namespace tobiss
{
// forward declarations
class ControlMsgEncoder;
class ControlMsgDecoder;
class ControlConnectionServer;
class SignalServer;
class TCPDataServer;
class UDPDataServer;

//-----------------------------------------------------------------------------

/**
* @class ControlConnection
*
* @brief Represents a single connection from a client.
*
*/
class ControlConnection : public boost::enable_shared_from_this<ControlConnection>
{
public:
  /**
   * @brief Control Connection Handle
   */
  typedef boost::shared_ptr<ControlConnection> pointer;

  /**
   * @brief Creates a new ControlConnection
   * @return Handle pointing to the new control connection
   * @throws
   */
  static pointer create(boost::asio::io_service& io_service,
      ControlConnectionServer& ctl_conn_server,
      const TCPConnection::pointer& tcp_conn_handle)
  {
    return pointer(new ControlConnection(io_service, ctl_conn_server, tcp_conn_handle));
  }

  void checkKeepAlive();

public:
  /**
   * @brief Destructor
   */
  virtual ~ControlConnection();

  /**
   * @brief Start the first asynchronous operation for the connection.
   */
  void start();

private:
  /// @brief Constructs a connection with the given io_service.
  ControlConnection(boost::asio::io_service& io_service, ControlConnectionServer& ctl_conn_server,
      const TCPConnection::pointer& tcp_conn);

private:
  /**
   * @brief Start the first asynchronous operation for the connection.
   */
  void handleClient();

  /**
   * @brief Handle completion of a read operation.
   */
  void handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /**
   * @brief Handle completion of a write operation.
   */
  void handle_write(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /**
   * @brief Sends a control message to the client
   */
  void sendMsg(const ControlMsg& msg);

  /**
   * @brief Closes the connection
   */
  void close();

private:
  static const size_t MAX_DATA_SIZE = 100 * 1024;
  ///
  boost::asio::io_service&                 io_service_; ///<

  ControlConnectionServer&                 ctl_conn_server_; ///<

  /// Buffer for incoming data.
  boost::asio::streambuf*                  input_buffer_; ///<
  /// Buffer for outgoing data.
  boost::asio::streambuf*                  output_buffer_; ///<
  ///
  TCPConnection::pointer                   tcp_connection_; ///<
  ///
  ControlMsgEncoder*                       msg_encoder_; ///<
  ///
  ControlMsgDecoder*                       msg_decoder_; ///<

  boost::shared_ptr<ConfigMsg>             config_msg_; ///<

  boost::shared_ptr<KeepAliveMsg>          keep_alive_msg_; ///<

  bool                                     transmission_started_; ///<
  int                                      connection_type_; ///<
  boost::asio::ip::tcp::endpoint           tcp_data_server_endpoint_; ///<
};

//-----------------------------------------------------------------------------

} // Namespace tobiss

#endif //CONTROLCONNECTION_H

// End Of File
