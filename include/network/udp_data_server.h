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
* @udp_data_server.h
*
* @brief \TODO.
*
**/

#ifndef UDPDATASERVER_H
#define UDPDATASERVER_H

// STL
#include <set>
#include <vector>

// BOOST
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

namespace tobiss
{
// forward declarations
class DataPacket;

/**
* @class UDPDataServer
*
* @brief Server to distribute DataPacket via UDP
*
* @todo
*/
class UDPDataServer
{
public:
  /**
   * @brief Constructor
   * @param io_service
   * @throws
   */
  UDPDataServer(boost::asio::io_service& io_service);
  /**
   * @brief Destructor
   */
  virtual ~UDPDataServer(){}

  /**
   * @brief Sets the destination to sent packets to.
   * \param address name or IPv4 address of the destination
   * \param port the destination port
   */
  void setDestination(const std::string& address, boost::uint16_t port);

  /**
   * @brief Returns the DataPacket destination
   * @sa setDestination()
   */
  boost::asio::ip::udp::endpoint destination() const { return target_; }

  /**
   * @brief Increments the number of clients of this server by one
   * @param[in]  packet
   * @throws
   */
  void incClientCount()
  {
    ++num_clients_;
  }

  /**
   * @brief Decrements the number of clients of this server by one
   * @param[in]  packet
   * @throws
   */
  void decClientCount()
  {
    if (num_clients_ > 0) --num_clients_;
  }

  /**
   * @brief Sends a DataPacket to the clients
   * @param  packet the DataPacket to be sent
   * FIXME: const correctness
   */
  void sendDataPacket(DataPacket& packet);

private:
  /**
   * @brief Runs the io_service
   */
  void run();

  /**
   * @brief Called if sending of a data packet has been finished
   */
  void handleWrite(const boost::system::error_code& error,
      std::size_t bytes_transferred);

private:
  boost::asio::io_service&                  io_service_; ///<
  boost::asio::ip::udp::socket              socket_; ///<
  boost::asio::ip::udp::endpoint            target_; ///<
  boost::uint32_t                           num_clients_; ///<
};

} // Namespace tobiss

#endif //UDPDATASERVER_H
