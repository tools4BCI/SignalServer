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

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

using boost::asio::ip::udp;

namespace tobiss
{
//---------------------------------------------------------------------------------------

class UDPServer
{
public:
  UDPServer(boost::asio::io_service& io_service)
    : io_sevice_(io_service), socket_(io_service)
  {
    boost::asio::socket_base::linger option(false, 0);
    socket_.set_option(option);
  }

  //@
  virtual ~UDPServer()
  {
    socket_.close();
  }

  void bind(boost::uint16_t port)
  {
    socket_.bind(udp::endpoint(udp::v4(), port));
  }

  void setDestination(const std::string& address, boost::uint16_t port);

protected:
    boost::asio::io_service&   io_service_;
    udp::socket                socket_;
    udp::endpoint              target_;
};

} // Namespace tobiss

//---------------------------------------------------------------------------------------

#endif // UDPSERVER_H
