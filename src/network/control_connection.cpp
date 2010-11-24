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
* @control_connection.cpp
*
* @brief \TODO.
*
**/

// Boost
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/iostreams/stream.hpp>

// local
#include "tia/constants.h"

#include "tia-private/config/control_message_decoder.h"
#include "tia-private/config/control_message_encoder.h"

#include "tia-private/network/control_connection.h"
#include "tia-private/network/control_connection_server.h"
#include "tia-private/network/tcp_data_server.h"
#include "tia-private/network/udp_data_server.h"

namespace tobiss
{
using std::cout;
using std::cerr;
using std::endl;
using std::istream;

using boost::uint16_t;
using boost::uint32_t;

//-----------------------------------------------------------------------------

ControlConnection::ControlConnection(boost::asio::io_service& io_service,
    const ConnectionID& id,
    ControlConnectionServer& ctl_conn_server,
    const TCPConnection::pointer& tcp_conn)
      : io_service_(io_service),
      connection_id_(id),
      ctl_conn_server_(ctl_conn_server),
      tcp_connection_(tcp_conn),
      msg_encoder_(0),
      msg_decoder_(0),
      state_(State_Connected),
      connection_type_(GetDataConnectionMsg::Tcp)
{
  input_buffer_   = new boost::asio::streambuf;
  output_buffer_  = new boost::asio::streambuf;

  msg_encoder_ = new ControlMsgEncoderXML;
  msg_decoder_ = new ControlMsgDecoderXML;

  config_msg_ = boost::shared_ptr<ConfigMsg>(new ConfigMsg);
  ctl_conn_server.getConfig(*config_msg_);
}

//-----------------------------------------------------------------------------

ControlConnection::~ControlConnection()
{
  delete input_buffer_;
  delete output_buffer_;
  delete msg_encoder_;
  delete msg_decoder_;
}

//-----------------------------------------------------------------------------

void ControlConnection::start()
{
  tcp_connection_->socket().async_read_some(input_buffer_->prepare(MAX_DATA_SIZE),
      boost::bind(&ControlConnection::handle_read, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

//-----------------------------------------------------------------------------

void ControlConnection::handle_read(const boost::system::error_code& error,
    std::size_t bytes_transferred)
{
  if (error)
  {
    if (error != boost::asio::error::eof)
    {
      cerr << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
           << "closing connection -- Error: "
           << "--> " << error.message() << endl;
    }

    close();
    return;
  }

  input_buffer_->commit(bytes_transferred);

  istream instream(input_buffer_);

  msg_decoder_->setInputStream(&instream);

  boost::shared_ptr<ControlMsg> msg(msg_decoder_->decodeMsg());

  if (msg != 0)
  {
    switch (msg->msgType())
    {
      case ControlMsg::KeepAlive:
      {
        // TODO:
        break;
      }

      case ControlMsg::GetConfig:
      {
        cout << "Client @" << connection_id_.second
             << " requests config" << endl;

        sendMsg(*config_msg_);

        break;
      }

      case ControlMsg::GetDataConnection:
      {
        #ifdef DEBUG
          cout << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
               << "Got Request 'GetDataConnection'" << endl;
        #endif

        if (state_ == State_TransmissionStarted)
        {
          cout << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
               << "request 'GetDataConnection' failed - "
                  "not allowed while transmission is running" << endl;
          sendMsg(ReplyMsg::error());
          break;
        }

        if (connection_type_ == GetDataConnectionMsg::Tcp &&
            (state_ == State_AllocatedDataConnection ||
             state_ == State_TransmissionStopped       ))
        {
          ctl_conn_server_.tcpDataServer()->removeConnection(tcp_data_server_local_endpoint_);
        }

        boost::shared_ptr<GetDataConnectionMsg> get_data_conn_msg =
            boost::static_pointer_cast<GetDataConnectionMsg>(msg);

        connection_type_ = get_data_conn_msg->connectionType();

        DataConnectionMsg data_conn_msg;

        switch (connection_type_)
        {
          case GetDataConnectionMsg::Udp:
          {
            boost::asio::ip::udp::endpoint endpoint =
                ctl_conn_server_.udpDataServer()->destination();

            data_conn_msg.setPort(endpoint.port());

            cout << "Client @" << connection_id_.second
                 << " requests UDP data broadcast - allocating port "
                 << endpoint.port() << endl;
            break;
          }
          case GetDataConnectionMsg::Tcp:
          {
            tcp_data_server_local_endpoint_ = ctl_conn_server_.tcpDataServer()->addConnection();

            data_conn_msg.setPort(tcp_data_server_local_endpoint_.port());

            cout << "Client @" << connection_id_.second
                 << " requests TCP data connection - allocating port "
                 << tcp_data_server_local_endpoint_.port() << endl;

            break;
          }
        }

        if (state_ == State_Connected)
        {
          state_ = State_AllocatedDataConnection;
        }

        sendMsg(data_conn_msg);

        break;
      }

      case ControlMsg::StartTransmission:
      {
        cout << "Client @" << connection_id_.second
             << " requests data transmission start." << endl;

        if (state_ == State_Connected)
        {
          cout << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
               << "request 'StartTransmission' failed - cannot start transmission before "
               << "data connection has been requested." << endl;

          sendMsg(ReplyMsg::error());
          break;
        }

        if (state_ == State_TransmissionStarted)
        {
          cout << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
               << "request 'StartTransmission' failed - transmission is already running." << endl;

          sendMsg(ReplyMsg::error());
          break;
        }

        switch (connection_type_)
        {
          case GetDataConnectionMsg::Udp:
          {
            ctl_conn_server_.udpDataServer()->incClientCount();
            break;
          }
          case GetDataConnectionMsg::Tcp:
          {
            ctl_conn_server_.tcpDataServer()->enableTransmission(
                tcp_data_server_local_endpoint_, true);
            break;
          }
        }

        sendMsg(ReplyMsg::ok());

        state_ = State_TransmissionStarted;

        break;
      }

      case ControlMsg::StopTransmission:
      {
        cout << "Client @" << connection_id_.second
             << " requests data transmission stop." << endl;

        if (state_ == State_Connected)
        {
          cout << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
               << "request 'StopTransmission' failed - cannot stop transmission if  "
               << "data connection has never been requested." << endl;

          sendMsg(ReplyMsg::error());
          break;
        }

        if (state_ == State_TransmissionStopped)
        {
          cout << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
               << "request 'StopTransmission' failed - transmission is not running."
               << endl;
          sendMsg(ReplyMsg::error());
          break;
        }

        switch (connection_type_)
        {
          case GetDataConnectionMsg::Udp:
          {
            ctl_conn_server_.udpDataServer()->decClientCount();
            break;
          }
          case GetDataConnectionMsg::Tcp:
          {
            ctl_conn_server_.tcpDataServer()->enableTransmission(
                tcp_data_server_local_endpoint_, false);
            break;
          }
        }

        sendMsg(ReplyMsg::ok());

        state_ = State_TransmissionStopped;

        break;
      }

      default:
      {
        cout << "ControlConnection::handle_read [Client @" << connection_id_.second << "]: "
             << "unsupported request type (" << msg->msgType() << ") - ignoring request."
             << endl;
        break;
      }
    }

    // Read next message
    tcp_connection_->socket().async_read_some(input_buffer_->prepare(MAX_DATA_SIZE),
        boost::bind(&ControlConnection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}

//-----------------------------------------------------------------------------

void ControlConnection::sendMsg(const ControlMsg& msg)
{
#ifdef DEBUG
  cout << "ControlConnection::sendMsg [Client @" << connection_id_.second << "]" << endl;
#endif

  std::ostream ostream(output_buffer_);
  msg.writeMsg(*msg_encoder_, ostream);

#ifdef DEBUG
  {
    cout << "ControlConnection::sendMsg [Client @" << connection_id_.second << "]" << endl;
    cout << "-----------" << endl;

    std::stringstream sstream;
    msg.writeMsg(*msg_encoder_, sstream);

    cout << sstream.str() << endl;

    cout << "-----------" << endl;
  }
#endif

  boost::asio::async_write(tcp_connection_->socket(), output_buffer_->data(),
      boost::bind(&ControlConnection::handle_write, shared_from_this(),
        boost::asio::placeholders::error, output_buffer_->size()));
}

//-----------------------------------------------------------------------------

void ControlConnection::close()
{

#ifdef DEBUG
  cout << "ControlConnection::close [Client @" << connection_id_.second << "]" << endl;
#endif

  // Initiate graceful connection closure.
  boost::system::error_code ignored_ec;
  tcp_connection_->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

  tcp_connection_->socket().close();

  switch (connection_type_)
  {
    case GetDataConnectionMsg::Udp:
    {
      if (state_ == State_TransmissionStarted) ctl_conn_server_.udpDataServer()->decClientCount();
      break;
    }
    case GetDataConnectionMsg::Tcp:
    {
      if (state_ == State_AllocatedDataConnection ||
          state_ == State_TransmissionStarted     ||
          state_ == State_TransmissionStopped)
      {
        ctl_conn_server_.tcpDataServer()->removeConnection(tcp_data_server_local_endpoint_);
      }

      break;
    }
  }

  state_ = State_ConnectionClosed;

  ctl_conn_server_.clientHasDisconnected(connection_id_);
}

//-----------------------------------------------------------------------------

void ControlConnection::handle_write(const boost::system::error_code& error,
                                     std::size_t bytes_transferred)
{
#ifdef DEBUG
  cout << "ControlConnection::handle_write [Client @" << connection_id_.second << "]" << endl;
#endif

  if (error)
  {
    cerr << "ControlConnection::handle_write [Client@" << connection_id_.second << "]: "
         << "error during write - closing connection."
         << "--> " << error.message() << endl;
    close();

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
    return;
  }

  // Consume whole input sequence
  output_buffer_->consume(bytes_transferred);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

// End Of File
