/**
* @control_connection_server.h
*
* @brief \TODO.
*
**/

#ifndef CONTROLCONNECTIONSERVER_H
#define CONTROLCONNECTIONSERVER_H

// Standard
#include <assert.h>
#include <iostream>

// Boost
#include <boost/thread/condition.hpp>
#include <boost/cstdint.hpp>

// local
#include "config/control_messages.h"
#include "tcp_server.h"
#include "definitions/constants.h"

// forward declarations
class ControlMsgEncoder;
class ControlMsgDecoder;
class ControlConnectionServer;
class SignalServer;
class TCPDataServer;
class UDPDataServer;

using namespace tobiss;

//-----------------------------------------------------------------------------

class ControlConnectionHandler
{
public:
  ControlConnectionHandler() {}
  virtual ~ControlConnectionHandler() {}
  void handle_message(const ControlMsg& message, ReplyMsg& reply);
};

//-----------------------------------------------------------------------------

/// Represents a single connection from a client.
class ControlConnection : public boost::enable_shared_from_this<ControlConnection>
{
public:
  virtual ~ControlConnection();

  typedef boost::shared_ptr<ControlConnection> pointer;

  static pointer create(boost::asio::io_service& io_service, ControlConnectionServer& ctl_conn_server,
      const TCPConnection::pointer& tcp_conn_handle)
  {
    return pointer(new ControlConnection(io_service, ctl_conn_server, tcp_conn_handle));
  }

  /// Start the first asynchronous operation for the connection.
  void start();

private:
  /// Construct a connection with the given io_service.
  ControlConnection(boost::asio::io_service& io_service, ControlConnectionServer& ctl_conn_server,
      const TCPConnection::pointer& tcp_conn);

private:
  void handleClient();

  /// Handle completion of a read operation.
  void handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a write operation.
  void handle_write(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  void sendMsg(const ControlMsg& msg);

  void close();

private:
  static const size_t MAX_DATA_SIZE = 100 * 1024;
  ///
  boost::asio::io_service&                 io_service_;

  ControlConnectionServer&                 ctl_conn_server_;

  /// Buffer for incoming data.
  boost::asio::streambuf*                  input_buffer_;
  /// Buffer for outgoing data.
  boost::asio::streambuf*                  output_buffer_;
  ///
  TCPConnection::pointer                   tcp_connection_;
  /// The handler used to process the incoming request.
  ControlConnectionHandler*                connection_handler_;
  ///
  ControlMsgEncoder*                       msg_encoder_;
  ///
  ControlMsgDecoder*                       msg_decoder_;

  boost::shared_ptr<ConfigMsg>             config_msg_;

  bool                                     transmission_started_;
  int                                      connection_type_;
  boost::asio::ip::tcp::endpoint           tcp_data_server_endpoint_;
};

//-----------------------------------------------------------------------------

class ControlConnectionServer : public TCPServer
{
public:
  ///
  ControlConnectionServer(boost::asio::io_service& io_service, SignalServer& server);
  ///
  virtual ~ControlConnectionServer()
  {
    delete signal_info_;
    delete subject_info_;
  }

  TCPDataServer* tcpDataServer() const;

  UDPDataServer* udpDataServer() const;

  void getConfig(ConfigMsg& config);

protected:
  void createSubjectInfo();
  void createSignalInfo();

  ///
  virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error);

private:
  std::vector<ControlConnection::pointer> connections_;  ///< list holding the tcp connections
  boost::mutex                            mutex_;        ///< mutex needed for the connection list
  SignalServer&                           server_;
  SubjectInfo*                            subject_info_;
  SignalInfo*                             signal_info_;

  Constants                               cst_;
};

#endif //CONTROLCONNECTIONSERVER_H

// End Of File
