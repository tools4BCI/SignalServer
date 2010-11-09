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
* @udp_data_server.cpp
*
* @brief \TODO.
*
**/

// STL
#include <sstream>

// boost
#include <boost/bind.hpp>

// Standard
#include <assert.h>
#include <iostream>

// local
#include "network/udp_data_server.h"
#include "datapacket/data_packet.h"

namespace tobiss
{

using std::cout;
using std::cerr;
using std::endl;
using boost::uint16_t;
using boost::uint32_t;

//-----------------------------------------------------------------------------

UDPDataServer::UDPDataServer(boost::asio::io_service& io_service)
  : io_service_(io_service),
  socket_(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),
  num_clients_(0)
{
  boost::asio::socket_base::broadcast bcast(true);
  socket_.set_option(bcast);
}

//-----------------------------------------------------------------------------

void UDPDataServer::setDestination(const std::string& address, uint16_t port)
{
  boost::asio::ip::udp::resolver resolver(io_service_);
  std::ostringstream ss;
  ss << port;
  boost::asio::ip::udp::resolver::query query(address, ss.str());
  target_ = *resolver.resolve(query);
}

//-----------------------------------------------------------------------------

void UDPDataServer::sendDataPacket(DataPacket& packet)
{
  if (num_clients_ == 0)
    return;

  void* data = packet.getRaw();
  uint32_t size = packet.getRawMemorySize();

  assert(data != 0);
  assert(size != 0);
  #ifdef DEBUG
    cout << "UDPDataServer::sendDataPacket: broadcasting data packet at "
         << target_.address().to_string() << ":" << target_.port() << endl;
  #endif
  socket_.async_send_to(boost::asio::buffer(data, size),
                        target_,
                        boost::bind(&UDPDataServer::handleWrite,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}

//-----------------------------------------------------------------------------

void UDPDataServer::handleWrite(const boost::system::error_code& error,
    size_t /*bytes_transferred*/)
{
  if (error)
  {
    cerr << "UDPDataServer::handleWrite: broadcasting data packet failed -- Error:" << endl;
    cerr << "--> " << error.message() << endl;
    return;
  }
}

//-----------------------------------------------------------------------------

} // Namespace tobiss
