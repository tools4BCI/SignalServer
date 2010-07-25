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
#include "ssconfig.h"
#include "definitions/defines.h"

using std::string;

namespace tobiss
{
// forward declaration
class ControlMsgEncoder;
class ControlMsgDecoder;
class DataPacket;
class SSConfig;

//-----------------------------------------------------------------------------
/**
* @class SSConfig
*
* @brief The core of the Signal Server
*
* @todo
*/
class SSClientImpl : public SSClientImplBase
{
public:
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  SSClientImpl();
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual ~SSClientImpl();
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual void connect(const std::string& address, short unsigned port);
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual bool connected() const;
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual void disconnect();
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual void requestConfig();
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual SSConfig config() const;
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual void startReceiving(bool use_udp_bc);
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual bool receiving() const;
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual void stopReceiving();
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual void getDataPacket(DataPacket& packet);
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  virtual void setBufferSize(size_t size);

protected:
  enum ControlConnState
  {
    ControlConnState_NotConnected    = 0,
    ControlConnState_Connected       = 1,
  };

  enum DataInputState
  {
    DataInputState_NotConnected      = 0,
    DataInputState_Connected,
    DataInputState_Receiving
  };

protected:
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  void establishDataConnection(bool use_udp_bc);
  /**
  * @brief Destructor
  * @param[in]  packet
  * @throws
  */
  void closeDataConnection();

protected:
  boost::asio::io_service         io_service_; ///<
  boost::asio::ip::tcp::iostream  ctl_conn_stream_; ///<

  boost::asio::ip::tcp::socket    data_socket_tcp_; ///<
  boost::asio::ip::udp::socket    data_socket_udp_; ///<

  ControlMsgEncoder*              msg_encoder_; ///<
  ControlMsgDecoder*              msg_decoder_; ///<

  int                             ctl_conn_state_; ///<
  int                             data_input_state_; ///<

  SSConfig                        config_; ///<
  bool                            use_udp_bc_; ///<

  boost::uint64_t                 last_packet_nr_; ///<
  boost::uint32_t                 packet_offset_; ///<
  boost::uint32_t                 buffer_offset_; ///<
  size_t                          buffered_data_; ///<

  size_t                          buffer_size_; ///<

  vector<char>                    recv_buf_; ///<
  vector<char>                    data_buf_; ///<
  boost::asio::ip::tcp::endpoint  tcp_target_; ///<

#ifdef TIMING_TEST
private:
bool  lpt_flag_;
#endif
};

} // Namespace tobiss
//-----------------------------------------------------------------------------

#endif // SSCLIENTIMPL_H
