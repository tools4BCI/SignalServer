#include "tia-private/newtia/network_impl/boost_tcp_server_socket_impl.h"
#include "tia-private/newtia/network_impl/boost_tcp_socket_impl.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>
#include <iostream>

using namespace boost::asio::ip;
using boost::shared_ptr;
using boost::weak_ptr;

namespace tia
{

//-----------------------------------------------------------------------------
void BoostTCPServerSocketImpl::startListening (unsigned port,
                                               NewConnectionListener* new_connection_listener)
{
    new_connection_listener_ = new_connection_listener;

    tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
    acceptor_.open (endpoint.protocol());
    acceptor_.set_option (boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind (endpoint);
    acceptor_.listen ();
    asyncAccept ();
}

//-----------------------------------------------------------------------------
unsigned BoostTCPServerSocketImpl::startListening (NewConnectionListener* new_connection_listener)
{
    new_connection_listener_ = new_connection_listener;

    tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 0);
    acceptor_.open (endpoint.protocol());
    acceptor_.set_option (boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind (endpoint);
    acceptor_.listen ();
    asyncAccept ();
    return acceptor_.local_endpoint().port();
}

//-------------------------------------------------------------------------
void BoostTCPServerSocketImpl::stopListening ()
{
    new_connection_listener_ = 0;
    acceptor_.cancel ();
}

//-----------------------------------------------------------------------------
void BoostTCPServerSocketImpl::asyncAccept ()
{
    shared_ptr<tcp::socket> peer_socket (new tcp::socket(acceptor_.get_io_service()));
    acceptor_.async_accept (*peer_socket, boost::bind (&BoostTCPServerSocketImpl::handleAccept,
                                                       this, peer_socket));
}

//-----------------------------------------------------------------------------
void BoostTCPServerSocketImpl::handleAccept (shared_ptr<tcp::socket> socket)
{
    if (new_connection_listener_)
        new_connection_listener_->newConnection (boost::shared_ptr<Socket>(new BoostTCPSocketImpl (socket)));
    asyncAccept ();
}


}
