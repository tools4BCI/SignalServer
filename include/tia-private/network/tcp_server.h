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
* @file tcp_server.h
* @brief This file includes classes handling TCP connections.
**/

#ifndef TCPSERVER_H
#define TCPSERVER_H

// Standard
#include <string>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

//local
#include "tia/defines.h"

namespace tia
{
//-----------------------------------------------------------------------------

/**
* @class TCPConnection
*
* @brief A TCP Connection to a client
*/
class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
public:

  /**
  * @brief TCPConnection Handle
  */
  typedef boost::shared_ptr<TCPConnection> pointer;

  /**
  * @brief Creates a new TCPConnection object
  * @return Return a handle pointing to the new object.
  */
  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new TCPConnection(io_service));
  }

  static std::string endpointToString(const boost::asio::ip::tcp::endpoint& endpoint);

public:
  /**
   * @brief The socket associated with this connection
   */
  boost::asio::ip::tcp::socket& socket() { return socket_; }

  virtual ~TCPConnection()
  {
    socket_.close();
  }

private:
  /**
   * @brief Constructor
   */
  TCPConnection(boost::asio::io_service& io_service) :
      socket_(io_service)
    {    }

private:
  boost::asio::ip::tcp::socket socket_;
};

//-----------------------------------------------------------------------------

/**
* @class TCPServer
*
* @brief A TCP server base class used to establish TCP connections.
*/
class TCPServer
{
public:
  /**
   * @brief Constructor
   * @param io_service
   */
  TCPServer(boost::asio::io_service& io_service);

  /**
   * @brief Binds the server to the given port
   */
  void bind(boost::uint16_t port);

  /**
   * @brief Binds the server to the given address and port
   */
  void bind(const std::string& address, boost::uint16_t port);

  /**
   * @brief Starts listening
   */
  void listen();

protected:
  /**
   * @brief Accept a new connection
   */
  virtual void startAccept();

  /**
   * @brief Handles a new client connection (abstract method).
   * @param[in] new_connection A reference to a new TCPConnection object used from this TCPServer object.
   */
  virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error) = 0;

protected:
  boost::asio::io_service&       io_service_; ///<
  boost::asio::ip::tcp::acceptor acceptor_;   ///<
};

} // Namespace tobiss

#endif //TCPSERVER_H

// End Of File
