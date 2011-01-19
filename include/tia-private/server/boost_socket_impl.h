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
        : socket_ (io_service)
    {
        socket_.connect (endpoint);
    }

    virtual ~BoostTCPSocketImpl ()
    {
        socket_.close ();
        buffered_string_.clear ();
    }

    virtual std::string readLine (unsigned max_length);
    virtual std::string readString (unsigned length);
    virtual char readCharacter ();
    virtual void waitForData ();
    virtual void sendString (std::string const& str);

private:
    void readBytes (unsigned num_bytes);

    boost::asio::ip::tcp::socket socket_;
    std::string buffered_string_;
};

}

#endif // BOOST_SOCKET_IMPL_H
