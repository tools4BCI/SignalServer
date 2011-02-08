#include "tia-private/newtia/tia_server_state_server_impl.h"

#include <boost/thread.hpp>
#include <iostream>

namespace tia
{

//-----------------------------------------------------------------------------
TiAServerStateServerImpl::TiAServerStateServerImpl (boost::shared_ptr<TCPServerSocket> server_socket, unsigned port)
    : server_socket_ (server_socket),
      port_ (port),
      current_state_ (SERVER_STATE_RUNNING)
{
    server_socket_->startListening (port_, this);
}

//-----------------------------------------------------------------------------
TiAServerStateServerImpl::~TiAServerStateServerImpl ()
{
    server_socket_->stopListening ();
}

//-----------------------------------------------------------------------------
unsigned TiAServerStateServerImpl::getPort () const
{
    return port_;
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::emitState (ServerState server_state)
{
    current_state_ = server_state;
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::newConnection (boost::shared_ptr<Socket> socket)
{
    socket->sendString ("asdf");
}

}
