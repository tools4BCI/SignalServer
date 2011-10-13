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
* @file udp_data_server.h
* @brief This file includes a class handling UDP data tranmission.
**/

#ifndef UDPDATASERVER_H
#define UDPDATASERVER_H

// STL
#include <set>
#include <vector>

// BOOST
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

namespace tia
{
// forward declarations
class DataPacket;

/**
* @class UDPDataServer
*
* @brief Server to distribute DataPacket via UDP
*
* @todo Maybe use transmission into multiple subnets or use multicast.
*/
class UDPDataServer
{
public:
  /**
   * @brief Constructor
   * @param boost::asio::io_service
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

  std::string getTargetIP(){return(target_.address ().to_string ());}

  /**
   * @brief Returns the DataPacket destination
   * @sa setDestination()
   */
  boost::asio::ip::udp::endpoint destination() const { return target_; }

  /**
   * @brief Increments the number of clients of this server by one
   */
  void incClientCount()
  {
    ++num_clients_;
  }

  /**
   * @brief Decrements the number of clients of this server by one
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
