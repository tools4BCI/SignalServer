/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @control_connection_server.h
*
* @brief \TODO.
*
**/

#ifndef CONTROLCONNECTIONSERVER_H
#define CONTROLCONNECTIONSERVER_H

// Standard
#include <assert.h>
#include <iostream>
#include <map>

// Boost
#include <boost/thread/condition.hpp>
#include <boost/cstdint.hpp>

// local
#include "config/control_messages.h"
#include "control_connection.h"
#include "definitions/constants.h"
#include "tcp_server.h"

namespace tobiss
{
// forward declarations
class SignalServer;
class SubjectInfo;
class TCPDataServer;
class UDPDataServer;
class SignalInfo;

//-----------------------------------------------------------------------------

/**
* @class ControlConnectionServer
*
* @brief Server to handle control requests from clients
*/
class ControlConnectionServer : public TCPServer
{
  friend class ControlConnection;

public:
  /**
   * @brief Constructor
   * @param
   * @throw
   */
  ControlConnectionServer(boost::asio::io_service& io_service, SignalServer& server);
  /**
   * @brief Destructor
   * @param
   * @throw
   */
  virtual ~ControlConnectionServer();

  /**
   * @brief Return a reference to the TCPDataServer
   */
  TCPDataServer* tcpDataServer() const;

  /**
   * @brief Return a reference to the UDPDataServer
   */
  UDPDataServer* udpDataServer() const;

  /**
   * @brief Get the server meta data
   * @param[out] config config message containing the server meta data
   */
  void getConfig(ConfigMsg& config);

protected:
  typedef std::map<ControlConnection::ConnectionID, ControlConnection::pointer> CtrlConnHandlers;

protected:

  /**
   * @brief Creates the SubjectInfo object
   */
  void createSubjectInfo();

  /**
   * @brief Creates the SignalInfo object
   */
  void createSignalInfo();

  /**
   * @brief Handles a new client connection
   * @param new_connection the connection to the client
   */
  virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error);

  void clientHasDisconnected(const ControlConnection::ConnectionID& id);

private:
  CtrlConnHandlers         connections_;  ///< list holding handlers for each connected client
  boost::mutex             mutex_;        ///< mutex needed for the connection list
  SignalServer&            server_;       ///< reference to the signal server core
  SubjectInfo*             subject_info_; ///< reference to the subject meta data
  SignalInfo*              signal_info_;  ///< reference to the signal meta data
  Constants                cst_;
};

} // Namespace tobiss

#endif //CONTROLCONNECTIONSERVER_H

// End Of File
