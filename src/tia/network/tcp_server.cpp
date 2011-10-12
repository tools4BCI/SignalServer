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
* @file tcp_server.cpp
*
* @brief \TODO.
*
**/

// STL
#include <sstream>
#include <iostream>

// Boost
#include <boost/bind.hpp>

// local
#include "tia-private/network/tcp_server.h"

namespace tia
{
using boost::uint16_t;
using boost::uint32_t;

//-----------------------------------------------------------------------------

std::string TCPConnection::endpointToString(const boost::asio::ip::tcp::endpoint& endpoint)
{
  std::ostringstream ss;
  ss << endpoint.address().to_string();
  ss << ":";
  ss << endpoint.port();
  return ss.str();
}

//-----------------------------------------------------------------------------

TCPServer::TCPServer(boost::asio::io_service& io_service)
  : io_service_(io_service),
    acceptor_(io_service)
{}

//-----------------------------------------------------------------------------

void TCPServer::bind(uint16_t port)
{
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
}

//-----------------------------------------------------------------------------

void TCPServer::bind(const std::string& address, uint16_t port)
{
  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  boost::asio::ip::tcp::resolver resolver(io_service_);
  std::ostringstream ss;
  ss << port;
  boost::asio::ip::tcp::resolver::query query(address, ss.str());
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
}

//-----------------------------------------------------------------------------

void TCPServer::listen()
{
  acceptor_.listen();
  startAccept();
}

//-----------------------------------------------------------------------------

void TCPServer::startAccept()
{
  TCPConnection::pointer new_connection = TCPConnection::create(io_service_);

  acceptor_.async_accept(new_connection->socket(), boost::bind(
      &TCPServer::handleAccept, this, new_connection,
      boost::asio::placeholders::error));
}

//-----------------------------------------------------------------------------

} //Namespace tobiss

// End Of File
