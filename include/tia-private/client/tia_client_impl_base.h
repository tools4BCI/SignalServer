/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file server_server.h
*
* @brief Declaration of the abstract base class of the TiAClient implementation
*
**/

#ifndef TIA_CLIENT_IMPL_BASE_H
#define TIA_CLIENT_IMPL_BASE_H

// local

#include "tia/data_packet.h"

namespace tobiss
{
// forward declarations;
class SSConfig;
class DataPacket;

//-----------------------------------------------------------------------------
/**
* @class TiAClientImplBase
*
* @brief Abstract Base of the TiAClient Implementation
*/
class TiAClientImplBase
{
public:
  /**
   * @brief Constructor
   */
  TiAClientImplBase(){}
  /**
   * @brief Destructor
   */
  virtual ~TiAClientImplBase(){}
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

#endif // TIA_CLIENT_IMPL_BASE_H
