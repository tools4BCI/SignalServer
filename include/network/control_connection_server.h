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

private:
  std::vector<ControlConnection::pointer> connections_;  ///< list holding the tcp connections
  boost::mutex                            mutex_;        ///< mutex needed for the connection list
  SignalServer&                           server_;       ///< reference to the signal server core
  SubjectInfo*                            subject_info_; ///< reference to the subject meta data
  SignalInfo*                             signal_info_;  ///< reference to the signal meta data
  Constants                               cst_;
};

} // Namespace tobiss

#endif //CONTROLCONNECTIONSERVER_H

// End Of File
