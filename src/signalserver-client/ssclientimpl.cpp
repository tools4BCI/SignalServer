/**
* @file ssclientimpl.cpp
*
* @brief
*
**/

// STL
#include <iostream>

// Boost
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/numeric/conversion/cast.hpp>

// local
#include "signalserver-client/ssclientimpl.h"
#include "config/control_message_decoder.h"
#include "config/control_message_encoder.h"
#include "config/control_messages.h"
#include "datapacket/data_packet.h"

#ifdef TIMING_TEST
  #include "LptTools/LptTools.h"
  #define LPT1  0
  #define LPT2  1
#endif
  
using boost::numeric_cast;
using boost::numeric::bad_numeric_cast;
using boost::numeric::positive_overflow;
using boost::numeric::negative_overflow;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::stringstream;
using boost::uint16_t;
using boost::uint32_t;
using boost::int32_t;
  
//-----------------------------------------------------------------------------

SSClientImpl::SSClientImpl() :
  data_socket_tcp_(io_service_),
  data_socket_udp_(io_service_),
  msg_encoder_(0),
  msg_decoder_(0),
  ctl_conn_state_(ControlConnState_NotConnected),
  data_input_state_(DataInputState_NotConnected),
  use_udp_bc_(false),
  last_packet_nr_(0),
  packet_offset_(0),
  buffer_offset_(0),
  buffered_data_(0),
  recv_buf_(BUFFER_SIZE),
  data_buf_(0)
{
  msg_encoder_ = new ControlMsgEncoderXML;
  msg_decoder_ = new ControlMsgDecoderXML;

  msg_decoder_->setInputStream(&ctl_conn_stream_);

  io_service_.run();

    #ifdef WIN32
      SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    #endif

  #ifdef TIMING_TEST
    lpt_flag_ = 0;

    if(!LptDriverInstall())
    {
      cerr << "Installing LptTools lpt driver failed (do you have access rights for the lpt-port?)." << endl;
      throw std::runtime_error("Error installing LptTools lpt driver!");
    }

//     LptDetectPorts(tmp,addr,3);

    if(!LptInit())
    {
      cerr << "Initializing lpt driver failed (do you have access rights for the lpt-port?)." << endl;
      throw std::runtime_error("Error initializing lpt driver!");
    }
  #endif
}

//-----------------------------------------------------------------------------

SSClientImpl::~SSClientImpl()
{
  io_service_.stop();
  delete msg_encoder_;
  delete msg_decoder_;

  #ifdef TIMING_TEST
    LptExit();
  #endif
}

//-----------------------------------------------------------------------------

SSConfig SSClientImpl::config() const
{
  return config_;
}

//-----------------------------------------------------------------------------

void SSClientImpl::connect(const std::string& address, uint16_t port)
{
   if (connected())
   {
     // TODO: error handling
     return;
   }

   stringstream conv;
   conv << port;

   ctl_conn_stream_.connect(address, conv.str());
   if (!ctl_conn_stream_)
   {
     // TODO: handle errors
     cerr << "SSClient: An error occurred while connecting to server "
          << address << ":" << port << endl;
     return;
   }

   ctl_conn_state_ = ControlConnState_Connected;
}

//-----------------------------------------------------------------------------

bool SSClientImpl::connected() const
{
  return (ctl_conn_state_ & ControlConnState_Connected) != 0;
}

//-----------------------------------------------------------------------------

void SSClientImpl::disconnect()
{
  if (!connected()) return;

  if (receiving())
  {
    stopReceiving();
  }

  ctl_conn_stream_.close();

  ctl_conn_state_ = ControlConnState_NotConnected;
}

//-----------------------------------------------------------------------------

void SSClientImpl::requestConfig()
{
  if (ctl_conn_state_ == ControlConnState_NotConnected || (ctl_conn_state_ & ControlConnState_AwaitingReply) != 0)
    return;

  GetConfigMsg msg;
  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  ctl_conn_state_ |= ControlConnState_AwaitingReply;

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  ctl_conn_state_ =~ ControlConnState_AwaitingReply;

  if (reply == 0)
  {
    cerr << "SSClient: Cannot decode message" << endl;
    return;
  }

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::Config: break;

    case ControlMsg::ErrorReply:
    {
      cerr << "SSClient: Getting the config failed due to a server error." << endl;
      return;
    }

    default:
      cerr << "SSClient: Got unexpected reply of type '" << reply->msgType() << "'" << endl;
  }

  boost::shared_ptr<ConfigMsg> config_msg =
      boost::static_pointer_cast<ConfigMsg>(reply);

  config_.subject_info = config_msg->subject_info;
  config_.signal_info = config_msg->signal_info;

  ctl_conn_state_ =~ ControlConnState_AwaitingReply;
}

//-----------------------------------------------------------------------------

void SSClientImpl::establishDataConnection(bool use_udp_bc)
{
  GetDataConnectionMsg msg;
  if (use_udp_bc)
    msg.setConnectionType(GetDataConnectionMsg::Udp);
  else
  {
    msg.setConnectionType(GetDataConnectionMsg::Tcp);
  }

  use_udp_bc_ = use_udp_bc;

  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  cout << "SSClient: Waiting on reply" << endl;

  ctl_conn_state_ |= ControlConnState_AwaitingReply;

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  ctl_conn_state_ =~ ControlConnState_AwaitingReply;

  if (reply == 0)
  {
    // TODO: error handling
    cerr << "SSClient: Cannot decode message" << endl;
    return;
  }

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::DataConnection: break;

    case ControlMsg::ErrorReply:
    {
      cerr << "SSClient: Establishing data connection failed due to a server error." << endl;
      return;
    }

    default:
      cerr << "SSClient: Got unexpected reply of type '" << reply->msgType() << "'" << endl;
  }
  boost::shared_ptr<DataConnectionMsg> data_conn_msg =
        boost::static_pointer_cast<DataConnectionMsg>(reply);

  boost::system::error_code ec;
  boost::asio::socket_base::receive_buffer_size buffer_size(BUFFER_SIZE);

  if (use_udp_bc_)
  {
    boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), data_conn_msg->port());
    data_socket_udp_.open(boost::asio::ip::udp::v4(), ec);
    if (!ec)
    {
      data_socket_udp_.bind(endpoint, ec);
      boost::asio::socket_base::broadcast bcast(true);
      data_socket_udp_.set_option(bcast);
      data_socket_udp_.set_option(buffer_size);
    }
  }
  else
  {
//     boost::system::error_code ec;
    tcp_target_ = boost::asio::ip::tcp::endpoint(
        ctl_conn_stream_.rdbuf()->remote_endpoint().address(),
        data_conn_msg->port());
//     data_socket_tcp_.connect(endpoint, ec);
  }

  if (ec)
  {
    data_input_state_ = DataInputState_NotConnected;
    cerr << "SSClient: Could not connect to signal server" << endl;
    return;
  }

  // TODO: get port from config
  data_input_state_ |= DataInputState_Connected;
}

//-----------------------------------------------------------------------------

void SSClientImpl::closeDataConnection()
{
    boost::system::error_code ec;

    if(use_udp_bc_)
    {
      data_socket_udp_.shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
      data_socket_udp_.close(ec);
    }
    else
    {
      data_socket_tcp_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
      data_socket_tcp_.close(ec);
    }

    if (ec)
    {
      cerr << "SSClient: An error occurred while closing data connection" << endl;
    }

    data_input_state_ = DataInputState_NotConnected;

    return;
}

//-----------------------------------------------------------------------------

void SSClientImpl::startReceiving(bool use_udp_bc)
{
  if (ctl_conn_state_ == ControlConnState_NotConnected
      || (ctl_conn_state_ & ControlConnState_AwaitingReply) != 0 || receiving())
    return;

  if (data_input_state_ == DataInputState_NotConnected)
  {
    establishDataConnection(use_udp_bc);
  }

  StartTransmissionMsg msg;
  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  cout << "SSClient: Waiting on reply" << endl;

  ctl_conn_state_ |= ControlConnState_AwaitingReply;

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  ctl_conn_state_ =~ ControlConnState_AwaitingReply;

  if (reply == 0)
  {
    // TODO: error handling
    cerr << "SSClient: Cannot decode message" << endl;
    return;
  }

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::OkReply: break;

    case ControlMsg::ErrorReply:
    {
      cerr << "SSClient: Start receiving failed due to a server error." << endl;
      return;
    }

    default:
      cerr << "SSClient: Got unexpected reply of type '" << reply->msgType() << "'" << endl;
  }

  if(!use_udp_bc)
  {
    boost::system::error_code ec;
    data_socket_tcp_.connect(tcp_target_, ec);
    boost::asio::socket_base::receive_buffer_size buffer_size(BUFFER_SIZE);
    data_socket_tcp_.set_option(buffer_size);
  }

  data_input_state_ |= DataInputState_Receiving;
}
//-----------------------------------------------------------------------------

bool SSClientImpl::receiving() const
{
  return (data_input_state_ & DataInputState_Receiving) != 0;
}

//-----------------------------------------------------------------------------

void SSClientImpl::stopReceiving()
{
  if (ctl_conn_state_ == ControlConnState_NotConnected
      || (ctl_conn_state_ & ControlConnState_AwaitingReply) != 0 || !receiving())
    return;

  StopTransmissionMsg msg;
  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  cout << "SSClient: Waiting on reply" << endl;

  ctl_conn_state_ |= ControlConnState_AwaitingReply;

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  ctl_conn_state_ =~ ControlConnState_AwaitingReply;

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::OkReply: break;

    case ControlMsg::ErrorReply:
    {
      cerr << "SSClient: Stop receiving failed due to a server error." << endl;
      return;
    }

    default:
      cerr << "SSClient: Got unexpected reply of type '" << reply->msgType() << "'" << endl;
  }

  closeDataConnection();
}

//-----------------------------------------------------------------------------

void SSClientImpl::getDataPacket(DataPacket& packet)
{
  if ((ctl_conn_state_ == ControlConnState_NotConnected) || !receiving())
    return;

  boost::system::error_code error;
  size_t bytes_transferred = 0;
  uint32_t packet_size = 0;
  DataPacket p;

  if (!(packet_offset_))
  {
    if(use_udp_bc_)
    {
      // TODO: What to do if server crashed?
      if(data_socket_udp_.is_open())
        bytes_transferred = data_socket_udp_.receive(boost::asio::buffer(recv_buf_));
    }
    else
    {
      // TODO: What to do if server crashed?
      if(data_socket_tcp_.is_open())
        bytes_transferred = data_socket_tcp_.read_some(boost::asio::buffer(recv_buf_), error);
    }
    data_buf_.resize(buffer_offset_);
    data_buf_.insert(data_buf_.end(), recv_buf_.begin(), recv_buf_.begin() + bytes_transferred );

    if (error && error != boost::asio::error::message_size)
      throw boost::system::system_error(error);

    buffer_offset_ = 0;
    packet_size = p.getRequiredRawMemorySize(&(data_buf_[packet_offset_]),
                                           numeric_cast<int32_t>(bytes_transferred));
  }

  if (error == boost::asio::error::eof)
    return;

  while(!(packet_offset_) && ( (bytes_transferred < packet_size) || (packet_size == 0) ) )
  {
    uint32_t bytes_to_receive = 0;
    if(packet_size == 0)
      bytes_to_receive = 32;
    else
      bytes_to_receive = packet_size - bytes_transferred;

    vector<char> v(bytes_to_receive);

    if(use_udp_bc_)
    {
      cerr << "SSClient: ERROR -- Packet fragmentation not possible in UDP!"<< endl;
      return;
    }
    else
    {
    boost::asio::read(data_socket_tcp_, boost::asio::buffer(v),
      boost::asio::transfer_at_least(bytes_to_receive), error);
    }

    if (error && error != boost::asio::error::message_size)
      throw boost::system::system_error(error);

    data_buf_.insert(data_buf_.end(), v.begin(), v.end());
    bytes_transferred += v.size();
    packet_size = p.getRequiredRawMemorySize(&(data_buf_[packet_offset_]),
                                           numeric_cast<int32_t>(bytes_transferred));
  }

  try
  {
    p = DataPacket(reinterpret_cast<char*>(&(data_buf_[packet_offset_])));
  }
  catch(std::runtime_error& e)
  {
    string ex_str("SSClient: ***** STL Exception -- Runtime error -- caught! *****\n  -->");
    ex_str += e.what();
    cerr << ex_str << endl;

    packet_offset_ = 0;
    data_buf_.clear();
    buffer_offset_ = 0;
    return;
  }

  packet_size = p.getRequiredRawMemorySize();
  packet_offset_ += packet_size;

  try
  {
    if(packet_offset_ == data_buf_.size())
    {
      packet_offset_ = 0;
      buffer_offset_ = 0;
      data_buf_.clear();
    }
    else
    {
      uint32_t next_packet_size =
      p.getRequiredRawMemorySize(&(data_buf_[packet_offset_]),
                                 numeric_cast<int32_t>(data_buf_.size() - packet_offset_));

      if( next_packet_size == 0 ||
          numeric_cast<int32_t>(data_buf_.size() - packet_offset_) < numeric_cast<int32_t>(next_packet_size))
      {
        buffer_offset_ = data_buf_.size() - packet_offset_;
        for(unsigned int n = 0; n < buffer_offset_ ; n++ )
          data_buf_[n] = data_buf_[n + packet_offset_];

        data_buf_.resize(buffer_offset_);
        packet_offset_ = 0;
      }
    }


    if(p.getSampleNr() > (last_packet_nr_ +1) )
    {
      cerr << "SSClient: Warning @packet: " << numeric_cast<uint32_t>(p.getSampleNr());
      cerr << " -- lost " << numeric_cast<uint32_t>(p.getSampleNr() - (last_packet_nr_ +1))  << " sample(s)!" << endl;
    }
    if(p.getSampleNr() < (last_packet_nr_) )
    {
      cerr << "SSClient: Warning @packet: " << numeric_cast<uint32_t>(p.getSampleNr());
      cerr << " -- previous sample nr: " << numeric_cast<uint32_t>(last_packet_nr_) << " -- got packet twice!" << endl;
//       return;
    }
  }
  catch(positive_overflow& e)
  {
    string ex_str(" SSClient: ***** SampleNumber overflow detected! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    cerr << ex_str << endl;

    cerr << "SSClient: *** New packet nr: " << (uint32_t)(p.getSampleNr());
    cerr << " -- previous packet nr: " << (uint32_t)(last_packet_nr_) << "!" << endl;
    return;
  }

  last_packet_nr_ =  p.getSampleNr();
  packet = p;

  #ifdef TIMING_TEST
    int port_state = LptPortIn(LPT1,0);
    if(!lpt_flag_)
    {
      lpt_flag_ = 1;
      LptPortOut(LPT1, 0, port_state | 0x04);
    }
    else
    {
      lpt_flag_ = 0;
      LptPortOut(LPT1, 0, port_state & ~0x04);
    }
  #endif

}

//-----------------------------------------------------------------------------


