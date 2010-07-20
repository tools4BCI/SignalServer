/**
* @file server_server.h
*
* @brief
*
**/

#ifndef SSCLIENTIMPL_H
#define SSCLIENTIMPL_H

// Boost
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

// local
#include "ssclientimpl_base.h"
#include "definitions/defines.h"

using std::string;

namespace tobiss
{
// forward declaration
class ControlMsgEncoder;
class ControlMsgDecoder;
class DataPacket;

//-----------------------------------------------------------------------------

class SSClientImpl : public SSClientImplBase
{
  public:
    SSClientImpl();

    virtual ~SSClientImpl();

    virtual void connect(const std::string& address, boost::uint16_t port);

    virtual bool connected() const;

    virtual void disconnect();

    virtual void requestConfig();

    virtual SSConfig config() const;

    virtual void startReceiving(bool use_udp_bc);

    virtual bool receiving() const;

    virtual void stopReceiving();

    virtual void getDataPacket(DataPacket& packet);
	
	virtual void setBufferSize(size_t size);

  protected:
    enum ControlConnState
    {
      ControlConnState_NotConnected    = 0,
      ControlConnState_Connected       = 1,
      ControlConnState_AwaitingReply   = 2
    };

    enum DataInputState
    {
      DataInputState_NotConnected      = 0,
      DataInputState_Connected,
      DataInputState_Receiving
    };

  protected:
    void establishDataConnection(bool use_udp_bc);

    void closeDataConnection();

  protected:
    boost::asio::io_service         io_service_;
    boost::asio::ip::tcp::iostream  ctl_conn_stream_;

    boost::asio::ip::tcp::socket    data_socket_tcp_;
    boost::asio::ip::udp::socket    data_socket_udp_;

    ControlMsgEncoder*              msg_encoder_;
    ControlMsgDecoder*              msg_decoder_;

    int                             ctl_conn_state_;
    int                             data_input_state_;

    SSConfig                        config_;
    bool                            use_udp_bc_;

    boost::uint64_t                 last_packet_nr_;
    boost::uint32_t                 packet_offset_;
    boost::uint32_t                 buffer_offset_;
    size_t                          buffered_data_;

	size_t                          buffer_size_;
	
    vector<char>                    recv_buf_;
    vector<char>                    data_buf_;
    boost::asio::ip::tcp::endpoint  tcp_target_;

  #ifdef TIMING_TEST
    private:
      bool  lpt_flag_;
  #endif
};

} // Namespace tobiss
//-----------------------------------------------------------------------------

#endif // SSCLIENTIMPL_H
