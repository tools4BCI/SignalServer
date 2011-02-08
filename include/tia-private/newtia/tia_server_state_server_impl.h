#ifndef TIA_SERVER_STATE_SERVER_IMPL_H
#define TIA_SERVER_STATE_SERVER_IMPL_H

#include "tia_server_state_server.h"
#include "network/tcp_server_socket.h"
#include "server_state_connection.h"

#include <boost/asio/io_service.hpp>
#include <memory>
#include <list>

namespace tia
{

//-----------------------------------------------------------------------------
class TiAServerStateServerImpl : public TiAServerStateServer, public NewConnectionListener
{
public:
    TiAServerStateServerImpl (boost::shared_ptr<TCPServerSocket> server_socket, unsigned port);
    virtual ~TiAServerStateServerImpl ();

    virtual unsigned getPort () const;

    virtual void newConnection (boost::shared_ptr<Socket> socket);
    virtual void emitState (ServerState server_state);
private:
    boost::shared_ptr<TCPServerSocket> server_socket_;
    std::list<boost::shared_ptr<ServerStateConnection> > connections_;
    unsigned port_;
    ServerState current_state_;
};

}

#endif // TIA_SERVER_STATE_SERVER_IMPL_H
