#ifndef BOOST_SOCKET_IMPL_H
#define BOOST_SOCKET_IMPL_H

#include "socket.h"

#include <boost/asio.hpp>

#include <memory>

namespace tobiss
{
class TCPConnection;
}

namespace tia
{

class BoostTCPSocketImpl : public Socket
{
public:
    BoostTCPSocketImpl (boost::asio::io_service& io_service, boost::asio::ip::tcp::endpoint const& endpoint)
        : socket_ (new boost::asio::ip::tcp::socket (io_service))
    {
        socket_->connect (endpoint);
    }

    BoostTCPSocketImpl (boost::asio::io_service& io_service, boost::shared_ptr<tobiss::TCPConnection> con);

    virtual ~BoostTCPSocketImpl ()
    {
        socket_->close ();
        buffered_string_.clear ();
        if (!fusty_connection_.get ())
            delete socket_;
    }

    virtual std::string readLine (unsigned max_length);
    virtual std::string readString (unsigned length);
    virtual char readCharacter ();
    virtual void waitForData ();
    virtual void sendString (std::string const& str);

private:
    void readBytes (unsigned num_bytes);

    boost::shared_ptr<tobiss::TCPConnection> fusty_connection_;
    boost::asio::ip::tcp::socket* socket_;
    std::string buffered_string_;
};

}

#endif // BOOST_SOCKET_IMPL_H
