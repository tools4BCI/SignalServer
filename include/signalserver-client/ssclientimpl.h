/**
* @file server_server.h
*
* @brief Declaration of the base implementation of the SSClientImplBase interface
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
* @class SSClientImpl
*
* @brief Base implementation of the SSClient
*
*/
class SSClientImpl : public SSClientImplBase
{
public:
  /**
  * @brief Default Constructor
  */
  SSClientImpl();
  /**
  * @brief Destructor
  */
  virtual ~SSClientImpl();
  /**
  *  @brief Establish a connection to a TOBI SignalServer
  * The caller will be blocked until the connection has been made or an error has occurred.
  * @param address host name or the IPv4 address of the server
  * @param port the control connection port the server listens on
  * @throw std::ios_base::failure if the client could not
  *   connect to the server or if the client is already connected.
  */
  virtual void connect(const std::string& address, short unsigned port);
  /**
   * @brief Tells if the client is connected to the server,
   *        i.e. if the control connection has been established.
   * @return \c true if connected \c false otherwise.
   * @sa connect()
   */
  virtual bool connected() const;
  /**
   * @brief Disconnects from the server, i.e. stops receiving and closes the control connection.
   * @throw std::ios_base::failure if an occurs while disconnecting
   * \sa connect(), connected(), receiving()
   */
  virtual void disconnect();
  /**
   * @brief Requests the meta data information from the server
   * The caller will be blocked until the request has been processed or an error has occurred.
   * @throw std::ios_base::failure if the client is not connected or if an error occurred
   * sa config()
   */
  virtual void requestConfig();
  /**
   * @brief Returns the meta data information requested from the server
   * @sa requestConfig()
   */
  virtual SSConfig config() const;
  /**
   * @brief Turns the client into receiving state
   * The caller will be blocked until the client is ready to receive or an error has occurred.
   * @param use_udp_bc if \c true data will be received via UDP broadcast instead of TCP
   * @throw std::ios_base::failure if the client is not connected or if an error occurred
   * \sa stopReceiving(), receiving(), getDataPacket()
   */
  virtual void startReceiving(bool use_udp_bc);
  /**
   * @brief Tell if the client if is in receiving state
   * @return \c true if the client is in receiving state, \c false otherwise.
   * \sa startReceiving(), stopReceiving()
   */
  virtual bool receiving() const;
  /**
   * @brief Stops receiving, i.e. calling getDataPacket() will fail afterwards.
   * @throw std::ios_base::failure if the client is not connected or if an error occurred
   * \sa startReceiving(), receiving()
   */
  virtual void stopReceiving();
  /**
   * @brief Gets a packet from the server.
   * The caller will be blocked until a packet has been received or an error has occurred.
   * @param[out] packet the received data packet
   * @throw std::ios_base::failure if the client is not connected or isn't in receiving state or
   *                               if a receiving error has occurred
   * @throw std::overflow_error if an overflow occurs
   *
   */
  virtual void getDataPacket(DataPacket& packet);

  /**
   * @brief Sets the client's data input buffer size to the given value
   * @param size the size of the input buffer in [byte]
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
  * @brief Establishes a data connection to the SignalServer
  * @param use_udp_bc if \c true data will be received via UDP broadcast instead of TCP
  * @throws std::ios_base::failure if an error has occurred
  */
  void establishDataConnection(bool use_udp_bc);
  /**
  * @brief Closes the data connection
  * @throws std::ios_base::failure if an error has occurred
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
