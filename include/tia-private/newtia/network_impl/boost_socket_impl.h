#ifndef BOOST_SOCKET_IMPL_H
#define BOOST_SOCKET_IMPL_H

#include "tia-private/newtia/network/socket.h"

#include <boost/asio.hpp>

#include <memory>

namespace tobiss
{
class TCPConnection;
}

namespace tia
{

//-----------------------------------------------------------------------------
class BoostTCPSocketImpl : public Socket
{
public:
    BoostTCPSocketImpl (boost::asio::io_service& io_service, boost::asio::ip::tcp::endpoint const& endpoint, unsigned buffer_size)
        : socket_ (new boost::asio::ip::tcp::socket (io_service))
    {
        socket_->connect (endpoint);
        boost::asio::socket_base::receive_buffer_size option (buffer_size);
        socket_->set_option (option);
    }

    BoostTCPSocketImpl (boost::shared_ptr<tobiss::TCPConnection> con);

    BoostTCPSocketImpl (boost::shared_ptr<boost::asio::ip::tcp::socket> boost_socket);

    virtual ~BoostTCPSocketImpl ();

    virtual void setReceiveBufferSize (unsigned size);
    virtual std::string readLine (unsigned max_length);
    virtual std::string readString (unsigned length);
    virtual char readCharacter ();
    virtual void waitForData ();
    virtual void sendString (std::string const& str) throw (TiALostConnection);

private:
    void readBytes (unsigned num_bytes);
    boost::shared_ptr<tobiss::TCPConnection> fusty_connection_;
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
    std::string buffered_string_;
};

}

#endif // BOOST_SOCKET_IMPL_H
