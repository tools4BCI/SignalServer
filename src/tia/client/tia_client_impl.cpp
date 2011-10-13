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
* @file tia_client_impl.cpp
**/

// STL
#include <iostream>

// Boost
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/numeric/conversion/cast.hpp>

// local
#include "tia-private/client/tia_client_impl.h"
#include "tia-private/config/control_message_decoder.h"
#include "tia-private/config/control_message_encoder.h"
#include "tia-private/config/control_messages.h"
#include "tia-private/datapacket/data_packet_impl.h"

#ifdef TIMING_TEST
  #include "LptTools/LptTools.h"
  #define LPT1  0
  #define LPT2  1
#endif

namespace tia
{
using boost::numeric_cast;
using boost::numeric::bad_numeric_cast;
using boost::numeric::positive_overflow;
using boost::numeric::negative_overflow;

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::stringstream;
using boost::uint16_t;
using boost::uint32_t;
using boost::int32_t;

//-----------------------------------------------------------------------------

TiAClientImpl::TiAClientImpl() :
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
  buffer_size_(BUFFER_SIZE),
  recv_buf_(buffer_size_),
  data_buf_(0)
{
  msg_encoder_ = new ControlMsgEncoderXML;
  msg_decoder_ = new ControlMsgDecoderXML;

  packet_.reset( new DataPacketImpl);

  msg_decoder_->setInputStream(&ctl_conn_stream_);

    #ifdef WIN32
      SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    #endif

  #ifdef TIMING_TEST
    lpt_flag_ = 0;

    if(!LptDriverInstall())
    {
      cerr << "Installing LptTools lpt driver failed (do you have access rights for the lpt-port?).";
      throw std::runtime_error("Error installing LptTools lpt driver!");
    }

//     LptDetectPorts(tmp,addr,3);

    if(!LptInit())
    {
      cerr << "Initializing lpt driver failed (do you have access rights for the lpt-port?).";
      throw std::runtime_error("Error initializing lpt driver!");
    }
  #endif
}

//-----------------------------------------------------------------------------

TiAClientImpl::~TiAClientImpl()
{
  if(msg_encoder_)
    delete msg_encoder_;

  if(msg_decoder_)
    delete msg_decoder_;

  #ifdef TIMING_TEST
    LptExit();
  #endif
}

//-----------------------------------------------------------------------------

SSConfig TiAClientImpl::config() const
{
  return config_;
}

//-----------------------------------------------------------------------------

void TiAClientImpl::connect(const std::string& address,  short unsigned port)
{
   if (connected())
   {
     stringstream ex_str;
     ex_str << "TiAClient: Already connected!"
            << address << ":" << port;
     throw std::ios_base::failure(ex_str.str());
   }

   stringstream conv;
   conv << port;

   ctl_conn_stream_.connect(address, conv.str());
   if (!ctl_conn_stream_)
   {
     stringstream ex_str;
     ex_str << "TiAClient: An error occurred while connecting to server "
            << address << ":" << port;
     throw std::ios_base::failure(ex_str.str());
   }

   ctl_conn_state_ = ControlConnState_Connected;
}

//-----------------------------------------------------------------------------

bool TiAClientImpl::connected() const
{
  return (ctl_conn_state_ & ControlConnState_Connected) != 0;
}

//-----------------------------------------------------------------------------

void TiAClientImpl::disconnect()
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

void TiAClientImpl::requestConfig()
{
  if (ctl_conn_state_ == ControlConnState_NotConnected)
  {
    stringstream ex_str;
    ex_str << "TiAClient: Not connected!";
    throw std::ios_base::failure(ex_str.str());
  }

  GetConfigMsg msg;

  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  if (reply == 0)
  {
    stringstream ex_str;
    ex_str << "TiAClient: Cannot decode message";
    throw std::ios_base::failure(ex_str.str());
  }

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::Config: break;

    case ControlMsg::ErrorReply:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Getting the config failed due to a server error.";
      throw std::ios_base::failure(ex_str.str());
    }

    default:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Got unexpected reply of type '" << reply->msgType() << "'";
      throw std::ios_base::failure(ex_str.str());
    }
  }

  boost::shared_ptr<ConfigMsg> config_msg =
      boost::static_pointer_cast<ConfigMsg>(reply);

  config_.subject_info = config_msg->subject_info;
  config_.signal_info = config_msg->signal_info;
}

//-----------------------------------------------------------------------------

void TiAClientImpl::establishDataConnection(bool use_udp_bc)
{
  GetDataConnectionMsg msg;
  if (use_udp_bc)
    msg.setConnectionType(GetDataConnectionMsg::Udp);
  else
  {
    msg.setConnectionType(GetDataConnectionMsg::Tcp);
  }

  use_udp_bc_ = use_udp_bc;

  // TODO: check for connection loss
  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  cout << "TiAClient: Waiting on reply" << endl;

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  if (reply == 0)
  {
    stringstream ex_str;
    ex_str << "TiAClient: Cannot decode message";
    throw std::ios_base::failure(ex_str.str());
  }

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::DataConnection: break;

    case ControlMsg::ErrorReply:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Establishing data connection failed due to a server error.";
      throw std::ios_base::failure(ex_str.str());
    }

    default:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Got unexpected reply of type '" << reply->msgType() << "'";
      throw std::ios_base::failure(ex_str.str());
    }
  }
  boost::shared_ptr<DataConnectionMsg> data_conn_msg =
        boost::static_pointer_cast<DataConnectionMsg>(reply);

  boost::system::error_code ec;
  boost::asio::socket_base::receive_buffer_size buffer_size(buffer_size_);

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
     boost::system::error_code ec;
     tcp_target_ = boost::asio::ip::tcp::endpoint(
        ctl_conn_stream_.rdbuf()->remote_endpoint().address(),
        data_conn_msg->port());
     data_socket_tcp_.connect(tcp_target_, ec);
     boost::asio::socket_base::receive_buffer_size buffer_size(buffer_size_);
     data_socket_tcp_.set_option(buffer_size);
  }

  if (ec)
  {
    data_input_state_ = DataInputState_NotConnected;
    stringstream ex_str;
    ex_str << "TiAClient: Could not connect to signal server:";
    ex_str << "-->" << ec.message();
    throw std::ios_base::failure(ex_str.str());
  }

  data_input_state_ |= DataInputState_Connected;
}

//-----------------------------------------------------------------------------

void TiAClientImpl::closeDataConnection()
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
      stringstream ex_str;
      ex_str << "TiAClient: An error occurred while closing data connection:" << endl;
      ex_str << "-->" << ec.message();
      throw std::ios_base::failure(ex_str.str());
    }

    data_input_state_ = DataInputState_NotConnected;
}

//-----------------------------------------------------------------------------

void TiAClientImpl::startReceiving(bool use_udp_bc)
{
  if (ctl_conn_state_ == ControlConnState_NotConnected)
  {
    stringstream ex_str;
    ex_str << "TiAClient: Not connected!";
    throw std::ios_base::failure(ex_str.str());
  }

  if (receiving())
  {
    stringstream ex_str;
    ex_str << "TiAClient: Client already in receiving state!";
    throw std::ios_base::failure(ex_str.str());
  }

  if (data_input_state_ == DataInputState_NotConnected)
  {
    try
    {
      establishDataConnection(use_udp_bc);
    }
    catch(std::exception& e)
    {
      throw e;
    }
  }

  StartTransmissionMsg msg;

  // TODO: check for connection loss
  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  cout << "TiAClient: Waiting on reply" << endl;

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  if (reply == 0)
  {
    stringstream ex_str;
    ex_str << "TiAClient: Cannot decode message";
    throw std::ios_base::failure(ex_str.str());
  }

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::OkReply: break;

    case ControlMsg::ErrorReply:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Stop receiving failed due to a server error.";
      throw std::ios_base::failure(ex_str.str());
    }

    default:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Got unexpected reply of type '" << reply->msgType() << "'";
      throw std::ios_base::failure(ex_str.str());
    }
  }

  data_input_state_ |= DataInputState_Receiving;
}
//-----------------------------------------------------------------------------

bool TiAClientImpl::receiving() const
{
  return (data_input_state_ & DataInputState_Receiving) != 0;
}

//-----------------------------------------------------------------------------

void TiAClientImpl::stopReceiving()
{
  if (ctl_conn_state_ == ControlConnState_NotConnected)
  {
    stringstream ex_str;
    ex_str << "TiAClient: Not connected!";
    throw std::ios_base::failure(ex_str.str());
  }

  if (!receiving()) return;

  StopTransmissionMsg msg;

  // TODO: check for connection loss
  msg_encoder_->encodeMsg(msg, ctl_conn_stream_);

  cout << "TiAClient: Waiting on reply" << endl;

  boost::shared_ptr<ControlMsg> reply(msg_decoder_->decodeMsg());

  // Check reply type
  switch (reply->msgType())
  {
    case ControlMsg::OkReply: break;

    case ControlMsg::ErrorReply:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Stop receiving failed due to a server error.";
      throw std::ios_base::failure(ex_str.str());
    }

    default:
    {
      stringstream ex_str;
      ex_str << "TiAClient: Got unexpected reply of type '" << reply->msgType() << "'";
      throw std::ios_base::failure(ex_str.str());
    }
  }

  closeDataConnection();
}

//-----------------------------------------------------------------------------

DataPacket* TiAClientImpl::getEmptyDataPacket()
{
  return(packet_.get());
}

//-----------------------------------------------------------------------------

void TiAClientImpl::getDataPacket(DataPacket& packet)
{
  if ((ctl_conn_state_ == ControlConnState_NotConnected))
  {
    stringstream ex_str;
    ex_str << "TiAClient: Not connected!";
    throw std::ios_base::failure(ex_str.str());
  }

  if (!receiving())
  {
    stringstream ex_str;
    ex_str << "TiAClient: Client not in receiving state!";
    throw std::ios_base::failure(ex_str.str());
  }

  boost::system::error_code error;
  size_t bytes_transferred = 0;
  uint32_t packet_size = 0;
  DataPacketImpl p;

  if (packet_offset_ == 0)
  {
    if(use_udp_bc_)
    {
        bytes_transferred = data_socket_udp_.receive(boost::asio::buffer(recv_buf_), 0, error);

        if (error)
        {
          // TODO: try to sent stop cmd to server?
          closeDataConnection();
          std::string ex_str("TiAClient: Data connection broken\n -->");
          ex_str += error.message();

          throw std::ios_base::failure(ex_str);
        }
    }
    else
    {
        bytes_transferred = data_socket_tcp_.read_some(boost::asio::buffer(recv_buf_), error);

        if (error && error != boost::asio::error::message_size)
        {
          // TODO: try to sent stop cmd to server?
          closeDataConnection();
          std::string ex_str("TiAClient: Data connection broken\n -->");
          ex_str += error.message();

          throw std::ios_base::failure(ex_str);
        }

    }

    data_buf_.resize(buffer_offset_);
    data_buf_.insert(data_buf_.end(), recv_buf_.begin(), recv_buf_.begin() + bytes_transferred );

    buffer_offset_ = 0;
    packet_size = p.getRequiredRawMemorySize(&(data_buf_[packet_offset_]),
                                           numeric_cast<int32_t>(bytes_transferred));
  }

  while(packet_offset_ == 0 && ( (bytes_transferred < packet_size) || (packet_size == 0) ) )
  {
    uint32_t bytes_to_receive = 0;
    if(packet_size == 0)
      bytes_to_receive = 32;
    else
      bytes_to_receive = packet_size - bytes_transferred;

    vector<char> v(bytes_to_receive);

    if(use_udp_bc_)
    {
      cerr << "TiAClient: ERROR -- Packet fragmentation not possible in UDP!"<< endl;
      throw (std::ios_base::failure("TiAClientImpl::getDataPacket() --  Can not decode packet"));
    }
    else
    {
      boost::asio::read(data_socket_tcp_, boost::asio::buffer(v),
      boost::asio::transfer_at_least(bytes_to_receive), error);
    }

    if (error && error != boost::asio::error::message_size)
    {
      // TODO: try to sent stop cmd to server?
      closeDataConnection();
      std::string ex_str("TiAClient: Data connection broken\n -->");
      ex_str += error.message();

      throw std::ios_base::failure(ex_str);
    }

    data_buf_.insert(data_buf_.end(), v.begin(), v.end());
    bytes_transferred += v.size();
    packet_size = p.getRequiredRawMemorySize(&(data_buf_[packet_offset_]),
                                           numeric_cast<int32_t>(bytes_transferred));
  }

  try
  {
    p.reset(reinterpret_cast<char*>(&(data_buf_[packet_offset_])));
  }
  catch(std::runtime_error& e)
  {
    string ex_str("TiAClient: ***** STL Exception -- Runtime error -- caught! *****\n  -->");
    ex_str += e.what();
    cerr << ex_str << endl;

    packet_offset_ = 0;
    data_buf_.clear();
    buffer_offset_ = 0;

    throw (std::ios_base::failure("TiAClientImpl::getDataPacket() --  Can not decode packet"));
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
      cerr << "TiAClient: Warning @packet: " << numeric_cast<uint32_t>(p.getSampleNr());
      cerr << " -- lost " << numeric_cast<uint32_t>(p.getSampleNr() - (last_packet_nr_ +1))  << " sample(s)!" << endl;
    }
    if(p.getSampleNr() < (last_packet_nr_) )
    {
      cerr << "TiAClient: Warning @packet: " << numeric_cast<uint32_t>(p.getSampleNr());
      cerr << " -- previous sample nr: " << numeric_cast<uint32_t>(last_packet_nr_) << " -- got packet twice!" << endl;
//       return;
    }
  }
  catch(positive_overflow& e)
  {
    string ex_str(" TiAClient: ***** SampleNumber overflow detected! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    cerr << ex_str;

    cerr << "TiAClient: *** New packet nr: " << (uint32_t)(p.getSampleNr());
    cerr << " -- previous packet nr: " << (uint32_t)(last_packet_nr_) << "!" << endl;

    throw(std::overflow_error(ex_str));
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

void TiAClientImpl::setBufferSize(size_t size)
{
	recv_buf_.resize(size);
	buffer_size_ = size;
}

//-----------------------------------------------------------------------------

} // Namespace tobiss
