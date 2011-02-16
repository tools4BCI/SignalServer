#include "tia-private/newtia/server_impl/tia_server_state_server_impl.h"
#include "tia-private/newtia/messages_impl/tia_control_message_builder_1_0.h"

#include <boost/thread.hpp>
#include <iostream>

namespace tia
{

//-----------------------------------------------------------------------------
TiAServerStateServerImpl::TiAServerStateServerImpl (boost::shared_ptr<TCPServerSocket> server_socket, unsigned port)
    : server_socket_ (server_socket),
      message_builder_ (new TiAControlMessageBuilder10),
      port_ (port),
      current_state_ (SERVER_STATE_RUNNING)
{
    state_map_[SERVER_STATE_RUNNING] = TiAControlMessageTags10::SERVER_STATE_RUNNING;
    state_map_[SERVER_STATE_SHUTDOWN] = TiAControlMessageTags10::SERVER_STATE_SHUTDOWN;
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

    std::list<boost::shared_ptr<Socket> >::iterator client_iter = server_to_client_sockets_.begin();
    while (client_iter != server_to_client_sockets_.end ())
    {
        try
        {
            emitStateViaSocket (*client_iter, current_state_);
            ++client_iter;
        }
        catch (TiALostConnection& /*exc*/)
        {
            client_iter = server_to_client_sockets_.erase (client_iter);
        }
    }
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::newConnection (boost::shared_ptr<Socket> socket)
{
    try
    {
        emitStateViaSocket (socket, current_state_);
        server_to_client_sockets_.push_back (socket);
    }
    catch (TiALostConnection& /*exc*/)
    {
        // do nothing
    }
}

//-----------------------------------------------------------------------------
void TiAServerStateServerImpl::emitStateViaSocket (boost::shared_ptr<Socket> socket, ServerState server_state)
{
    socket->sendString (message_builder_->buildTiAMessage (TiAControlMessage ("1.0", state_map_[server_state], "", "")));
}


}
