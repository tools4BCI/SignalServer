/**
* @file server_server.h
*
* @brief Declaration of the abstract base class of the SSClient implementation
*
**/

#ifndef SSCLIENTIMPL_BASE_H
#define SSCLIENTIMPL_BASE_H

// local

#include "datapacket/data_packet.h"

namespace tobiss
{
// forward declarations;
class SSConfig;
class DataPacket;

//-----------------------------------------------------------------------------
/**
* @class SSClientImplBase
*
* @brief Abstract Base of the SSClient Implementation
*/
class SSClientImplBase
{
public:
  /**
   * @brief Constructor
   */
  SSClientImplBase(){}
  /**
   * @brief Destructor
   */
  virtual ~SSClientImplBase(){}
  /**
   * @brief Establish a connection to a TOBI SignalServer
   * \sa connected(), disconnect()
   */
  virtual void connect(const std::string& address, short unsigned port) = 0;
  /**
   * @brief Tells if the client is connected to the server,
   *        i.e. if the control connection has been established.
   * \sa connect(), disconnect()
   */
  virtual bool connected() const = 0;
  /**
   * @brief Disconnect from the server, i.e. stops receiving and closes the control connection.
   * \sa connect(), disconnect()
   */
  virtual void disconnect() = 0;
  /**
   * @brief Request the meta data information from the server
   * \sa    config()
   */
  virtual void requestConfig() = 0;
  /**
   * @brief Returns the meta data information requested from the server
   * \sa requestConfig()
   */
  virtual SSConfig config() const = 0;
  /**
   * @brief Turns the client into receiving state
   * \sa stopReceiving(), receiving()
   */
  virtual void startReceiving(bool use_udp_bc) = 0;
  /**
   * @brief Tell if the client if is in receiving state
   * \sa startReceiving(), stopReceiving()
   */
  virtual bool receiving() const = 0;
  /**
   * @brief Stops receiving, i.e. calling getDataPacket() will fail afterwards.
   * \sa receiving(), startReceiving()
   */
  virtual void stopReceiving() = 0;
  /**
   * @brief Gets a packet from the server.
   */
  virtual void getDataPacket(DataPacket& packet) = 0;
  /**
   * @brief Sets the client's data input buffer size to the given value
   */
  virtual void setBufferSize(size_t size) = 0;
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SSCLIENTIMPL_BASE_H
