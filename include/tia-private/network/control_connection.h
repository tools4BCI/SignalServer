/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

/**
* @file control_connection.h
* @brief This file includes the class used to handle the TCP control connection.
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

namespace tia
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
* @brief ControlConnection handles a single TCP control connection to one client.
* @todo  Improve documentation of this class.
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
