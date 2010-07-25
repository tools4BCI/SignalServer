/**
* @file server_server.h
*
* @brief
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
* @class SSConfig
*
* @brief The core of the Signal Server
*
* @todo
*/
class SSClientImplBase
{
public:
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  SSClientImplBase(){}
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual ~SSClientImplBase(){}
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual void connect(const std::string& address, short unsigned port) = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual bool connected() const = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual void disconnect() = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual void requestConfig() = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual SSConfig config() const = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual void startReceiving(bool use_udp_bc) = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual bool receiving() const = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual void stopReceiving() = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual void getDataPacket(DataPacket& packet) = 0;
  /**
   * @brief Destructor
   * @param[in]  packet
   * @throws
   */
  virtual void setBufferSize(size_t size) = 0;
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SSCLIENTIMPL_BASE_H
