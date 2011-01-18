#ifndef BOOST_SOCKET_IMPL_H
#define BOOST_SOCKET_IMPL_H

#include "socket.h"

#include <boost/asio/ip/tcp.hpp>

namespace tia
{

class BoostTCPSocketImpl : public Socket
{
public:
    BoostTCPSocketImpl (boost::asio::io_service& io_service, boost::asio::ip::tcp::endpoint const& endpoint)
        : socket_ (io_service, endpoint)
    {}

    virtual std::string readLine (unsigned max_length);
    virtual std::string readString (unsigned max_length);
    virtual char readCharacter ();
    virtual void waitForData ();
    virtual void sendString (std::string const& str);

private:
    boost::asio::ip::tcp::socket socket_;
};

}

#endif // BOOST_SOCKET_IMPL_H
