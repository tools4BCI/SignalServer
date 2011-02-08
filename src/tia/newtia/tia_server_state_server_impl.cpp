#include "tia-private/newtia/tia_server_state_server_impl.h"
#include "tia-private/newtia/network_impl/boost_tcp_server_socket_impl.h"

namespace tia
{

//-----------------------------------------------------------------------------
TiAServerStateServerImpl::TiAServerStateServerImpl (boost::asio::io_service& io_service, unsigned port)
    : server_socket_ (new BoostTCPServerSocketImpl (io_service))
{
    server_socket_->startListening (port, shared_from_this());
}

//-----------------------------------------------------------------------------
unsigned TiAServerStateServerImpl::getPort () const
{
    return 0;
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::newConnection (boost::shared_ptr<Socket> socket)
{

}

}
