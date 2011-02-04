#ifndef BOOST_TCP_SERVER_SOCKET_IMPL_H
#define BOOST_TCP_SERVER_SOCKET_IMPL_H

#include "tia-private/newtia/network/tcp_server_socket.h"

#include <boost/asio.hpp>

namespace tia
{

//-----------------------------------------------------------------------------
class BoostTCPServerSocketImpl : public TCPServerSocket
{
public:
    //-------------------------------------------------------------------------
    BoostTCPServerSocketImpl (boost::asio::io_service& io_service)
        : acceptor_ (io_service)
    {}

    //-------------------------------------------------------------------------
    virtual void startListening (unsigned port,
                                 boost::shared_ptr<NewConnectionListener> new_connection_listener);

private:
    //-------------------------------------------------------------------------
    void asyncAccept ();

    //-------------------------------------------------------------------------
    void handleAccept (boost::shared_ptr<boost::asio::ip::tcp::socket> socket);

    boost::shared_ptr<NewConnectionListener> new_connection_listener_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

}

#endif // BOOST_TCP_SERVER_SOCKET_IMPL_H
