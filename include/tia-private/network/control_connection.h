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
* @file control_connection.h
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
#include "tia-private/config/control_messages.h"
#include "tia/constants.h"
#include "tcp_server.h"

namespace tobiss
{
// forward declarations
class ControlMsgEncoder;
class ControlMsgDecoder;
class ControlConnectionServer;
class TiAServer;
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

  typedef std::pair<int, std::string> ConnectionID;

  /**
   * @brief Creates a new ControlConnection
   * @return Handle pointing to the new control connection
   * @throws
   */
  static pointer create(boost::asio::io_service& io_service,
      const ConnectionID& id,
      ControlConnectionServer& ctl_conn_server,
      const TCPConnection::pointer& tcp_conn_handle)
  {
    return pointer(new ControlConnection(io_service, id, ctl_conn_server, tcp_conn_handle));
  }

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
  ControlConnection(boost::asio::io_service& io_service,
      const ConnectionID& id,
      ControlConnectionServer& ctl_conn_server,
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

  ConnectionID                             connection_id_;

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

  enum State
  {
    State_Connected,
    State_AllocatedDataConnection,
    State_TransmissionStarted,
    State_TransmissionStopped,
    State_ConnectionClosed
  };

  int                                      state_;                    ///<
  int                                      connection_type_;          ///<
  boost::asio::ip::tcp::endpoint           tcp_data_server_local_endpoint_;  ///<
};

//-----------------------------------------------------------------------------

} // Namespace tobiss

#endif //CONTROLCONNECTION_H

// End Of File
