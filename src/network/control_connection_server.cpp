/**
* @control_connection_server.cpp
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

#include "signalserver/signal_server.h"

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

ControlConnection::ControlConnection(boost::asio::io_service& io_service, ControlConnectionServer& ctl_conn_server,
    const TCPConnection::pointer& tcp_conn)
      : io_service_(io_service),
      ctl_conn_server_(ctl_conn_server),
      tcp_connection_(tcp_conn),
      connection_handler_(0),
      msg_encoder_(0),
      msg_decoder_(0),
      transmission_started_(false),
      connection_type_(GetDataConnectionMsg::Tcp)
{
  input_buffer_   = new boost::asio::streambuf;
  output_buffer_  = new boost::asio::streambuf;

  connection_handler_ = new ControlConnectionHandler;
  msg_encoder_        = new ControlMsgEncoderXML;
  msg_decoder_        = new ControlMsgDecoderXML;

  config_msg_ = boost::shared_ptr<ConfigMsg>(new ConfigMsg);
  ctl_conn_server.getConfig(*config_msg_);
}

//-----------------------------------------------------------------------------

ControlConnection::~ControlConnection()
{
  delete input_buffer_;
  delete output_buffer_;
  delete connection_handler_;
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

ControlConnectionServer::ControlConnectionServer(boost::asio::io_service& io_service, SignalServer& server)
  : TCPServer(io_service),
  server_(server),
  subject_info_(0),
  signal_info_(0)
{
  signal_info_ = new SignalInfo;
  subject_info_ = new SubjectInfo;

  createSubjectInfo();
  createSignalInfo();
}

//-----------------------------------------------------------------------------

TCPDataServer* ControlConnectionServer::tcpDataServer() const
{
  return server_.tcp_data_server_;
}

//-----------------------------------------------------------------------------

UDPDataServer* ControlConnectionServer::udpDataServer() const
{
  return server_.udp_data_server_;
}

//-----------------------------------------------------------------------------

void ControlConnectionServer::getConfig(ConfigMsg& config)
{
  config.subject_info = *subject_info_;
  config.signal_info  = *signal_info_;
}

//-----------------------------------------------------------------------------

void ControlConnectionServer::createSubjectInfo()
{

  map<string,string> subject_map = server_.config_->parseSubject();

  subject_info_->setId(subject_map["id"]);
  subject_info_->setFirstName(subject_map["first_name"]);
  subject_info_->setSurname(subject_map["surname"]);
  subject_info_->setBirthday(subject_map["birthday"]);
  subject_info_->setMedication(subject_map["medication"]);

  std::string value = subject_map["sex"];
  if (value == "m")
    subject_info_->setSex(SubjectInfo::Male);
  else if (value == "f")
    subject_info_->setSex(SubjectInfo::Female);

  value = subject_map["handedness"];
  if (value == "r")
    subject_info_->setHandedness(SubjectInfo::RightHanded);
  else if (value == "l")
    subject_info_->setHandedness(SubjectInfo::LeftHanded);

  value = subject_map["glasses"];
  if (value == "y")
    subject_info_->setShortInfo(SubjectInfo::Glasses, SubjectInfo::Yes);
  else if (value == "n")
    subject_info_->setShortInfo(SubjectInfo::Glasses, SubjectInfo::No);
  else
    subject_info_->setShortInfo(SubjectInfo::Glasses, SubjectInfo::Unknown);

  value = subject_map["smoker"];
  if (value == "y")
    subject_info_->setShortInfo(SubjectInfo::Smoking, SubjectInfo::Yes);
  else if (value == "n")
    subject_info_->setShortInfo(SubjectInfo::Smoking, SubjectInfo::No);
  else
    subject_info_->setShortInfo(SubjectInfo::Smoking, SubjectInfo::Unknown);
}

//-----------------------------------------------------------------------------

void ControlConnectionServer::createSignalInfo()
{
  const std::vector<uint32_t>& sig_types       = server_.sig_types_;
  const std::vector<uint16_t>& blocksizes      = server_.blocksizes_;
  const std::vector<uint32_t>& fs_per_sig_type = server_.fs_per_sig_type_;

  const std::map<uint32_t, std::vector<std::string> >& channel_map = server_.channels_per_sig_type_;

  assert(sig_types.size() == blocksizes.size() && sig_types.size() == fs_per_sig_type.size());

  cout << endl;
  cout << "Sent Signal Types: (ordered)" << endl;
  for (vector<uint32_t>::size_type index = 0; index < sig_types.size(); ++index)
  {
    Signal signal;

    uint32_t sig_num_type = sig_types[index];
    std::string sig_str_type = Constants().getSignalName(sig_num_type);
    signal.setType(sig_str_type);
    cout << "  ... Signal type " << sig_str_type << endl;

    uint16_t block_size = blocksizes[index];
    signal.setBlockSize(block_size);

    uint32_t fs = fs_per_sig_type[index];
    signal.setSamplingRate(fs);

    std::map<uint32_t, std::vector<std::string> >::const_iterator it_channel_map =
      channel_map.find(sig_num_type);

    if (it_channel_map != channel_map.end())
    {
      const std::vector<std::string>& channel_names = (*it_channel_map).second;
      std::vector<std::string>::const_iterator it_channels = channel_names.begin();
      std::vector<std::string>::const_iterator end_channels = channel_names.end();
      for (; it_channels != end_channels; ++it_channels)
      {
        Channel channel;
        channel.setId(*it_channels);
        signal.channels().push_back(channel);
      }
    }

    signal_info_->signals().insert(make_pair(sig_str_type,signal));
  }

  signal_info_->setMasterBlockSize(server_.master_blocksize_);
  signal_info_->setMasterSamplingRate(server_.master_samplingrate_);
}


//-----------------------------------------------------------------------------

void ControlConnectionServer::handleAccept(const TCPConnection::pointer& new_connection,
      const boost::system::error_code& error)
{
  if (error)
  {
    // TODO: error handling
    return;
  }

  ControlConnection::pointer connection = ControlConnection::create(io_service_, *this, new_connection);
  connection->start();

  // lock the connection list
  boost::unique_lock<boost::mutex> lock(mutex_);
  connections_.push_back(connection);

  startAccept();
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

// End Of File
