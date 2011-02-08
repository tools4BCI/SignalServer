#ifndef TIA_SERVER_STATE_SERVER_IMPL_H
#define TIA_SERVER_STATE_SERVER_IMPL_H

#include "tia_server_state_server.h"
#include "network/tcp_server_socket.h"
#include "server_state_connection.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/io_service.hpp>
#include <memory>
#include <list>

namespace tia
{

//-----------------------------------------------------------------------------
class TiAServerStateServerImpl : public TiAServerStateServer, public NewConnectionListener, public boost::enable_shared_from_this<TiAServerStateServerImpl>
{
public:
    TiAServerStateServerImpl (boost::asio::io_service& io_service, unsigned port);

    unsigned getPort () const;

    virtual void newConnection (boost::shared_ptr<Socket> socket);

private:
    std::auto_ptr<TCPServerSocket> server_socket_;
    std::list<boost::shared_ptr<ServerStateConnection> > connections_;
};

}

#endif // TIA_SERVER_STATE_SERVER_IMPL_H
