#ifndef TIA_SERVER_STATE_SERVER_IMPL_H
#define TIA_SERVER_STATE_SERVER_IMPL_H

#include "server/tia_server_state_server.h"
#include "network/tcp_server_socket.h"
#include "tia_control_message_builder.h"


#include <boost/asio/io_service.hpp>
#include <memory>
#include <list>
#include <map>

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
    void emitStateViaSocket (boost::shared_ptr<Socket> socket, ServerState server_state);

    std::map<ServerState, std::string> state_map_;
    boost::shared_ptr<TCPServerSocket> server_socket_;
    std::list<boost::shared_ptr<Socket> > server_to_client_sockets_;
    std::auto_ptr<TiAControlMessageBuilder> message_builder_;
    unsigned port_;
    ServerState current_state_;
};

}

#endif // TIA_SERVER_STATE_SERVER_IMPL_H
