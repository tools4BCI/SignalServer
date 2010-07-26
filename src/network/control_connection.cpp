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
#include "definitions/constants.h"

#include "config/control_message_decoder.h"
#include "config/control_message_encoder.h"
#include "config/xml_parser.h"

#include "network/control_connection.h"
#include "network/control_connection_server.h"
#include "network/tcp_data_server.h"
#include "network/udp_data_server.h"

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
    ControlConnectionServer& ctl_conn_server,
    const TCPConnection::pointer& tcp_conn)
      : io_service_(io_service),
      ctl_conn_server_(ctl_conn_server),
      tcp_connection_(tcp_conn),
      msg_encoder_(0),
      msg_decoder_(0),
      transmission_started_(false),
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
    // TODO:
    if (error != boost::asio::error::eof)
      cerr << error.message() << endl;
    close();
    return;
  }

  input_buffer_->commit(bytes_transferred);

  istream instream(input_buffer_);

  msg_decoder_->setInputStream(&instream);

  boost::shared_ptr<ControlMsg> msg(msg_decoder_->decodeMsg());

  // TODO: do proper error handling
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
        cout << "Got Config Request" << endl;
        sendMsg(*config_msg_);
        break;
      }

      case ControlMsg::GetDataConnection:
      {
        cout << "Got GetDataConnection Request" << endl;

        boost::shared_ptr<GetDataConnectionMsg> get_data_conn_msg =
            boost::static_pointer_cast<GetDataConnectionMsg>(msg);

        connection_type_ = get_data_conn_msg->connectionType();

        DataConnectionMsg data_conn_msg;

        switch (connection_type_)
        {
          case GetDataConnectionMsg::Udp:
          {
            boost::asio::ip::udp::endpoint endpoint = ctl_conn_server_.udpDataServer()->destination();
            data_conn_msg.setPort(endpoint.port());
            break;
          }
          case GetDataConnectionMsg::Tcp:
          {
            // FIXME: may have already been requested
            tcp_data_server_endpoint_ = ctl_conn_server_.tcpDataServer()->addConnection();
            data_conn_msg.setPort(tcp_data_server_endpoint_.port());
            break;
          }
        }
        sendMsg(data_conn_msg);
        break;
      }

      case ControlMsg::StartTransmission:
      {
        cout << "Got Start Transmission Request" << endl;

        if (transmission_started_)
        {
          cerr << "Transmission already started" << endl;
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
            ctl_conn_server_.tcpDataServer()->enableTransmission(tcp_data_server_endpoint_, true);
            break;
          }
        }

        sendMsg(ReplyMsg::ok());

        transmission_started_ = true;

        break;
      }

      case ControlMsg::StopTransmission:
      {
        cout << "Got Stop Transmission Request" << endl;

        if (!transmission_started_)
        {
          cerr << "Cannot stop transmission - no transmission running." << endl;
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
            ctl_conn_server_.tcpDataServer()->enableTransmission(tcp_data_server_endpoint_, false);
            break;
          }
        }

        sendMsg(ReplyMsg::ok());
        transmission_started_ = false;
        break;
      }

      default:
      {
        //TODO: logging
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
  std::ostream ostream(output_buffer_);
  msg.writeMsg(*msg_encoder_, ostream);

  boost::asio::async_write(tcp_connection_->socket(), output_buffer_->data(),
      boost::bind(&ControlConnection::handle_write, shared_from_this(),
        boost::asio::placeholders::error, output_buffer_->size()));
}

//-----------------------------------------------------------------------------

void ControlConnection::close()
{
  cerr << "Closing control connection" << endl;

  // Initiate graceful connection closure.
  boost::system::error_code ignored_ec;
  tcp_connection_->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

  tcp_connection_->socket().close();

  switch (connection_type_)
  {
    case GetDataConnectionMsg::Udp:
    {
      if (transmission_started_) ctl_conn_server_.udpDataServer()->decClientCount();
      break;
    }
    case GetDataConnectionMsg::Tcp:
    {
      ctl_conn_server_.tcpDataServer()->removeConnection(tcp_data_server_endpoint_);
      break;
    }
  }
}

//-----------------------------------------------------------------------------

void ControlConnection::handle_write(const boost::system::error_code& e, std::size_t bytes_transferred)
{
  // TODO: do proper error handling

  if (e)
  {
    // TODO: logging ...
    cerr << "Error: " << e.message() << endl;
    close();
    return;
  }

  // Consume whole input sequence
  output_buffer_->consume(bytes_transferred);

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

// End Of File
