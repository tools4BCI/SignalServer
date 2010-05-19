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

// forward declarations
class DataPacket;

// @
class UDPDataServer
{
public:
  // @
  UDPDataServer(boost::asio::io_service& io_service);
  // @
  virtual ~UDPDataServer(){}

  void setDestination(const std::string& address, boost::uint16_t port);

  boost::asio::ip::udp::endpoint destination() const { return target_; }

  void incClientCount()
  {
    ++num_clients_;
  }

  void decClientCount()
  {
    if (num_clients_ > 0) --num_clients_;
  }

  // @
  // FIXME: const correctness
  void sendDataPacket(DataPacket& packet);

private:
  // @
  void run();

  void handleWrite(const boost::system::error_code& error,
      std::size_t bytes_transferred);

private:
  boost::asio::io_service&                  io_service_;
  boost::asio::ip::udp::socket              socket_;
  boost::asio::ip::udp::endpoint            target_;
  boost::uint32_t                           num_clients_;
};

#endif //UDPDATASERVER_H
