
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
  {}

  //@
  virtual ~UDPServer()
  {}

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
